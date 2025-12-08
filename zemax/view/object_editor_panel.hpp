#pragma once

#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/input_text.hpp"
#include "custom-hui-impl/label.hpp"
#include "custom-hui-impl/scrollable_widget.hpp"
#include "zemax/model/primitives/impls/aabb.hpp"
#include "zemax/model/primitives/impls/hex_prism.hpp"
#include "zemax/model/primitives/impls/sphere.hpp"
#include "zemax/model/primitives/impls/torus.hpp"
#include "zemax/model/rendering/scene_manager.hpp"
#include "zemax/view/closable_panel.hpp"
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>

namespace zemax {
namespace view {

class ObjectEditorPanel : public ClosablePanel {
  public:
    explicit ObjectEditorPanel( hui::WindowManager*  wm,
                                model::SceneManager& scene_manager,
                                const dr4::Vec2f&    pos,
                                const dr4::Vec2f&    size )
        : ClosablePanel( wm, pos.x, pos.y, size.x, size.y, "Object Editor" ),
          scene_manager_( scene_manager ),
          apply_btn_( wm,
                      { 12.0f, size.y - 34.0f },
                      { 88.0f, 24.0f },
                      CloseBtnDefaultColor,
                      CloseBtnHoveredColor,
                      CloseBtnPressedColor,
                      "Apply",
                      CloseBtnFontColor,
                      CloseBtnFontSize )
    {
        setDraggable( true );
        buildForm( Type::None );

        apply_btn_.setParent( this );
        apply_btn_.setOnClick( [this]() { onApply(); } );
    }

    void
    setTarget( std::optional<size_t> idx )
    {
        target_idx_ = idx;
        if ( !idx.has_value() )
        {
            buildForm( Type::None );
            return;
        }

        auto info = scene_manager_.getObjectInfo( idx.value() );
        if ( info.type_name == "Sphere" )
        {
            buildForm( Type::Sphere );
        } else if ( info.type_name == "AABB" )
        {
            buildForm( Type::AABB );
        } else if ( info.type_name == "Torus" )
        {
            buildForm( Type::Torus );
        } else if ( info.type_name == "HexPrism" )
        {
            buildForm( Type::HexPrism );
        } else
        {
            buildForm( Type::None );
        }

        prefill( idx.value(), info );
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        if ( !visible_ )
        {
            return false;
        }

        if ( event.apply( scroll_.get() ) )
        {
            return true;
        }

        if ( event.apply( &apply_btn_ ) )
        {
            return true;
        }

        return ObjInfoBox::propagateEventToChildren( event );
    }

    void
    RedrawMyTexture() const override
    {
        if ( !visible_ )
        {
            return;
        }

        ObjInfoBox::RedrawMyTexture();

        if ( scroll_ )
        {
            scroll_->Redraw();
        }

        apply_btn_.Redraw();
    }

  private:
    enum class Type { None, Sphere, AABB, Torus, HexPrism };

    struct FieldRef
    {
        std::string                     key;
        std::unique_ptr<dr4::Text>      label;
        std::unique_ptr<hui::InputText> input;
        dr4::Vec2f                      label_pos;
    };

    class FormContent : public hui::ContainerWidget {
      public:
        explicit FormContent( hui::WindowManager* wm, const dr4::Vec2f& size )
            : ContainerWidget( wm, { 0, 0 }, size )
        {
        }

        bool
        propagateEventToChildren( const hui::Event& event ) override
        {
            for ( auto& f : fields_ )
            {
                if ( event.apply( f.input.get() ) )
                {
                    return true;
                }
            }
            return false;
        }

        void
        RedrawMyTexture() const override
        {
            texture_->Clear( { 0, 0, 0, 0 } );
            for ( auto& f : fields_ )
            {
                f.label->SetPos( f.label_pos );
                f.label->DrawOn( *texture_ );
                f.input->Redraw();
            }
        }

        void
        setFields( std::vector<FieldRef> fields )
        {
            fields_ = std::move( fields );
        }

        std::vector<FieldRef>&
        fields()
        {
            return fields_;
        }

      private:
        std::vector<FieldRef> fields_;
    };

    static std::string
    fmt2( double v )
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision( 2 ) << v;
        return ss.str();
    }

