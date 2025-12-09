#pragma once

#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/input_text.hpp"
#include "custom-hui-impl/label.hpp"
#include "custom-hui-impl/scrollable_widget.hpp"
#include "custom-hui-impl/closable_panel.hpp"
#include "zemax/config.hpp"
#include "zemax/model/primitives/impls/aabb.hpp"
#include "zemax/model/primitives/impls/hex_prism.hpp"
#include "zemax/model/primitives/impls/sphere.hpp"
#include "zemax/model/primitives/impls/torus.hpp"
#include "zemax/model/rendering/scene_manager.hpp"
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace zemax {
namespace view {

// Single scrollable editor for objects: create when none selected, edit/copy/delete when selected.
class ObjectEditorPanel : public hui::ClosablePanel {
  public:
    explicit ObjectEditorPanel( hui::WindowManager*  wm,
                                model::SceneManager& scene_manager,
                                const dr4::Vec2f&    pos,
                                const dr4::Vec2f&    size )
        : hui::ClosablePanel( wm, pos.x, pos.y, size.x, size.y, "Object Editor" ),
          scene_manager_( scene_manager ),
          add_btn_( wm,
                    { 12.0f, size.y - 34.0f },
                    { 88.0f, 24.0f },
                    CloseBtnDefaultColor,
                    CloseBtnHoveredColor,
                    CloseBtnPressedColor,
                    "Add",
                    CloseBtnFontColor,
                    CloseBtnFontSize ),
          copy_btn_( wm,
                     { 112.0f, size.y - 34.0f },
                     { 88.0f, 24.0f },
                     CloseBtnDefaultColor,
                     CloseBtnHoveredColor,
                     CloseBtnPressedColor,
                     "Copy",
                     CloseBtnFontColor,
                     CloseBtnFontSize ),
          del_btn_( wm,
                    { 212.0f, size.y - 34.0f },
                    { 88.0f, 24.0f },
                    CloseBtnDefaultColor,
                    CloseBtnHoveredColor,
                    CloseBtnPressedColor,
                    "Del",
                    CloseBtnFontColor,
                    CloseBtnFontSize ),
          apply_btn_( wm,
                      { size.x - 100.0f, size.y - 34.0f },
                      { 88.0f, 24.0f },
                      CloseBtnDefaultColor,
                      CloseBtnHoveredColor,
                      CloseBtnPressedColor,
                      "Apply",
                      CloseBtnFontColor,
                      CloseBtnFontSize )
    {
        setDraggable( true );
        addButtons();
        buildForm( current_type_ );
        prefillDefaults();
    }

    void
    setTarget( std::optional<size_t> idx )
    {
        target_idx_ = idx;
        auto info   = idx.has_value() ? std::optional<model::SceneManager::ObjectInfo>(
                                          scene_manager_.getObjectInfo( idx.value() ) )
                                      : std::nullopt;

        if ( info.has_value() )
        {
            current_type_ = typeFromName( info->type_name );
            buildForm( current_type_ );
            prefill( idx.value(), info.value() );
        } else
        {
            current_type_ = creation_type_;
            buildForm( current_type_ );
            prefillDefaults();
        }

        updateTypeButtons();
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        if ( !visible_ )
        {
            return false;
        }

        if ( event.apply( &add_btn_ ) )
            return true;
        if ( copy_visible_ && event.apply( &copy_btn_ ) )
            return true;
        if ( del_visible_ && event.apply( &del_btn_ ) )
            return true;
        for ( auto& btn : type_btns_ )
        {
            if ( event.apply( btn.get() ) )
                return true;
        }

        if ( event.apply( scroll_.get() ) )
        {
            return true;
        }

        if ( event.apply( &apply_btn_ ) )
        {
            return true;
        }

        return hui::DialogBox::propagateEventToChildren( event );
    }

    void
    RedrawMyTexture() const override
    {
        if ( !visible_ )
        {
            return;
        }

        hui::DialogBox::RedrawMyTexture();

        if ( scroll_ )
        {
            scroll_->Redraw();
        }

        for ( const auto& btn : type_btns_ )
        {
            btn->Redraw();
        }

        add_btn_.Redraw();
        if ( copy_visible_ )
            copy_btn_.Redraw();
        if ( del_visible_ )
            del_btn_.Redraw();
        apply_btn_.Redraw();
    }

  private:
    enum class Type { Sphere, AABB, Torus, HexPrism };

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
        const float margin_top  = TopBarHeight + type_row_height_ + 14.0f;
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
            t->SetFontSize( 15 );
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

    struct CommonFields
    {
        std::string     name;
        model::Vector3f origin;
        model::Material material;
    };

    std::optional<CommonFields>
    parseCommonFields()
    {
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
            return std::nullopt;
        }

        CommonFields res{ f_name ? std::string( f_name->input->getString().value_or( "" ) )
                                 : std::string(),
                          model::Vector3f( static_cast<float>( *vx ),
                                           static_cast<float>( *vy ),
                                           static_cast<float>( *vz ) ),
                          model::Material( model::Color( static_cast<std::uint8_t>( *vr ),
                                                         static_cast<std::uint8_t>( *vg ),
                                                         static_cast<std::uint8_t>( *vb ),
                                                         255 ),
                                           static_cast<float>( *vf ) ) };
        return res;
    }

    void
    onApply()
    {
        if ( !form_raw_ )
        {
            return;
        }

        auto common = parseCommonFields();
        if ( !common.has_value() )
        {
            return;
        }

        if ( target_idx_.has_value() )
        {
            applyToExisting( *common );
        } else
        {
            addNewObject( *common );
        }
    }

    void
    applyToExisting( const CommonFields& common )
    {
        auto& obj = scene_manager_.getObjects()[target_idx_.value()];
        obj->setOrigin( common.origin );
        obj->setMaterial( common.material );
        obj->setDisplayName( common.name );

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

    void
    addNewObjectFromUI()
    {
        auto common = parseCommonFields();
        if ( !common.has_value() )
        {
            return;
        }
        addNewObject( *common );
    }

    static Type
    typeFromName( const std::string& name )
    {
        if ( name == "Sphere" )
            return Type::Sphere;
        if ( name == "AABB" )
            return Type::AABB;
        if ( name == "Torus" )
            return Type::Torus;
        return Type::HexPrism;
    }

    void
    prefillDefaults()
    {
        if ( auto* f = findField( "name" ) )
            f->input->setString( defaultNameForType( current_type_ ) );
        if ( auto* f = findField( "x" ) )
            f->input->setString( "0.00" );
        if ( auto* f = findField( "y" ) )
            f->input->setString( "0.00" );
        if ( auto* f = findField( "z" ) )
            f->input->setString( "-10.00" );
        if ( auto* f = findField( "r" ) )
            f->input->setString( "118" );
        if ( auto* f = findField( "g" ) )
            f->input->setString( "185" );
        if ( auto* f = findField( "b" ) )
            f->input->setString( "0" );
        if ( auto* f = findField( "f" ) )
            f->input->setString( "0.50" );

        switch ( current_type_ )
        {
            case Type::Sphere:
                if ( auto* f = findField( "p1" ) )
                    f->input->setString( "1.00" );
                break;
            case Type::AABB:
                if ( auto* f = findField( "p1" ) )
                    f->input->setString( "1.00" );
                if ( auto* f = findField( "p2" ) )
                    f->input->setString( "1.00" );
                if ( auto* f = findField( "p3" ) )
                    f->input->setString( "1.00" );
                break;
            case Type::Torus:
                if ( auto* f = findField( "p1" ) )
                    f->input->setString( "2.00" );
                if ( auto* f = findField( "p2" ) )
                    f->input->setString( "0.80" );
                break;
            case Type::HexPrism:
                if ( auto* f = findField( "p1" ) )
                    f->input->setString( "1.00" );
                if ( auto* f = findField( "p2" ) )
                    f->input->setString( "2.00" );
                break;
        }
    }

    std::string
    defaultNameForType( Type t )
    {
        switch ( t )
        {
            case Type::Sphere:
                return "Sphere";
            case Type::AABB:
                return "AABB";
            case Type::Torus:
                return "Torus";
            case Type::HexPrism:
            default:
                return "HexPrism";
        }
    }

    void
    addNewObject( const CommonFields& common )
    {
        float p1 = parse( findField( "p1" ), []( double v ) { return v > 0; } ).value_or( 1.0 );
        float p2 = parse( findField( "p2" ), []( double v ) { return v > 0; } ).value_or( 1.0 );
        float p3 = parse( findField( "p3" ), []( double v ) { return v > 0; } ).value_or( 1.0 );

        model::Primitive* created = nullptr;
        switch ( current_type_ )
        {
            case Type::Sphere:
                scene_manager_.addSphere( common.material, common.origin, p1 );
                break;
            case Type::AABB:
                scene_manager_.addAABB( common.material, common.origin, { p1, p2, p3 } );
                break;
            case Type::Torus:
                // UI shows major radius first, but addTorus expects (minor, major)
                scene_manager_.addTorus( common.material, common.origin, p2, p1 );
                break;
            case Type::HexPrism:
                scene_manager_.addHexPrism( common.material, common.origin, p1, p2 );
                break;
        }

        if ( !scene_manager_.getObjects().empty() )
        {
            created = scene_manager_.getObjects().back().get();
        }

        created->setDisplayName( common.name );
        scene_manager_.setTargetObj( created );
        target_idx_                 = scene_manager_.getObjects().size() - 1;
        scene_manager_.needUpdate() = true;
        prefill( target_idx_.value(), scene_manager_.getObjectInfo( target_idx_.value() ) );
        updateTypeButtons();
    }

    void
    copyTarget()
    {
        if ( !target_idx_.has_value() )
            return;
        auto* target = scene_manager_.getTargetObj();
        if ( !target )
            return;
        auto  origin = target->getOrigin();
        float dx     = Config::Camera::ObjMoveFactor * 2.0f;
        scene_manager_.copyTargetObj( origin.x + dx, origin.y, origin.z );
        target_idx_ = scene_manager_.getObjects().size() - 1;
        scene_manager_.setTargetObj( scene_manager_.getObjects().back().get() );
        current_type_ =
            typeFromName( scene_manager_.getObjectInfo( target_idx_.value() ).type_name );
        buildForm( current_type_ );
        prefill( target_idx_.value(), scene_manager_.getObjectInfo( target_idx_.value() ) );
        updateTypeButtons();
    }

    void
    deleteTarget()
    {
        if ( !target_idx_.has_value() )
            return;
        scene_manager_.deleteTargetObj();
        scene_manager_.needUpdate() = true;
        target_idx_.reset();
        current_type_ = creation_type_;
        buildForm( current_type_ );
        updateTypeButtons();
        prefillDefaults();
    }

    void
    addButtons()
    {
        add_btn_.setParent( this );
        copy_btn_.setParent( this );
        del_btn_.setParent( this );
        apply_btn_.setParent( this );

        add_btn_.setOnClick( [this]() { addNewObjectFromUI(); } );
        copy_btn_.setOnClick( [this]() { copyTarget(); } );
        del_btn_.setOnClick( [this]() { deleteTarget(); } );
        apply_btn_.setOnClick( [this]() { onApply(); } );

        const float btn_w   = 70.0f;
        const float btn_h   = 22.0f;
        const float btn_gap = 8.0f;
        const float base_y  = TopBarHeight + 6.0f;
        float       x       = 12.0f;

        auto make_btn = [&]( const char* label, Type type ) {
            auto btn = std::make_unique<hui::Button>( wm_,
                                                      dr4::Vec2f{ x, base_y },
                                                      dr4::Vec2f{ btn_w, btn_h },
                                                      CloseBtnDefaultColor,
                                                      CloseBtnHoveredColor,
                                                      CloseBtnPressedColor,
                                                      label,
                                                      CloseBtnFontColor,
                                                      CloseBtnFontSize );
            btn->setParent( this );
            btn->setOnClick( [this, type]() {
                if ( target_idx_.has_value() )
                    return;
                creation_type_ = type;
                current_type_  = type;
                buildForm( current_type_ );
                updateTypeButtons();
                prefillDefaults();
            } );
            x += btn_w + btn_gap;
            return btn;
        };

        type_btns_.push_back( make_btn( "Sphere", Type::Sphere ) );
        type_btns_.push_back( make_btn( "AABB", Type::AABB ) );
        type_btns_.push_back( make_btn( "Torus", Type::Torus ) );
        type_btns_.push_back( make_btn( "Hex", Type::HexPrism ) );

        updateTypeButtons();
    }

    void
    updateTypeButtons()
    {
        copy_visible_ = target_idx_.has_value();
        del_visible_  = target_idx_.has_value();
    }

  private:
    model::SceneManager&  scene_manager_;
    std::optional<size_t> target_idx_;

    hui::ScrollableWidget*                    scroll_raw_ = nullptr;
    std::unique_ptr<hui::ScrollableWidget>    scroll_;
    FormContent*                              form_raw_        = nullptr;
    Type                                      current_type_    = Type::Sphere;
    Type                                      creation_type_   = Type::Sphere;
    const float                               type_row_height_ = 28.0f;
    std::vector<std::unique_ptr<hui::Button>> type_btns_;
    bool                                      copy_visible_ = false;
    bool                                      del_visible_  = false;
    hui::Button                               add_btn_;
    hui::Button                               copy_btn_;
    hui::Button                               del_btn_;
    hui::Button                               apply_btn_;
};

} // namespace view
} // namespace zemax
