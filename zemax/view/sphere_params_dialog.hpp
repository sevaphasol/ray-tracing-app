#pragma once

#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/input_text.hpp"
#include "zemax/model/primitives/impls/sphere.hpp"
#include "zemax/model/rendering/color.hpp"
#include "zemax/model/rendering/scene_manager.hpp"
#include "zemax/view/obj_info_box.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace zemax {
namespace view {

class SphereParamsDialog : public ObjInfoBox {
  public:
    using CloseCb = std::function<void()>;

    enum class Mode { Create, Edit };

    // Режим СОЗДАНИЯ – новый объект, пустые поля, "Add Sphere"
    SphereParamsDialog( hui::WindowManager*         wm,
                        float                       x,
                        float                       y,
                        float                       w,
                        float                       h,
                        zemax::model::SceneManager& scene_manager,
                        CloseCb                     close_cb )
        : ObjInfoBox( wm, x, y, w, h, close_cb, "Add Sphere" ),
          scene_manager_( scene_manager ),
          mode_( Mode::Create ),
          obj_idx_( std::nullopt ),
          ok_btn_( wm,
                   { 10.0f, TopBarHeight + 8.0f },
                   { 80.0f, 24.0f },
                   CloseBtnDefaultColor,
                   CloseBtnHoveredColor,
                   CloseBtnPressedColor,
                   "OK",
                   CloseBtnFontColor,
                   CloseBtnFontSize ),
          cancel_btn_( wm,
                       { 100.0f, TopBarHeight + 8.0f },
                       { 80.0f, 24.0f },
                       CloseBtnDefaultColor,
                       CloseBtnHoveredColor,
                       CloseBtnPressedColor,
                       "Cancel",
                       CloseBtnFontColor,
                       CloseBtnFontSize )
    {
        initLayout( w );
        // В режиме Create поля НЕ заполняем
        wireButtons( close_cb );
    }

    // Режим РЕДАКТИРОВАНИЯ – существующий объект, поля заполнены, "Edit Sphere"
    SphereParamsDialog( hui::WindowManager*         wm,
                        float                       x,
                        float                       y,
                        float                       w,
                        float                       h,
                        zemax::model::SceneManager& scene_manager,
                        size_t                      obj_idx,
                        CloseCb                     close_cb )
        : ObjInfoBox( wm, x, y, w, h, close_cb, "Edit Sphere" ),
          scene_manager_( scene_manager ),
          mode_( Mode::Edit ),
          obj_idx_( obj_idx ),
          ok_btn_( wm,
                   { 10.0f, TopBarHeight + 8.0f },
                   { 80.0f, 24.0f },
                   CloseBtnDefaultColor,
                   CloseBtnHoveredColor,
                   CloseBtnPressedColor,
                   "OK",
                   CloseBtnFontColor,
                   CloseBtnFontSize ),
          cancel_btn_( wm,
                       { 100.0f, TopBarHeight + 8.0f },
                       { 80.0f, 24.0f },
                       CloseBtnDefaultColor,
                       CloseBtnHoveredColor,
                       CloseBtnPressedColor,
                       "Cancel",
                       CloseBtnFontColor,
                       CloseBtnFontSize )
    {
        initLayout( w );
        prefillFromObject(); // заполняем поля из сцены
        wireButtons( close_cb );
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        // Сначала кнопки
        if ( event.apply( &ok_btn_ ) )
        {
            return true;
        }
        if ( event.apply( &cancel_btn_ ) )
        {
            return true;
        }

        // Затем инпуты
        for ( const auto& u : inputs_storage_ )
        {
            if ( event.apply( u.get() ) )
            {
                return true;
            }
        }

        // И уже потом – базовый close button
        return ObjInfoBox::propagateEventToChildren( event );
    }

    void
    RedrawMyTexture() const override
    {
        // фон, рамки, топбар, крестик + заголовок
        ObjInfoBox::RedrawMyTexture();

        // перерисовать подписи
        for ( size_t i = 0; i < labels_.size(); ++i )
        {
            dr4::Vec2f pos = label_positions_[i];
            labels_[i]->SetPos( { pos.x, pos.y } );
            labels_[i]->DrawOn( *texture_ );
            inputs_storage_[i]->Redraw();
        }

        // инпуты сами рисуют себя через Redraw()
        ok_btn_.Redraw();
        cancel_btn_.Redraw();
    }

  private:
    void
    initLayout( float w )
    {
        const float margin_x   = 12.0f;
        const float margin_top = TopBarHeight + 8.0f + 36.0f;
        const float line_h     = 28.0f;
        const float label_w    = 80.0f;
        const float field_h    = 20.0f;
        const float field_w    = w - margin_x - label_w - 20.0f;

        const char* labels[] = {
            "X",
            "Y",
            "Z",
            "Radius",
            "R",
            "G",
            "B",
            "Reflection",
        };
        size_t n = sizeof( labels ) / sizeof( labels[0] );

        float cur_y = margin_top;

        auto*            win  = wm_->getWindow();
        const dr4::Font* font = win->GetDefaultFont();

        for ( size_t i = 0; i < n; ++i )
        {
            auto* t = win->CreateText();
            t->SetFont( font );
            t->SetFontSize( 12 );
            t->SetText( labels[i] );
            t->SetColor( { 220, 220, 220, 255 } );
            labels_.emplace_back( t );

            float fx    = margin_x + label_w + 8.0f;
            auto* input = new hui::InputText( wm_, fx, cur_y, field_w, field_h );
            input->setParent( this );
            inputs_storage_.emplace_back( input );

            switch ( i )
            {
                case 0:
                    x_field_ = input;
                    break;
                case 1:
                    y_field_ = input;
                    break;
                case 2:
                    z_field_ = input;
                    break;
                case 3:
                    radius_field_ = input;
                    break;
                case 4:
                    r_field_ = input;
                    break;
                case 5:
                    g_field_ = input;
                    break;
                case 6:
                    b_field_ = input;
                    break;
                case 7:
                    refl_field_ = input;
                    break;
            }

            label_positions_.push_back( dr4::Vec2f{ margin_x, cur_y } );
            cur_y += line_h;
        }

        ok_btn_.setParent( this );
        cancel_btn_.setParent( this );
    }