    void
    buildForm( Type type )
    {
        const float margin_x    = 12.0f;
        const float margin_top  = TopBarHeight + 10.0f;
        const float line_h      = 24.0f;
        const float label_w     = 120.0f;
        const float field_h     = 18.0f;
        const float field_w     = size_.x - margin_x - label_w - 20.0f;
        const float scrollbar_w = 12.0f;

        struct FieldDef
        {
            std::string label;
            std::string key;
        };

        std::vector<FieldDef> defs;
        defs.push_back( { "Name", "name" } );
        defs.push_back( { "X", "x" } );
        defs.push_back( { "Y", "y" } );
        defs.push_back( { "Z", "z" } );
        switch ( type )
        {
            case Type::Sphere:
                defs.push_back( { "Radius", "p1" } );
                break;
            case Type::AABB:
                defs.push_back( { "Half size X", "p1" } );
                defs.push_back( { "Half size Y", "p2" } );
                defs.push_back( { "Half size Z", "p3" } );
                break;
            case Type::Torus:
                defs.push_back( { "Major radius", "p1" } );
                defs.push_back( { "Minor radius", "p2" } );
                break;
            case Type::HexPrism:
                defs.push_back( { "Radius", "p1" } );
                defs.push_back( { "Height", "p2" } );
                break;
            case Type::None:
            default:
                break;
        }
        defs.push_back( { "R", "r" } );
        defs.push_back( { "G", "g" } );
        defs.push_back( { "B", "b" } );
        defs.push_back( { "Reflection", "f" } );

        float content_height = margin_top + defs.size() * line_h + 10.0f;
        auto  form           = std::make_unique<FormContent>(
            wm_,
            dr4::Vec2f{ size_.x - scrollbar_w,
                        std::max( size_.y - TopBarHeight, content_height ) } );

        auto* win  = wm_->getWindow();
        auto* font = win->GetDefaultFont();

        std::vector<FieldRef> fields;
        fields.reserve( defs.size() );

        float cur_y = margin_top;
        for ( auto& d : defs )
        {
            auto* t = win->CreateText();
            t->SetFont( font );
            t->SetFontSize( 12 );
            t->SetText( d.label );
            t->SetColor( { 220, 220, 220, 255 } );

            auto input = std::make_unique<hui::InputText>( wm_,
                                                           margin_x + label_w,
                                                           cur_y,
                                                           field_w,
                                                           field_h );
            input->setParent( form.get() );

            fields.push_back( { d.key,
                                std::unique_ptr<dr4::Text>( t ),
                                std::move( input ),
                                { margin_x, cur_y } } );
            cur_y += line_h;
        }

        form->setFields( std::move( fields ) );

        auto form_raw = form.get();
        scroll_ =
            std::make_unique<hui::ScrollableWidget>( wm_,
                                                     contentOffset(),
                                                     dr4::Vec2f{ size_.x, size_.y - TopBarHeight },
                                                     scrollbar_w );
        scroll_->setParent( this );
        scroll_->setContent( std::move( form ) );
        form_raw_   = form_raw;
        scroll_raw_ = scroll_.get();
    }

    FieldRef*
    findField( const std::string& key )
    {
        if ( !form_raw_ )
        {
            return nullptr;
        }

        for ( auto& f : form_raw_->fields() )
        {
            if ( f.key == key )
                return &f;
        }
        return nullptr;
    }

