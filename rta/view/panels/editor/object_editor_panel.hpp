#pragma once

#include "hui/button.hpp"
#include "hui/button_cancel.hpp"
#include "hui/button_ok.hpp"
#include "hui/closable_panel.hpp"
#include "hui/dialog_box.hpp"
#include "hui/input_text.hpp"
#include "hui/label.hpp"
#include "hui/scrollable_buttons_list_widget.hpp"
#include "hui/slider.hpp"
#include "rta/model/primitives/impls/aabb.hpp"
#include "rta/model/primitives/impls/capped_cone.hpp"
#include "rta/model/primitives/impls/capped_cylinder.hpp"
#include "rta/model/primitives/impls/capsule.hpp"
#include "rta/model/primitives/impls/ellipse.hpp"
#include "rta/model/primitives/impls/ellipsoid.hpp"
#include "rta/model/primitives/impls/goursat.hpp"
#include "rta/model/primitives/impls/hex_prism.hpp"
#include "rta/model/primitives/impls/rounded_box.hpp"
#include "rta/model/primitives/impls/rounded_cone.hpp"
#include "rta/model/primitives/impls/sphere.hpp"
#include "rta/model/primitives/impls/torus.hpp"
#include "rta/model/primitives/impls/triangle.hpp"
#include "rta/model/primitives/impls/wedge.hpp"
#include "rta/model/rendering/scene_manager.hpp"
#include "rta/view/utils/rgb_picker.hpp"
#include <algorithm>
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace rta {
namespace view {

class ObjectEditorPanel : public hui::ClosablePanel {
  public:
    explicit ObjectEditorPanel( hui::WindowManager*   wm,
                                model::SceneManager&  scene_manager,
                                const dr4::Vec2f&     pos,
                                const dr4::Vec2f&     size,
                                std::function<void()> objects_changed_cb = nullptr )
        : hui::ClosablePanel( wm, pos.x, pos.y, size.x, size.y, "Object Editor" ),
          scene_manager_( scene_manager ),
          objects_changed_cb_( std::move( objects_changed_cb ) ),
          label_text_( wm->getWindow()->CreateText() ),
          type_btn_( wm, { 12.0f, size.y - 34.0f }, { 88.0f, 24.0f }, "Type" ),
          add_btn_( wm, { 112.0f, size.y - 34.0f }, { 88.0f, 24.0f }, "Add" ),
          copy_btn_( wm, { 12.0f, size.y - 34.0f }, { 88.0f, 24.0f }, "Copy" ),
          del_btn_( wm, { 112.0f, size.y - 34.0f }, { 88.0f, 24.0f }, "Delete" ),
          apply_btn_( wm, { size.x - 100.0f, size.y - 34.0f }, { 88.0f, 24.0f }, "Apply" ),
          color_picker_( wm,
                         { 20.0f, TopBarHeight + 350.0f },
                         dr4::Color{ 118, 185, 0, 255 },
                         [this]( const dr4::Color& c ) { current_color_ = c; } ),
          slider_reflection_( wm, { 0, 0 }, { size.x - 150.0f, 14.0f } ),
          slider_refraction_( wm, { 0, 0 }, { size.x - 150.0f, 14.0f } ),
          slider_eta_( wm, { 0, 0 }, { size.x - 150.0f, 14.0f } )
    {
        setDraggable( true );
        color_picker_.setParent( this );
        color_label_.reset( wm->getWindow()->CreateText() );
        color_label_->SetFont( wm->getWindow()->GetDefaultFont() );
        color_label_->SetFontSize( 13 );
        color_label_->SetColor( { 220, 220, 220, 255 } );
        // color_label_->SetText( "Color" );
        color_label_->SetPos( { size.x - 180.0f, TopBarHeight - 2.0f } );
        slider_reflection_.setParent( this );
        slider_refraction_.setParent( this );
        slider_eta_.setParent( this );
        slider_reflection_.setOnChange( [this]( float ) { updateSliderLabels(); } );
        slider_refraction_.setOnChange( [this]( float ) { updateSliderLabels(); } );
        slider_eta_.setOnChange( [this]( float ) { updateSliderLabels(); } );

        auto mkText = [wm]() {
            auto* t = wm->getWindow()->CreateText();
            t->SetFont( wm->getWindow()->GetDefaultFont() );
            t->SetFontSize( 13 );
            t->SetColor( { 220, 220, 220, 255 } );
            return std::unique_ptr<dr4::Text>( t );
        };
        reflection_label_ = mkText();
        refraction_label_ = mkText();
        eta_label_        = mkText();
        reflection_value_ = mkText();
        refraction_value_ = mkText();
        eta_value_        = mkText();
        reflection_label_->SetText( "Reflection" );
        refraction_label_->SetText( "Refraction" );
        eta_label_->SetText( "Eta" );
        buildForm( current_type_ );
        prefillDefaults();
        wireButtons();
    }

    void
    setTarget( std::optional<size_t> idx )
    {
        target_idx_   = idx;
        editing_mode_ = idx.has_value();
        auto info     = idx.has_value() ? std::optional<model::SceneManager::ObjectInfo>(
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
        if ( event.apply( &slider_reflection_ ) )
            return true;
        if ( event.apply( &slider_refraction_ ) )
            return true;
        if ( event.apply( &slider_eta_ ) )
            return true;

        if ( event.apply( &color_picker_ ) )
            return true;

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
        if ( color_label_ )
            color_label_->DrawOn( *texture_ );
        color_picker_.Redraw();

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
        label_text_->SetText( editing_mode_ ? "Editing"
                                            : ( "Adding " + typeName( current_type_ ) ) );
        label_text_->SetPos( { 12.0f, TopBarHeight + 4.0f } );
        label_text_->DrawOn( *texture_ );

        if ( reflection_label_ )
        {
            reflection_label_->DrawOn( *texture_ );
            refraction_label_->DrawOn( *texture_ );
            eta_label_->DrawOn( *texture_ );
            reflection_value_->DrawOn( *texture_ );
            refraction_value_->DrawOn( *texture_ );
            eta_value_->DrawOn( *texture_ );
        }

        slider_reflection_.Redraw();
        slider_refraction_.Redraw();
        slider_eta_.Redraw();
    }

  private:
    enum class Type {
        Sphere,
        AABB,
        Torus,
        HexPrism,
        Goursat,
        RoundedBox,
        Ellipsoid,
        Capsule,
        RoundedCone,
        CappedCone,
        CappedCylinder,
        Wedge,
        Ellipse,
        Triangle
    };

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

    static float
    clamp01( float v )
    {
        if ( v < 0.0f )
            return 0.0f;
        if ( v > 1.0f )
            return 1.0f;
        return v;
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
                return "HexPrism";
            case Type::Goursat:
                return "Goursat";
            case Type::RoundedBox:
                return "RoundedBox";
            case Type::Ellipsoid:
                return "Ellipsoid";
            case Type::Capsule:
                return "Capsule";
            case Type::RoundedCone:
                return "RoundedCone";
            case Type::CappedCone:
                return "CappedCone";
            case Type::CappedCylinder:
                return "CappedCylinder";
            case Type::Wedge:
                return "Wedge";
            case Type::Ellipse:
                return "Ellipse";
            case Type::Triangle:
            default:
                return "Triangle";
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
            case Type::Goursat:
                defs.push_back( { "ka", "ka" } );
                defs.push_back( { "kb", "kb" } );
                break;
            case Type::RoundedBox:
                defs.push_back( { "Half size X", "p1" } );
                defs.push_back( { "Half size Y", "p2" } );
                defs.push_back( { "Half size Z", "p3" } );
                defs.push_back( { "Radius", "p4" } );
                break;
            case Type::Ellipsoid:
                defs.push_back( { "Radius X", "p1" } );
                defs.push_back( { "Radius Y", "p2" } );
                defs.push_back( { "Radius Z", "p3" } );
                break;
            case Type::Capsule:
                defs.push_back( { "Height", "p1" } );
                defs.push_back( { "Radius", "p2" } );
                break;
            case Type::RoundedCone:
                defs.push_back( { "Height", "p1" } );
                defs.push_back( { "Radius A", "p2" } );
                defs.push_back( { "Radius B", "p3" } );
                break;
            case Type::CappedCone:
                defs.push_back( { "Height", "p1" } );
                defs.push_back( { "Radius A", "p2" } );
                defs.push_back( { "Radius B", "p3" } );
                break;
            case Type::CappedCylinder:
                defs.push_back( { "Height", "p1" } );
                defs.push_back( { "Radius", "p2" } );
                break;
            case Type::Wedge:
                defs.push_back( { "Half size X", "p1" } );
                defs.push_back( { "Half size Y", "p2" } );
                defs.push_back( { "Half size Z", "p3" } );
                break;
            case Type::Ellipse:
                defs.push_back( { "U.x", "ux" } );
                defs.push_back( { "U.y", "uy" } );
                defs.push_back( { "U.z", "uz" } );
                defs.push_back( { "V.x", "vx" } );
                defs.push_back( { "V.y", "vy" } );
                defs.push_back( { "V.z", "vz" } );
                break;
            case Type::Triangle:
                defs.pop_back();
                defs.pop_back();
                defs.pop_back();
                defs.push_back( { "V0.x", "v0x" } );
                defs.push_back( { "V0.y", "v0y" } );
                defs.push_back( { "V0.z", "v0z" } );
                defs.push_back( { "V1.x", "v1x" } );
                defs.push_back( { "V1.y", "v1y" } );
                defs.push_back( { "V1.z", "v1z" } );
                defs.push_back( { "V2.x", "v2x" } );
                defs.push_back( { "V2.y", "v2y" } );
                defs.push_back( { "V2.z", "v2z" } );
                break;
        }

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

            form_fields_.push_back( { d.key,
                                      std::unique_ptr<dr4::Text>( t ),
                                      std::move( input ),
                                      { margin_x, cur_y } } );
            cur_y += line_h;
        }

        slider_origin_y_ = cur_y + 6.0f;
        layoutSliders();
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
        dr4::Color c{ info.material.color.r,
                      info.material.color.g,
                      info.material.color.b,
                      info.material.color.a };
        color_picker_.setColor( c );
        current_color_ = c;
        slider_reflection_.setFactor( clamp01( info.material.reflection_factor ) );
        slider_refraction_.setFactor( clamp01( info.material.refraction_factor ) );
        slider_eta_.setFactor(
            clamp01( ( info.material.refraction_eta - eta_min_ ) / ( eta_max_ - eta_min_ ) ) );
        updateSliderLabels();

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
        } else if ( auto* g = dynamic_cast<model::Goursat*>( obj.get() ) )
        {
            if ( auto* f = findField( "ka" ) )
                f->input->setString( fmt2( g->getKa() ) );
            if ( auto* f = findField( "kb" ) )
                f->input->setString( fmt2( g->getKb() ) );
        } else if ( auto* rb = dynamic_cast<model::RoundedBox*>( obj.get() ) )
        {
            auto hs = rb->getHalfSize();
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( hs.x ) );
            if ( auto* f = findField( "p2" ) )
                f->input->setString( fmt2( hs.y ) );
            if ( auto* f = findField( "p3" ) )
                f->input->setString( fmt2( hs.z ) );
            if ( auto* f = findField( "p4" ) )
                f->input->setString( fmt2( rb->getRadius() ) );
        } else if ( auto* el = dynamic_cast<model::Ellipsoid*>( obj.get() ) )
        {
            auto r = el->getRadii();
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( r.x ) );
            if ( auto* f = findField( "p2" ) )
                f->input->setString( fmt2( r.y ) );
            if ( auto* f = findField( "p3" ) )
                f->input->setString( fmt2( r.z ) );
        } else if ( auto* cap = dynamic_cast<model::Capsule*>( obj.get() ) )
        {
            auto  pa = cap->getPaLocal();
            auto  pb = cap->getPbLocal();
            float h  = std::abs( ( pb - pa ).y );
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( h ) );
            if ( auto* f = findField( "p2" ) )
                f->input->setString( fmt2( cap->getRadius() ) );
        } else if ( auto* rc = dynamic_cast<model::RoundedCone*>( obj.get() ) )
        {
            auto  pa = rc->getPaLocal();
            auto  pb = rc->getPbLocal();
            float h  = std::abs( ( pb - pa ).y );
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( h ) );
            if ( auto* f = findField( "p2" ) )
                f->input->setString( fmt2( rc->getRadiusA() ) );
            if ( auto* f = findField( "p3" ) )
                f->input->setString( fmt2( rc->getRadiusB() ) );
        } else if ( auto* cc = dynamic_cast<model::CappedCone*>( obj.get() ) )
        {
            auto  pa = cc->getPaLocal();
            auto  pb = cc->getPbLocal();
            float h  = std::abs( ( pb - pa ).y );
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( h ) );
            if ( auto* f = findField( "p2" ) )
                f->input->setString( fmt2( cc->getRadiusA() ) );
            if ( auto* f = findField( "p3" ) )
                f->input->setString( fmt2( cc->getRadiusB() ) );
        } else if ( auto* cyl = dynamic_cast<model::CappedCylinder*>( obj.get() ) )
        {
            auto  a = cyl->getALocal();
            auto  b = cyl->getBLocal();
            float h = std::abs( ( b - a ).y );
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( h ) );
            if ( auto* f = findField( "p2" ) )
                f->input->setString( fmt2( cyl->getRadius() ) );
        } else if ( auto* w = dynamic_cast<model::Wedge*>( obj.get() ) )
        {
            auto hs = w->getS();
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( hs.x ) );
            if ( auto* f = findField( "p2" ) )
                f->input->setString( fmt2( hs.y ) );
            if ( auto* f = findField( "p3" ) )
                f->input->setString( fmt2( hs.z ) );
        } else if ( auto* e = dynamic_cast<model::Ellipse*>( obj.get() ) )
        {
            auto u = e->getU();
            auto v = e->getV();
            if ( auto* f = findField( "ux" ) )
                f->input->setString( fmt2( u.x ) );
            if ( auto* f = findField( "uy" ) )
                f->input->setString( fmt2( u.y ) );
            if ( auto* f = findField( "uz" ) )
                f->input->setString( fmt2( u.z ) );
            if ( auto* f = findField( "vx" ) )
                f->input->setString( fmt2( v.x ) );
            if ( auto* f = findField( "vy" ) )
                f->input->setString( fmt2( v.y ) );
            if ( auto* f = findField( "vz" ) )
                f->input->setString( fmt2( v.z ) );
        } else if ( auto* t = dynamic_cast<model::Triangle*>( obj.get() ) )
        {
            auto v0  = t->getV0();
            auto v1  = t->getV1();
            auto v2  = t->getV2();
            auto set = [&]( const char* key, const model::Vector3f& v, int idx ) {
                if ( auto* f = findField( key ) )
                {
                    float val = idx == 0 ? v.x : ( idx == 1 ? v.y : v.z );
                    f->input->setString( fmt2( val ) );
                }
            };
            set( "v0x", v0, 0 );
            set( "v0y", v0, 1 );
            set( "v0z", v0, 2 );
            set( "v1x", v1, 0 );
            set( "v1y", v1, 1 );
            set( "v1z", v1, 2 );
            set( "v2x", v2, 0 );
            set( "v2y", v2, 1 );
            set( "v2z", v2, 2 );
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

    float
    sliderReflectionValue() const
    {
        return slider_reflection_.getFactor();
    }

    float
    sliderRefractionValue() const
    {
        return slider_refraction_.getFactor();
    }

    float
    sliderEtaValue() const
    {
        float t = slider_eta_.getFactor();
        return eta_min_ + t * ( eta_max_ - eta_min_ );
    }

    void
    setSliderDefaults()
    {
        slider_reflection_.setFactor( 0.5f );
        slider_refraction_.setFactor( 0.0f );
        slider_eta_.setFactor( 0.0f );
        updateSliderLabels();
    }

    void
    updateSliderLabels()
    {
        auto fmtv = []( float v ) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision( 2 ) << v;
            return ss.str();
        };
        reflection_value_->SetText( fmtv( sliderReflectionValue() ) );
        refraction_value_->SetText( fmtv( sliderRefractionValue() ) );
        eta_value_->SetText( fmtv( sliderEtaValue() ) );
    }

    void
    layoutSliders()
    {
        const float label_w   = 90.0f;
        const float value_w   = 48.0f;
        const float row_h     = 26.0f;
        const float start_x   = 12.0f;
        const float slider_w  = size_.x - start_x - label_w - value_w - 20.0f;
        const float slider_h  = 20.0f;
        const float slider_up = 4.0f;
        float       cur_y     = slider_origin_y_;

        auto place = [&]( dr4::Text* label, dr4::Text* value, hui::Slider& slider ) {
            if ( label )
                label->SetPos( { start_x, cur_y } );
            if ( value )
                value->SetPos( { start_x + label_w + slider_w + 8.0f, cur_y } );
            slider.setSize( { slider_w, slider_h } );
            slider.setRelPos( { start_x + label_w, cur_y - slider_up } );
            cur_y += row_h;
        };

        place( reflection_label_.get(), reflection_value_.get(), slider_reflection_ );
        place( refraction_label_.get(), refraction_value_.get(), slider_refraction_ );
        place( eta_label_.get(), eta_value_.get(), slider_eta_ );
        updateSliderLabels();
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

        auto vx = parse( fx, []( double ) { return true; } );
        auto vy = parse( fy, []( double ) { return true; } );
        auto vz = parse( fz, []( double ) { return true; } );
        if ( !vx || !vy || !vz )
        {
            return std::nullopt;
        }

        auto  color = color_picker_.getColor();
        float vf    = sliderReflectionValue();
        float vref  = sliderRefractionValue();
        float veta  = sliderEtaValue();
        if ( vref == 0.0f )
        {
            veta = std::max( veta, 0.0f );
        } else if ( veta <= 0.0f )
        {
            return std::nullopt;
        }

        CommonFields res{
            f_name ? std::string( f_name->input->getString().value_or( "" ) ) : std::string(),
            model::Vector3f( static_cast<float>( *vx ),
                             static_cast<float>( *vy ),
                             static_cast<float>( *vz ) ),
            model::Material( model::Color( color.r, color.g, color.b, color.a ), vf, vref, veta ) };
        return res;
    }

    std::unique_ptr<model::Primitive>
    buildObject( Type type, const CommonFields& common )
    {
        auto read = [&]( const char* key, auto validator ) {
            return parse( findField( key ), validator );
        };

        switch ( type )
        {
            case Type::Sphere:
                {
                    auto r = read( "p1", []( double v ) { return v > 0; } );
                    if ( !r )
                        return nullptr;
                    return std::make_unique<model::Sphere>( common.material,
                                                            common.origin,
                                                            static_cast<float>( *r ) );
                }
            case Type::AABB:
                {
                    auto p1 = read( "p1", []( double v ) { return v > 0; } );
                    auto p2 = read( "p2", []( double v ) { return v > 0; } );
                    auto p3 = read( "p3", []( double v ) { return v > 0; } );
                    if ( !p1 || !p2 || !p3 )
                        return nullptr;
                    return std::make_unique<model::AABB>(
                        common.material,
                        common.origin,
                        model::Vector3f( static_cast<float>( *p1 ),
                                         static_cast<float>( *p2 ),
                                         static_cast<float>( *p3 ) ) );
                }
            case Type::Torus:
                {
                    auto maj = read( "p1", []( double v ) { return v > 0; } );
                    auto min = read( "p2", []( double v ) { return v > 0; } );
                    if ( !maj || !min )
                        return nullptr;
                    return std::make_unique<model::Torus>( common.material,
                                                           common.origin,
                                                           static_cast<float>( *maj ),
                                                           static_cast<float>( *min ) );
                }
            case Type::HexPrism:
                {
                    auto r = read( "p1", []( double v ) { return v > 0; } );
                    auto h = read( "p2", []( double v ) { return v > 0; } );
                    if ( !r || !h )
                        return nullptr;
                    return std::make_unique<model::HexPrism>( common.material,
                                                              common.origin,
                                                              static_cast<float>( *r ),
                                                              static_cast<float>( *h ) );
                }
            case Type::Goursat:
                {
                    auto ka = read( "ka", []( double v ) { return v > 0; } );
                    auto kb = read( "kb", []( double v ) { return v > 0; } );
                    if ( !ka || !kb )
                        return nullptr;
                    return std::make_unique<model::Goursat>( common.material,
                                                             common.origin,
                                                             static_cast<float>( *ka ),
                                                             static_cast<float>( *kb ) );
                }
            case Type::RoundedBox:
                {
                    auto p1 = read( "p1", []( double v ) { return v > 0; } );
                    auto p2 = read( "p2", []( double v ) { return v > 0; } );
                    auto p3 = read( "p3", []( double v ) { return v > 0; } );
                    auto p4 = read( "p4", []( double v ) { return v > 0; } );
                    if ( !p1 || !p2 || !p3 || !p4 )
                        return nullptr;
                    return std::make_unique<model::RoundedBox>(
                        common.material,
                        common.origin,
                        model::Vector3f( static_cast<float>( *p1 ),
                                         static_cast<float>( *p2 ),
                                         static_cast<float>( *p3 ) ),
                        static_cast<float>( *p4 ) );
                }
            case Type::Ellipsoid:
                {
                    auto p1 = read( "p1", []( double v ) { return v > 0; } );
                    auto p2 = read( "p2", []( double v ) { return v > 0; } );
                    auto p3 = read( "p3", []( double v ) { return v > 0; } );
                    if ( !p1 || !p2 || !p3 )
                        return nullptr;
                    return std::make_unique<model::Ellipsoid>(
                        common.material,
                        common.origin,
                        model::Vector3f( static_cast<float>( *p1 ),
                                         static_cast<float>( *p2 ),
                                         static_cast<float>( *p3 ) ) );
                }
            case Type::Capsule:
                {
                    auto h = read( "p1", []( double v ) { return v > 0; } );
                    auto r = read( "p2", []( double v ) { return v > 0; } );
                    if ( !h || !r )
                        return nullptr;
                    float fh = static_cast<float>( *h );
                    return std::make_unique<model::Capsule>(
                        common.material,
                        common.origin,
                        model::Vector3f{ 0.0f, -0.5f * fh, 0.0f },
                        model::Vector3f{ 0.0f, 0.5f * fh, 0.0f },
                        static_cast<float>( *r ) );
                }
            case Type::RoundedCone:
                {
                    auto h  = read( "p1", []( double v ) { return v > 0; } );
                    auto ra = read( "p2", []( double v ) { return v > 0; } );
                    auto rb = read( "p3", []( double v ) { return v > 0; } );
                    if ( !h || !ra || !rb )
                        return nullptr;
                    float fh = static_cast<float>( *h );
                    return std::make_unique<model::RoundedCone>(
                        common.material,
                        common.origin,
                        model::Vector3f{ 0.0f, -0.5f * fh, 0.0f },
                        model::Vector3f{ 0.0f, 0.5f * fh, 0.0f },
                        static_cast<float>( *ra ),
                        static_cast<float>( *rb ) );
                }
            case Type::CappedCone:
                {
                    auto h  = read( "p1", []( double v ) { return v > 0; } );
                    auto ra = read( "p2", []( double v ) { return v > 0; } );
                    auto rb = read( "p3", []( double v ) { return v > 0; } );
                    if ( !h || !ra || !rb )
                        return nullptr;
                    float fh = static_cast<float>( *h );
                    return std::make_unique<model::CappedCone>(
                        common.material,
                        common.origin,
                        model::Vector3f{ 0.0f, -0.5f * fh, 0.0f },
                        model::Vector3f{ 0.0f, 0.5f * fh, 0.0f },
                        static_cast<float>( *ra ),
                        static_cast<float>( *rb ) );
                }
            case Type::CappedCylinder:
                {
                    auto h = read( "p1", []( double v ) { return v > 0; } );
                    auto r = read( "p2", []( double v ) { return v > 0; } );
                    if ( !h || !r )
                        return nullptr;
                    float fh = static_cast<float>( *h );
                    return std::make_unique<model::CappedCylinder>(
                        common.material,
                        common.origin,
                        model::Vector3f{ 0.0f, -0.5f * fh, 0.0f },
                        model::Vector3f{ 0.0f, 0.5f * fh, 0.0f },
                        static_cast<float>( *r ) );
                }
            case Type::Wedge:
                {
                    auto p1 = read( "p1", []( double v ) { return v > 0; } );
                    auto p2 = read( "p2", []( double v ) { return v > 0; } );
                    auto p3 = read( "p3", []( double v ) { return v > 0; } );
                    if ( !p1 || !p2 || !p3 )
                        return nullptr;
                    return std::make_unique<model::Wedge>(
                        common.material,
                        common.origin,
                        model::Vector3f( static_cast<float>( *p1 ),
                                         static_cast<float>( *p2 ),
                                         static_cast<float>( *p3 ) ) );
                }
            case Type::Ellipse:
                {
                    auto ux = read( "ux", []( double ) { return true; } );
                    auto uy = read( "uy", []( double ) { return true; } );
                    auto uz = read( "uz", []( double ) { return true; } );
                    auto vx = read( "vx", []( double ) { return true; } );
                    auto vy = read( "vy", []( double ) { return true; } );
                    auto vz = read( "vz", []( double ) { return true; } );
                    if ( !ux || !uy || !uz || !vx || !vy || !vz )
                        return nullptr;
                    return std::make_unique<model::Ellipse>(
                        common.material,
                        common.origin,
                        model::Vector3f( static_cast<float>( *ux ),
                                         static_cast<float>( *uy ),
                                         static_cast<float>( *uz ) ),
                        model::Vector3f( static_cast<float>( *vx ),
                                         static_cast<float>( *vy ),
                                         static_cast<float>( *vz ) ) );
                }
            case Type::Triangle:
                {
                    auto fetch = [&]( const char* key ) {
                        return read( key, []( double ) { return true; } );
                    };
                    auto v0x = fetch( "v0x" ), v0y = fetch( "v0y" ), v0z = fetch( "v0z" );
                    auto v1x = fetch( "v1x" ), v1y = fetch( "v1y" ), v1z = fetch( "v1z" );
                    auto v2x = fetch( "v2x" ), v2y = fetch( "v2y" ), v2z = fetch( "v2z" );
                    if ( !v0x || !v0y || !v0z || !v1x || !v1y || !v1z || !v2x || !v2y || !v2z )
                        return nullptr;
                    return std::make_unique<model::Triangle>(
                        common.material,
                        model::Vector3f( static_cast<float>( *v0x ),
                                         static_cast<float>( *v0y ),
                                         static_cast<float>( *v0z ) ),
                        model::Vector3f( static_cast<float>( *v1x ),
                                         static_cast<float>( *v1y ),
                                         static_cast<float>( *v1z ) ),
                        model::Vector3f( static_cast<float>( *v2x ),
                                         static_cast<float>( *v2y ),
                                         static_cast<float>( *v2z ) ) );
                }
        }

        return nullptr;
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
        auto obj = buildObject( current_type_, common );
        if ( !obj )
        {
            return;
        }

        obj->setDisplayName( common.name );
        scene_manager_.getObjects()[*target_idx_] = std::move( obj );
        scene_manager_.setTargetObj( scene_manager_.getObjects()[*target_idx_].get() );
        scene_manager_.needUpdate() = true;
        if ( objects_changed_cb_ )
            objects_changed_cb_();
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
        if ( name == "HexPrism" )
            return Type::HexPrism;
        if ( name == "Goursat" )
            return Type::Goursat;
        if ( name == "RoundedBox" )
            return Type::RoundedBox;
        if ( name == "Ellipsoid" )
            return Type::Ellipsoid;
        if ( name == "Capsule" )
            return Type::Capsule;
        if ( name == "RoundedCone" )
            return Type::RoundedCone;
        if ( name == "CappedCone" )
            return Type::CappedCone;
        if ( name == "CappedCylinder" )
            return Type::CappedCylinder;
        if ( name == "Wedge" )
            return Type::Wedge;
        if ( name == "Ellipse" )
            return Type::Ellipse;
        if ( name == "Triangle" )
            return Type::Triangle;
        return Type::Sphere;
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
        color_picker_.setColor( { 118, 185, 0, 255 } );
        current_color_ = { 118, 185, 0, 255 };
        setSliderDefaults();

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
            case Type::Goursat:
                if ( auto* f = findField( "ka" ) )
                    f->input->setString( "1.00" );
                if ( auto* f = findField( "kb" ) )
                    f->input->setString( "1.00" );
                break;
            case Type::RoundedBox:
                if ( auto* f = findField( "p1" ) )
                    f->input->setString( "1.00" );
                if ( auto* f = findField( "p2" ) )
                    f->input->setString( "1.00" );
                if ( auto* f = findField( "p3" ) )
                    f->input->setString( "1.00" );
                if ( auto* f = findField( "p4" ) )
                    f->input->setString( "0.20" );
                break;
            case Type::Ellipsoid:
                if ( auto* f = findField( "p1" ) )
                    f->input->setString( "1.00" );
                if ( auto* f = findField( "p2" ) )
                    f->input->setString( "1.50" );
                if ( auto* f = findField( "p3" ) )
                    f->input->setString( "0.75" );
                break;
            case Type::Capsule:
                if ( auto* f = findField( "p1" ) )
                    f->input->setString( "2.00" );
                if ( auto* f = findField( "p2" ) )
                    f->input->setString( "0.50" );
                break;
            case Type::RoundedCone:
                if ( auto* f = findField( "p1" ) )
                    f->input->setString( "2.00" );
                if ( auto* f = findField( "p2" ) )
                    f->input->setString( "1.00" );
                if ( auto* f = findField( "p3" ) )
                    f->input->setString( "0.50" );
                break;
            case Type::CappedCone:
                if ( auto* f = findField( "p1" ) )
                    f->input->setString( "2.00" );
                if ( auto* f = findField( "p2" ) )
                    f->input->setString( "1.00" );
                if ( auto* f = findField( "p3" ) )
                    f->input->setString( "0.50" );
                break;
            case Type::CappedCylinder:
                if ( auto* f = findField( "p1" ) )
                    f->input->setString( "2.00" );
                if ( auto* f = findField( "p2" ) )
                    f->input->setString( "0.50" );
                break;
            case Type::Wedge:
                if ( auto* f = findField( "p1" ) )
                    f->input->setString( "1.00" );
                if ( auto* f = findField( "p2" ) )
                    f->input->setString( "1.00" );
                if ( auto* f = findField( "p3" ) )
                    f->input->setString( "1.00" );
                break;
            case Type::Ellipse:
                if ( auto* f = findField( "ux" ) )
                    f->input->setString( "1.00" );
                if ( auto* f = findField( "uy" ) )
                    f->input->setString( "0.00" );
                if ( auto* f = findField( "uz" ) )
                    f->input->setString( "0.00" );
                if ( auto* f = findField( "vx" ) )
                    f->input->setString( "0.00" );
                if ( auto* f = findField( "vy" ) )
                    f->input->setString( "1.00" );
                if ( auto* f = findField( "vz" ) )
                    f->input->setString( "0.00" );
                break;
            case Type::Triangle:
                {
                    std::vector<std::pair<const char*, const char*>> defaults = {
                        { "v0x", "0.0" },
                        { "v0y", "0.0" },
                        { "v0z", "0.0" },
                        { "v1x", "1.0" },
                        { "v1y", "0.0" },
                        { "v1z", "0.0" },
                        { "v2x", "0.0" },
                        { "v2y", "1.0" },
                        { "v2z", "0.0" } };
                    for ( auto& p : defaults )
                    {
                        if ( auto* f = findField( p.first ) )
                            f->input->setString( p.second );
                    }
                    break;
                }
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
                return "HexPrism";
            case Type::Goursat:
                return "Goursat";
            case Type::RoundedBox:
                return "RoundedBox";
            case Type::Ellipsoid:
                return "Ellipsoid";
            case Type::Capsule:
                return "Capsule";
            case Type::RoundedCone:
                return "RoundedCone";
            case Type::CappedCone:
                return "CappedCone";
            case Type::CappedCylinder:
                return "CappedCylinder";
            case Type::Wedge:
                return "Wedge";
            case Type::Ellipse:
                return "Ellipse";
            case Type::Triangle:
            default:
                return "Triangle";
        }
    }

    void
    addNewObject( const CommonFields& common )
    {
        auto obj = buildObject( current_type_, common );
        if ( !obj )
        {
            return;
        }

        obj->setDisplayName( common.name );
        scene_manager_.getObjects().push_back( std::move( obj ) );
        target_idx_ = scene_manager_.getObjects().size() - 1;
        scene_manager_.setTargetObj( scene_manager_.getObjects().back().get() );
        scene_manager_.needUpdate() = true;
        if ( objects_changed_cb_ )
            objects_changed_cb_();
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
        float dx     = 0.2f;
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
        if ( objects_changed_cb_ )
            objects_changed_cb_();
    }

    class TypePickerDialog : public hui::DialogBox {
      public:
        TypePickerDialog( hui::WindowManager*         wm,
                          float                       x,
                          float                       y,
                          Type                        initial,
                          std::function<void( Type )> on_ok,
                          std::function<void()>       on_cancel )
            : DialogBox( wm, x, y, 220.0f, 300.0f, on_cancel, "Choose Type" ),
              list_( wm, { 10.0f, TopBarHeight + 10.0f }, { 200.0f, 180.0f }, 10.0f ),
              ok_( wm, { 20.0f, 230.0f }, { 80.0f, 26.0f } ),
              cancel_( wm, { 120.0f, 230.0f }, { 80.0f, 26.0f } ),
              on_ok_( std::move( on_ok ) ),
              on_cancel_( std::move( on_cancel ) ),
              selected_( initial )
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
            if ( event.apply( &ok_ ) )
                return true;
            if ( event.apply( &cancel_ ) )
                return true;
            if ( event.apply( &list_ ) )
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
            list_.clearButtons();
            auto add = [&]( const std::string& label, Type t ) {
                auto btn = std::make_unique<hui::Button>(
                    wm_,
                    dr4::Vec2f{ 0, 0 },
                    dr4::Vec2f{ 180.0f, 32.0f },
                    label,
                    t == selected_
                        ? hui::Button::Theme{ hui::Button::Theme::Default().pressed_color,
                                              hui::Button::Theme::Default().hovered_color,
                                              hui::Button::Theme::Default().pressed_color,
                                              hui::Button::Theme::Default().font_color,
                                              hui::Button::Theme::Default().font_size }
                        : hui::Button::Theme::Default() );
                btn->setOnClick( [this, t]() {
                    selected_ = t;
                    buildItems();
                } );
                list_.addButton( std::move( btn ) );
            };

            add( "Sphere", Type::Sphere );
            add( "AABB", Type::AABB );
            add( "Torus", Type::Torus );
            add( "HexPrism", Type::HexPrism );
            add( "Goursat", Type::Goursat );
            add( "RoundedBox", Type::RoundedBox );
            add( "Ellipsoid", Type::Ellipsoid );
            add( "Capsule", Type::Capsule );
            add( "RoundedCone", Type::RoundedCone );
            add( "CappedCone", Type::CappedCone );
            add( "CappedCylinder", Type::CappedCylinder );
            add( "Wedge", Type::Wedge );
            add( "Ellipse", Type::Ellipse );
            add( "Triangle", Type::Triangle );

            list_.rebuildLayout();
        }

      private:
        hui::ScrollableButtonsListWidget list_;
        hui::ButtonOk                    ok_;
        hui::ButtonCancel                cancel_;
        std::function<void( Type )>      on_ok_;
        std::function<void()>            on_cancel_;
        Type                             selected_ = Type::Sphere;
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
                current_type_,
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
    model::SceneManager&       scene_manager_;
    std::optional<size_t>      target_idx_;
    std::vector<FieldRef>      form_fields_;
    RGBPicker                  color_picker_;
    std::unique_ptr<dr4::Text> color_label_;
    dr4::Color                 current_color_{ 118, 185, 0, 255 };
    std::unique_ptr<dr4::Text> reflection_label_;
    std::unique_ptr<dr4::Text> refraction_label_;
    std::unique_ptr<dr4::Text> eta_label_;
    std::unique_ptr<dr4::Text> reflection_value_;
    std::unique_ptr<dr4::Text> refraction_value_;
    std::unique_ptr<dr4::Text> eta_value_;
    hui::Slider                slider_reflection_;
    hui::Slider                slider_refraction_;
    hui::Slider                slider_eta_;
    float                      slider_origin_y_ = 0.0f;
    const float                eta_min_         = 1.0f;
    const float                eta_max_         = 3.0f;
    Type                       current_type_    = Type::Sphere;
    Type                       creation_type_   = Type::Sphere;
    const float                type_row_height_ = 28.0f;
    bool                       copy_visible_    = false;
    bool                       del_visible_     = false;
    bool                       editing_mode_    = false;
    std::function<void()>      objects_changed_cb_;
    hui::Button                type_btn_;
    hui::Button                add_btn_;
    hui::Button                copy_btn_;
    hui::Button                del_btn_;
    hui::Button                apply_btn_;
    std::unique_ptr<dr4::Text> label_text_;
};

} // namespace view
} // namespace rta
