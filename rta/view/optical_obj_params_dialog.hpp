#pragma once

#include "hui/button.hpp"
#include "hui/button_cancel.hpp"
#include "hui/button_ok.hpp"
#include "hui/dialog_box.hpp"
#include "hui/input_text.hpp"
#include "rta/model/primitives/material.hpp"
#include "rta/model/rendering/scene_manager.hpp"
#include <functional>
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace rta {
namespace view {

// Generic dialog for optical object params; derived classes plug in type-specific fields/actions.
class OpticalObjParamsDialog : public hui::DialogBox {
  public:
    enum class Mode { Create, Edit };

    struct FieldDef
    {
        std::string label;
        std::string key;
    };

    struct CommonFields
    {
        std::string        name;
        model::Vector3f    pos;
        model::Material    material;
        std::optional<int> idx; // optional index for edit mode
    };

    using CloseCb = std::function<void()>;

  protected:
    OpticalObjParamsDialog( hui::WindowManager*          wm,
                            float                        x,
                            float                        y,
                            float                        w,
                            float                        h,
                            const std::string&           title,
                            rta::model::SceneManager&    scene_manager,
                            Mode                         mode,
                            std::optional<size_t>        obj_idx,
                            const std::vector<FieldDef>& specific_fields,
                            CloseCb                      close_cb )
        : DialogBox( wm, x, y, w, h, close_cb, title ),
          scene_manager_( scene_manager ),
          mode_( mode ),
          obj_idx_( obj_idx ),
          ok_btn_( wm, { 10.0f, TopBarHeight + 8.0f }, { 80.0f, 24.0f } ),
          cancel_btn_( wm, { 100.0f, TopBarHeight + 8.0f }, { 80.0f, 24.0f } )
    {
        initLayout( w, specific_fields );
        wireButtons( close_cb );
    }

    virtual ~OpticalObjParamsDialog() = default;

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        if ( event.apply( &ok_btn_ ) )
        {
            return true;
        }
        if ( event.apply( &cancel_btn_ ) )
        {
            return true;
        }
        for ( auto& f : fields_ )
        {
            if ( event.apply( f.input.get() ) )
                return true;
        }
        return hui::DialogBox::propagateEventToChildren( event );
    }

    void
    RedrawMyTexture() const override
    {
        DialogBox::RedrawMyTexture();
        ok_btn_.Redraw();
        cancel_btn_.Redraw();
        for ( auto& f : fields_ )
        {
            f.label->DrawOn( *texture_ );
            f.input->Redraw();
        }
    }

  protected:
    struct FieldRef
    {
        std::string                     key;
        std::unique_ptr<dr4::Text>      label;
        std::unique_ptr<hui::InputText> input;
    };

    FieldRef*
    findField( const std::string& key )
    {
        for ( auto& f : fields_ )
        {
            if ( f.key == key )
                return &f;
        }
        return nullptr;
    }

    static std::string
    fmt2( double v )
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision( 2 ) << v;
        return ss.str();
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
    prefillCommon( const model::SceneManager::ObjectInfo& info )
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
    }

    void
    prefillCommonDefaults()
    {
        if ( auto* f = findField( "name" ) )
            f->input->setString( "Object" );
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
    }

    std::optional<CommonFields>
    parseCommon()
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
                                           static_cast<float>( *vf ) ),
                          obj_idx_.has_value() ? std::optional<int>( static_cast<int>( *obj_idx_ ) )
                                               : std::nullopt };
        return res;
    }

    virtual void
    prefillSpecific() = 0;
    virtual void
    prefillSpecificDefaults() = 0;
    virtual bool
    applySpecific( const CommonFields& common ) = 0;

  private:
    void
    initLayout( float w, const std::vector<FieldDef>& specific_fields )
    {
        const float start_x   = 10.0f;
        const float start_y   = TopBarHeight + 40.0f;
        const float label_w   = 100.0f;
        const float input_w   = w - label_w - 30.0f;
        const float field_h   = 20.0f;
        const float line_step = field_h + 6.0f;

        std::vector<FieldDef> defs;
        defs.push_back( { "Name", "name" } );
        defs.push_back( { "X", "x" } );
        defs.push_back( { "Y", "y" } );
        defs.push_back( { "Z", "z" } );
        defs.insert( defs.end(), specific_fields.begin(), specific_fields.end() );
        defs.push_back( { "R", "r" } );
        defs.push_back( { "G", "g" } );
        defs.push_back( { "B", "b" } );
        defs.push_back( { "Reflection", "f" } );

        auto* win  = wm_->getWindow();
        auto* font = win->GetDefaultFont();

        float cur_y = start_y;
        for ( const auto& d : defs )
        {
            auto label = std::unique_ptr<dr4::Text>( win->CreateText() );
            label->SetFont( font );
            label->SetFontSize( 12 );
            label->SetText( d.label );
            label->SetColor( { 220, 220, 220, 255 } );
            label->SetPos( { start_x, cur_y } );

            auto input =
                std::make_unique<hui::InputText>( wm_, start_x + label_w, cur_y, input_w, field_h );
            input->setParent( this );

            fields_.push_back( { d.key, std::move( label ), std::move( input ) } );
            cur_y += line_step;
        }
    }

    void
    wireButtons( CloseCb close_cb )
    {
        ok_btn_.setParent( this );
        cancel_btn_.setParent( this );
        ok_btn_.setOnClick( [this]() {
            auto common = parseCommon();
            if ( !common.has_value() )
                return;
            if ( applySpecific( *common ) )
            {
                // derived should close the dialog if needed
            }
        } );
        cancel_btn_.setOnClick( [close_cb]() {
            if ( close_cb )
                close_cb();
        } );
    }

  protected:
    rta::model::SceneManager& scene_manager_;
    Mode                      mode_;
    std::optional<size_t>     obj_idx_;

  private:
    std::vector<FieldRef> fields_;

  protected:
    hui::ButtonOk     ok_btn_;
    hui::ButtonCancel cancel_btn_;
};

} // namespace view
} // namespace rta
