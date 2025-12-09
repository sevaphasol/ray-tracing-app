#pragma once

#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/input_text.hpp"
#include "custom-hui-impl/label.hpp"
#include "custom-hui-impl/closable_panel.hpp"
#include "custom-hui-impl/scrollable_list_widget.hpp"
#include "custom-hui-impl/dialog_box.hpp"
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

// Single editor for objects: create when none selected, edit/copy/delete when selected.
class ObjectEditorPanel : public hui::ClosablePanel {
  public:
    explicit ObjectEditorPanel( hui::WindowManager*  wm,
                                model::SceneManager& scene_manager,
                                const dr4::Vec2f&    pos,
                                const dr4::Vec2f&    size )
        : hui::ClosablePanel( wm, pos.x, pos.y, size.x, size.y, "Object Editor" ),
          scene_manager_( scene_manager ),
          label_text_( wm->getWindow()->CreateText() ),
          type_btn_( wm,
                     { 12.0f, size.y - 34.0f },
                     { 88.0f, 24.0f },
                     CloseBtnDefaultColor,
                     CloseBtnHoveredColor,
                     CloseBtnPressedColor,
                     "Type",
                     CloseBtnFontColor,
                     CloseBtnFontSize ),
          add_btn_( wm,
                    { 112.0f, size.y - 34.0f },
                    { 88.0f, 24.0f },
                    CloseBtnDefaultColor,
                    CloseBtnHoveredColor,
                    CloseBtnPressedColor,
                    "Add",
                    CloseBtnFontColor,
                    CloseBtnFontSize ),
          copy_btn_( wm,
                     { 12.0f, size.y - 34.0f },
                     { 88.0f, 24.0f },
                     CloseBtnDefaultColor,
                     CloseBtnHoveredColor,
                     CloseBtnPressedColor,
                     "Copy",
                     CloseBtnFontColor,
                     CloseBtnFontSize ),
          del_btn_( wm,
                    { 112.0f, size.y - 34.0f },
                    { 88.0f, 24.0f },
                    CloseBtnDefaultColor,
                    CloseBtnHoveredColor,
                    CloseBtnPressedColor,
                    "Delete",
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
        buildForm( current_type_ );
        prefillDefaults();
        wireButtons();
    }