    void
    prefill( size_t idx, const model::SceneManager::ObjectInfo& info )
    {
        if ( auto* f = findField( "name" ) )
            f->input->setString( info.display_name.empty() ? info.type_name : info.display_name );
        if ( auto* f = findField( "x" ) )
            f->input->setString( fmt2( info.pos.x ) );
        if ( auto* f = findField( "y" ) )
            f->input->setString( fmt2( info.pos.y ) );
        if ( auto* f = findField( "z" ) )
            f->input->setString( fmt2( info.pos.z ) );
        if ( auto* f = findField( "r" ) )
            f->input->setString( std::to_string( info.material.color.r ) );
        if ( auto* f = findField( "g" ) )
            f->input->setString( std::to_string( info.material.color.g ) );
        if ( auto* f = findField( "b" ) )
            f->input->setString( std::to_string( info.material.color.b ) );
        if ( auto* f = findField( "f" ) )
            f->input->setString( fmt2( info.material.reflection_factor ) );

        auto& obj = scene_manager_.getObjects()[idx];
        if ( auto* sphere = dynamic_cast<model::Sphere*>( obj.get() ) )
        {
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( sphere->getRadius() ) );
        } else if ( auto* aabb = dynamic_cast<model::AABB*>( obj.get() ) )
        {
            auto hs = aabb->getHalfSize();
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( hs.x ) );
            if ( auto* f = findField( "p2" ) )
                f->input->setString( fmt2( hs.y ) );
            if ( auto* f = findField( "p3" ) )
                f->input->setString( fmt2( hs.z ) );
        } else if ( auto* torus = dynamic_cast<model::Torus*>( obj.get() ) )
        {
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( torus->getMajorRadius() ) );
            if ( auto* f = findField( "p2" ) )
                f->input->setString( fmt2( torus->getMinorRadius() ) );
        } else if ( auto* hex = dynamic_cast<model::HexPrism*>( obj.get() ) )
        {
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( hex->getRadius() ) );
            if ( auto* f = findField( "p2" ) )
                f->input->setString( fmt2( hex->getHeight() ) );
        }
    }

    template<typename Validator>
    static std::optional<double>
    parse( FieldRef* f, Validator v )
    {
        if ( !f )
            return std::nullopt;
        auto val = f->input->getDouble();
        if ( !val.has_value() || !v( *val ) )
        {
            f->input->setColor( { 255, 64, 64, 255 } );
            return std::nullopt;
        }
        f->input->setColor( { 255, 255, 255, 255 } );
        return *val;
    }

    void
    onApply()
    {
        if ( !target_idx_.has_value() || !form_raw_ )
        {
            return;
        }

        auto* f_name = findField( "name" );
        auto* fx     = findField( "x" );
        auto* fy     = findField( "y" );
        auto* fz     = findField( "z" );
        auto* fr     = findField( "r" );
        auto* fg     = findField( "g" );
        auto* fb     = findField( "b" );
        auto* ff     = findField( "f" );

        auto vx = parse( fx, []( double ) { return true; } );
        auto vy = parse( fy, []( double ) { return true; } );
        auto vz = parse( fz, []( double ) { return true; } );
        auto vr = parse( fr, []( double v ) { return v >= 0 && v <= 255; } );
        auto vg = parse( fg, []( double v ) { return v >= 0 && v <= 255; } );
        auto vb = parse( fb, []( double v ) { return v >= 0 && v <= 255; } );
        auto vf = parse( ff, []( double ) { return true; } );

        if ( !vx || !vy || !vz || !vr || !vg || !vb || !vf )
        {
            return;
        }

        model::Vector3f origin( static_cast<float>( *vx ),
                                static_cast<float>( *vy ),
                                static_cast<float>( *vz ) );
        model::Material material( model::Color( static_cast<std::uint8_t>( *vr ),
                                                static_cast<std::uint8_t>( *vg ),
                                                static_cast<std::uint8_t>( *vb ),
                                                255 ),
                                  static_cast<float>( *vf ) );

        auto& obj = scene_manager_.getObjects()[target_idx_.value()];
        obj->setOrigin( origin );
        obj->setMaterial( material );
        if ( f_name )
        {
            auto name_sv = f_name->input->getString();
            obj->setDisplayName( std::string( name_sv.value_or( "" ) ) );
        }

        if ( auto* sphere = dynamic_cast<model::Sphere*>( obj.get() ) )
        {
            auto p1 = parse( findField( "p1" ), []( double v ) { return v > 0; } );
            if ( p1 )
                sphere->setRadius( static_cast<float>( *p1 ) );
        } else if ( auto* aabb = dynamic_cast<model::AABB*>( obj.get() ) )
        {
            auto p1 = parse( findField( "p1" ), []( double v ) { return v > 0; } );
            auto p2 = parse( findField( "p2" ), []( double v ) { return v > 0; } );
            auto p3 = parse( findField( "p3" ), []( double v ) { return v > 0; } );
            if ( p1 && p2 && p3 )
            {
                aabb->setHalfSize( { static_cast<float>( *p1 ),
                                     static_cast<float>( *p2 ),
                                     static_cast<float>( *p3 ) } );
            }
        } else if ( auto* torus = dynamic_cast<model::Torus*>( obj.get() ) )
        {
            auto pMaj = parse( findField( "p1" ), []( double v ) { return v > 0; } );
            auto pMin = parse( findField( "p2" ), []( double v ) { return v > 0; } );
            if ( pMaj && pMin )
            {
                torus->setMajorRadius( static_cast<float>( *pMaj ) );
                torus->setMinorRadius( static_cast<float>( *pMin ) );
            }
        } else if ( auto* hex = dynamic_cast<model::HexPrism*>( obj.get() ) )
        {
            auto pr = parse( findField( "p1" ), []( double v ) { return v > 0; } );
            auto ph = parse( findField( "p2" ), []( double v ) { return v > 0; } );
            if ( pr && ph )
            {
                hex->setRadius( static_cast<float>( *pr ) );
                hex->setHeight( static_cast<float>( *ph ) );
            }
        }

        scene_manager_.needUpdate() = true;
    }

  private:
    model::SceneManager&  scene_manager_;
    std::optional<size_t> target_idx_;

    hui::ScrollableWidget*                 scroll_raw_ = nullptr;
    std::unique_ptr<hui::ScrollableWidget> scroll_;
    FormContent*                           form_raw_ = nullptr;
    hui::Button                            apply_btn_;
};

} // namespace view
} // namespace zemax