    void
    wireButtons( CloseCb close_cb )
    {
        ok_btn_.setOnClick( [this, close_cb]() { onOk( close_cb ); } );
        cancel_btn_.setOnClick( close_cb );
    }

    void
    prefillFromObject()
    {
        if ( mode_ != Mode::Edit || !obj_idx_.has_value() )
        {
            return;
        }

        auto  obj_info = scene_manager_.getObjectInfo( obj_idx_.value() );
        auto& obj      = scene_manager_.getObjects()[obj_idx_.value()];

        float radius = 0.0f;

        if ( auto* sphere = dynamic_cast<model::Sphere*>( obj.get() ) )
        {
            radius = sphere->getRadius();
        }

        x_field_->setString( std::to_string( obj_info.pos.x ) );
        y_field_->setString( std::to_string( obj_info.pos.y ) );
        z_field_->setString( std::to_string( obj_info.pos.z ) );

        radius_field_->setString( std::to_string( radius ) );

        r_field_->setString( std::to_string( obj_info.material.color.r ) );
        g_field_->setString( std::to_string( obj_info.material.color.g ) );
        b_field_->setString( std::to_string( obj_info.material.color.b ) );

        refl_field_->setString( std::to_string( obj_info.material.reflection_factor ) );
    }

    static std::optional<double>
    parseDoubleField( hui::InputText* field )
    {
        auto v = field->getDouble();
        if ( !v.has_value() )
        {
            field->setColor( { 255, 64, 64, 255 } );
            return std::nullopt;
        }
        field->setColor( { 255, 255, 255, 255 } );
        return v;
    }

    void
    onOk( CloseCb close_cb )
    {
        auto vx    = parseDoubleField( x_field_ );
        auto vy    = parseDoubleField( y_field_ );
        auto vz    = parseDoubleField( z_field_ );
        auto vr    = parseDoubleField( radius_field_ );
        auto vcr   = parseDoubleField( r_field_ );
        auto vcg   = parseDoubleField( g_field_ );
        auto vcb   = parseDoubleField( b_field_ );
        auto vrefl = parseDoubleField( refl_field_ );

        if ( !vx || !vy || !vz || !vr || !vcr || !vcg || !vcb || !vrefl )
        {
            return; // что-то не так – не создаём/не обновляем
        }

        model::Vector3f origin( static_cast<float>( *vx ),
                                static_cast<float>( *vy ),
                                static_cast<float>( *vz ) );

        dr4::Color color{
            static_cast<std::uint8_t>( std::clamp( static_cast<int>( *vcr ), 0, 255 ) ),
            static_cast<std::uint8_t>( std::clamp( static_cast<int>( *vcg ), 0, 255 ) ),
            static_cast<std::uint8_t>( std::clamp( static_cast<int>( *vcb ), 0, 255 ) ),
            255 };

        model::Material material( model::Color{ color.r, color.g, color.b, color.a },
                                  vrefl.value() );

        float radius = static_cast<float>( *vr );

        if ( mode_ == Mode::Create )
        {
            scene_manager_.addSphere( material, origin, radius );
        } else
        {
            if ( !obj_idx_.has_value() )
            {
                return;
            }

            auto& obj = scene_manager_.getObjects()[obj_idx_.value()];
            obj->setOrigin( origin );
            obj->setMaterial( material );

            if ( auto* sphere = dynamic_cast<model::Sphere*>( obj.get() ) )
            {
                sphere->setRadius( radius );
            }
        }

        scene_manager_.needUpdate() = true;
        close_cb();
    }

  private:
    zemax::model::SceneManager& scene_manager_;
    Mode                        mode_;
    std::optional<size_t>       obj_idx_;

    hui::Button ok_btn_;
    hui::Button cancel_btn_;

    // инпуты
    hui::InputText* x_field_      = nullptr;
    hui::InputText* y_field_      = nullptr;
    hui::InputText* z_field_      = nullptr;
    hui::InputText* radius_field_ = nullptr;
    hui::InputText* r_field_      = nullptr;
    hui::InputText* g_field_      = nullptr;
    hui::InputText* b_field_      = nullptr;
    hui::InputText* refl_field_   = nullptr;

    std::vector<std::unique_ptr<hui::InputText>> inputs_storage_;
    std::vector<std::unique_ptr<dr4::Text>>      labels_;
    std::vector<dr4::Vec2f>                      label_positions_;
};

} // namespace view
} // namespace zemax