    void
    setTarget( std::optional<size_t> idx )
    {
        target_idx_ = idx;
        editing_mode_ = idx.has_value();
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

        if ( editing_mode_ )
        {
            if ( event.apply( &copy_btn_ ) )
                return true;
            if ( event.apply( &del_btn_ ) )
                return true;
        } else
        {
            if ( event.apply( &type_btn_ ) )
                return true;
            if ( event.apply( &add_btn_ ) )
                return true;
        }

        for ( auto& f : form_fields_ )
        {
            if ( event.apply( f.input.get() ) )
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

        for ( auto& f : form_fields_ )
        {
            f.label->SetPos( f.label_pos );
            f.label->DrawOn( *texture_ );
            f.input->Redraw();
        }

        if ( editing_mode_ )
        {
            copy_btn_.Redraw();
            del_btn_.Redraw();
        } else
        {
            type_btn_.Redraw();
            add_btn_.Redraw();
        }
        apply_btn_.Redraw();

        auto* font = wm_->getWindow()->GetDefaultFont();
        label_text_->SetFont( font );
        label_text_->SetFontSize( 14 );
        label_text_->SetColor( { 220, 220, 220, 255 } );
        label_text_->SetText( editing_mode_ ? "Editing" : ("Adding " + typeName( current_type_ )) );
        label_text_->SetPos( { 12.0f, TopBarHeight + 4.0f } );
        label_text_->DrawOn( *texture_ );
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

    static std::string
    fmt2( double v )
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision( 2 ) << v;
        return ss.str();
    }

    static std::string
    typeName( Type t )
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
        defs.push_back( { "Refraction", "refract" } );
        defs.push_back( { "Eta", "eta" } );

        auto* win  = wm_->getWindow();
        auto* font = win->GetDefaultFont();

        form_fields_.clear();
        form_fields_.reserve( defs.size() );

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
            input->setParent( this );

            form_fields_.push_back(
                { d.key, std::unique_ptr<dr4::Text>( t ), std::move( input ), { margin_x, cur_y } } );
            cur_y += line_h;
        }
    }

    FieldRef*
    findField( const std::string& key )
    {
        for ( auto& f : form_fields_ )
        {
            if ( f.key == key )
                return &f;
        }
        return nullptr;
    }

    void
    prefill( size_t idx, const model::SceneManager::ObjectInfo& info )
    {
        editing_mode_ = true;
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
        if ( auto* f = findField( "refract" ) )
            f->input->setString( fmt2( info.material.refraction_factor ) );
        if ( auto* f = findField( "eta" ) )
            f->input->setString( fmt2( info.material.refraction_eta ) );

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
        auto* fre    = findField( "refract" );
        auto* feta   = findField( "eta" );

        auto vx = parse( fx, []( double ) { return true; } );
        auto vy = parse( fy, []( double ) { return true; } );
        auto vz = parse( fz, []( double ) { return true; } );
        auto vr = parse( fr, []( double v ) { return v >= 0 && v <= 255; } );
        auto vg = parse( fg, []( double v ) { return v >= 0 && v <= 255; } );
        auto vb = parse( fb, []( double v ) { return v >= 0 && v <= 255; } );
        auto vf = parse( ff, []( double ) { return true; } );
        auto vref = parse( fre, []( double v ) { return v >= 0.0 && v <= 1.0; } );
        auto veta = parse( feta, []( double v ) { return v > 0.0; } );

        if ( !vx || !vy || !vz || !vr || !vg || !vb || !vf || !vref || !veta )
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
                                           static_cast<float>( *vf ),
                                           static_cast<float>( *vref ),
                                           static_cast<float>( *veta ) ) };
        return res;
    }

    void
    onApply()
    {
        if ( form_fields_.empty() )
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
        editing_mode_ = false;
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
        if ( auto* f = findField( "refract" ) )
            f->input->setString( "0.00" );
        if ( auto* f = findField( "eta" ) )
            f->input->setString( "1.00" );

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

    class TypePickerDialog : public hui::DialogBox {
      public:
        TypePickerDialog( hui::WindowManager* wm,
                          float               x,
                          float               y,
                          std::function<void(Type)> on_ok,
                          std::function<void()>     on_cancel )
            : DialogBox( wm, x, y, 220.0f, 260.0f, on_cancel, "Choose Type" ),
              list_( wm, { 10.0f, TopBarHeight + 10.0f }, { 200.0f, 180.0f }, 10.0f ),
              ok_( wm, { 20.0f, 210.0f }, { 80.0f, 26.0f } ),
              cancel_( wm, { 120.0f, 210.0f }, { 80.0f, 26.0f } ),
              on_ok_( std::move( on_ok ) ),
              on_cancel_( std::move( on_cancel ) )
        {
            list_.setParent( this );
            ok_.setParent( this );
            cancel_.setParent( this );

            ok_.setOnClick( [this]() {
                if ( on_ok_ )
                    on_ok_( selected_ );
                if ( on_cancel_ )
                    on_cancel_();
            } );
            cancel_.setOnClick( [this]() {
                if ( on_cancel_ )
                    on_cancel_();
            } );

            buildItems();
        }

        bool
        propagateEventToChildren( const hui::Event& event ) override
        {
            if ( event.apply( &list_ ) )
                return true;
            if ( event.apply( &ok_ ) )
                return true;
            if ( event.apply( &cancel_ ) )
                return true;
            return DialogBox::propagateEventToChildren( event );
        }

        void
        RedrawMyTexture() const override
        {
            DialogBox::RedrawMyTexture();
            list_.Redraw();
            ok_.Redraw();
            cancel_.Redraw();
        }

      private:
        void
        buildItems()
        {
            list_.clearItems();
            auto add = [&]( const std::string& label, Type t ) {
                auto btn = std::make_unique<hui::Button>( wm_,
                                                          dr4::Vec2f{ 0, 0 },
                                                          dr4::Vec2f{ 180.0f, 32.0f },
                                                          t == selected_ ? CloseBtnPressedColor
                                                                         : CloseBtnDefaultColor,
                                                          CloseBtnHoveredColor,
                                                          CloseBtnPressedColor,
                                                          label,
                                                          CloseBtnFontColor,
                                                          CloseBtnFontSize );
                btn->setOnClick( [this, t]() {
                    selected_ = t;
                    buildItems();
                } );
                list_.addItem( std::move( btn ) );
            };

            add( "Sphere", Type::Sphere );
            add( "AABB", Type::AABB );
            add( "Torus", Type::Torus );
            add( "HexPrism", Type::HexPrism );
        }

      private:
        hui::ScrollableListWidget    list_;
        hui::ButtonOk                ok_;
        hui::ButtonCancel            cancel_;
        std::function<void(Type)>    on_ok_;
        std::function<void()>        on_cancel_;
        Type                         selected_ = Type::Sphere;
    };

    void
    updateTypeButtons()
    {
        copy_visible_ = target_idx_.has_value();
        del_visible_  = target_idx_.has_value();
    }

    void
    wireButtons()
    {
        type_btn_.setParent( this );
        add_btn_.setParent( this );
        copy_btn_.setParent( this );
        del_btn_.setParent( this );
        apply_btn_.setParent( this );

        add_btn_.setOnClick( [this]() { addNewObjectFromUI(); } );
        type_btn_.setOnClick( [this]() {
            wm_->pushModal( std::make_unique<TypePickerDialog>(
                wm_,
                getRelPos().x + 20.0f,
                getRelPos().y + 20.0f,
                [this]( Type t ) {
                    current_type_  = t;
                    creation_type_ = t;
                    buildForm( current_type_ );
                    prefillDefaults();
                    updateTypeButtons();
                },
                [this]() { wm_->popModal(); } ) );
        } );
        copy_btn_.setOnClick( [this]() { copyTarget(); } );
        del_btn_.setOnClick( [this]() { deleteTarget(); } );
        apply_btn_.setOnClick( [this]() { onApply(); } );
    }

  private:
    model::SceneManager&  scene_manager_;
    std::optional<size_t> target_idx_;
    std::vector<FieldRef> form_fields_;
    Type                        current_type_    = Type::Sphere;
    Type                        creation_type_   = Type::Sphere;
    const float                 type_row_height_ = 28.0f;
    bool                        copy_visible_    = false;
    bool                        del_visible_     = false;
    bool                        editing_mode_    = false;
    hui::Button                 type_btn_;
    hui::Button                 add_btn_;
    hui::Button                 copy_btn_;
    hui::Button                 del_btn_;
    hui::Button                 apply_btn_;
    std::unique_ptr<dr4::Text>  label_text_;
};

} // namespace view
} // namespace zemax
