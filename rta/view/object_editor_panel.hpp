#pragma once

#include "dr4/texture.hpp"
#include "hui/button.hpp"
#include "hui/closable_panel.hpp"
#include "hui/dialog_box.hpp"
#include "hui/input_text.hpp"
#include "hui/label.hpp"
#include "hui/scrollable_list_widget.hpp"
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
#include "rta/view/rgb_picker.hpp"
#include <algorithm>
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace rta {
namespace view {

class ObjectEditorPanel : public hui::ClosablePanel {
  public:
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

    class GenericFields; // fwd

  private:
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
        return std::min( 1.0f, std::max( 0.0f, v ) );
    }

    struct TypeDescriptor
    {
        Type                                             type;
        const char*                                      name;
        std::vector<std::pair<std::string, std::string>> fields; // label,key (without Name)
        std::function<void()>                            set_defaults;
        std::function<void( size_t )>                    prefill_existing;
        std::function<std::unique_ptr<model::Primitive>( const model::Material&,
                                                         const std::string& )>
            create_new;
        std::function<bool( const model::Material&, const std::string&, model::Primitive& )>
            apply_to_existing;
    };

    class ObjEditFields {
      public:
        struct CommonValues
        {
            std::string     name;
            model::Material material;
        };

        ObjEditFields( hui::WindowManager* wm,
                       hui::DialogBox*     parent,
                       const dr4::Vec2f&   size,
                       const char*         type_name )
            : wm_( wm ),
              parent_( parent ),
              size_( size ),
              color_picker_( wm,
                             { size.x - 170.0f, TopBarHeight + 10.0f },
                             dr4::Color{ 118, 185, 0, 255 },
                             [this]( const dr4::Color& c ) { current_color_ = c; } ),
              slider_reflection_( wm, { 0, 0 }, { size.x - 150.0f, 14.0f } ),
              slider_refraction_( wm, { 0, 0 }, { size.x - 150.0f, 14.0f } ),
              slider_eta_( wm, { 0, 0 }, { size.x - 150.0f, 14.0f } ),
              type_name_( type_name )
        {
            color_label_.reset( wm_->getWindow()->CreateText() );
            color_label_->SetFont( wm_->getWindow()->GetDefaultFont() );
            color_label_->SetFontSize( 13 );
            color_label_->SetColor( { 220, 220, 220, 255 } );
            color_label_->SetText( "Color" );
            color_label_->SetPos( { size_.x - 180.0f, TopBarHeight - 2.0f } );
            color_picker_.setParent( parent_ );

            auto mkText = [this]() {
                auto* t = wm_->getWindow()->CreateText();
                t->SetFont( wm_->getWindow()->GetDefaultFont() );
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

            slider_reflection_.setParent( parent_ );
            slider_refraction_.setParent( parent_ );
            slider_eta_.setParent( parent_ );
            slider_reflection_.setOnChange( [this]( float ) { updateSliderLabels(); } );
            slider_refraction_.setOnChange( [this]( float ) { updateSliderLabels(); } );
            slider_eta_.setOnChange( [this]( float ) { updateSliderLabels(); } );

            addField( "Name", "name" );
        }

        virtual ~ObjEditFields() = default;

        virtual void
        prefillDefaults()
        {
            if ( auto* f = findField( "name" ) )
                f->input->setString( type_name_ );
            color_picker_.setColor( { 118, 185, 0, 255 } );
            current_color_ = { 118, 185, 0, 255 };
            setSliderDefaults();
        }

        virtual void
        prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj )
        {
            if ( auto* f = findField( "name" ) )
                f->input->setString( info.display_name.empty() ? info.type_name
                                                               : info.display_name );
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
            (void)obj;
        }

        virtual std::unique_ptr<model::Primitive>
        buildNew( const CommonValues& ) = 0;

        virtual bool
        applyToExisting( const CommonValues&, model::Primitive& ) = 0;

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

        bool
        propagateEvent( const hui::Event& event )
        {
            for ( auto& f : form_fields_ )
            {
                if ( event.apply( f.input.get() ) )
                    return true;
            }
            if ( event.apply( &color_picker_ ) )
                return true;
            if ( event.apply( &slider_reflection_ ) )
                return true;
            if ( event.apply( &slider_refraction_ ) )
                return true;
            if ( event.apply( &slider_eta_ ) )
                return true;
            return false;
        }

        void
        redraw( dr4::Texture& target )
        {
            for ( auto& f : form_fields_ )
            {
                f.label->SetPos( f.label_pos );
                f.label->DrawOn( target );
                f.input->Redraw();
            }
            if ( color_label_ )
                color_label_->DrawOn( target );
            color_picker_.Redraw();

            if ( reflection_label_ )
            {
                reflection_label_->DrawOn( target );
                refraction_label_->DrawOn( target );
                eta_label_->DrawOn( target );
                reflection_value_->DrawOn( target );
                refraction_value_->DrawOn( target );
                eta_value_->DrawOn( target );
            }

            slider_reflection_.Redraw();
            slider_refraction_.Redraw();
            slider_eta_.Redraw();
        }

        CommonValues
        parseCommon()
        {
            auto* f_name = findField( "name" );
            auto  color  = color_picker_.getColor();
            float vf     = sliderReflectionValue();
            float vref   = sliderRefractionValue();
            float veta   = sliderEtaValue();
            if ( vref != 0.0f && veta <= 0.0f )
            {
                setEtaErrorHighlight( true );
                throw std::runtime_error( "Eta must be > 0 when refraction enabled" );
            }
            setEtaErrorHighlight( false );

            return CommonValues{
                f_name ? std::string( f_name->input->getString().value_or( "" ) ) : std::string(),
                model::Material( model::Color( color.r, color.g, color.b, color.a ),
                                 vf,
                                 vref,
                                 veta ) };
        }

        float
        layoutFields( float start_y )
        {
            const float margin_x   = 12.0f;
            const float line_h     = 24.0f;
            const float label_w    = 120.0f;
            const float field_h    = 18.0f;
            const float field_w    = size_.x - margin_x - label_w - 20.0f;
            const float slider_gap = 12.0f;

            float cur_y = start_y;
            for ( auto& d : form_fields_ )
            {
                if ( d.key == "name" && d.label )
                    d.label->SetText( "Name" );
                d.label_pos = { margin_x, cur_y };

                d.input = std::make_unique<hui::InputText>( wm_,
                                                            margin_x + label_w,
                                                            cur_y,
                                                            field_w,
                                                            field_h );
                d.input->setParent( parent_ );
                cur_y += line_h;
            }

            slider_origin_y_ = cur_y + slider_gap;
            layoutSliders();
            return slider_origin_y_ + 70.0f;
        }

        void
        setField( const std::string& key, const std::string& val )
        {
            if ( auto* f = findField( key ) )
                f->input->setString( val );
        }

        template<typename Validator>
        std::optional<double>
        parseField( const std::string& key, Validator v )
        {
            return parse( findField( key ), v );
        }

      protected:
        FieldRef&
        addField( const std::string& label, const std::string& key )
        {
            form_fields_.push_back( { key, nullptr, nullptr, { 0, 0 } } );
            form_fields_.back().label = nullptr;
            auto* win                 = wm_->getWindow();
            auto* font                = win->GetDefaultFont();
            auto  t                   = std::unique_ptr<dr4::Text>( win->CreateText() );
            t->SetFont( font );
            t->SetFontSize( 15 );
            t->SetText( label );
            t->SetColor( { 220, 220, 220, 255 } );
            form_fields_.back().label = std::move( t );
            return form_fields_.back();
        }

        void
        setEtaErrorHighlight( bool error )
        {
            dr4::Color err{ 255, 64, 64, 255 };
            dr4::Color ok{ 220, 220, 220, 255 };
            if ( eta_label_ )
                eta_label_->SetColor( error ? err : ok );
            if ( eta_value_ )
                eta_value_->SetColor( error ? err : ok );
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

      protected:
        hui::WindowManager*        wm_;
        hui::DialogBox*            parent_;
        dr4::Vec2f                 size_;
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
        const char*                type_name_;
    };

    // Generic fields that delegate specifics to descriptor lambdas.
    class GenericFields : public ObjEditFields {
      public:
        GenericFields( hui::WindowManager*   wm,
                       hui::DialogBox*       parent,
                       const dr4::Vec2f&     size,
                       const TypeDescriptor& d,
                       model::SceneManager&  sm )
            : ObjEditFields( wm, parent, size, d.name ), desc_( d ), scene_manager_( sm )
        {
            // push fields from descriptor (without Name which base already added)
            for ( auto& p : desc_.fields )
            {
                addField( p.first, p.second );
            }
            layoutFields( TopBarHeight + 12.0f );
        }

        void
        prefillDefaults() override
        {
            ObjEditFields::prefillDefaults();
            if ( desc_.set_defaults )
                desc_.set_defaults();
        }

        void
        prefillExisting( const model::SceneManager::ObjectInfo& info,
                         model::Primitive&                      obj ) override
        {
            ObjEditFields::prefillExisting( info, obj );
            if ( desc_.prefill_existing )
                desc_.prefill_existing( info.objects_idx );
        }

        std::unique_ptr<model::Primitive>
        buildNew( const CommonValues& common ) override
        {
            if ( !desc_.create_new )
                return nullptr;
            auto res = desc_.create_new( common.material, common.name );
            if ( res )
                res->setDisplayName( common.name );
            return res;
        }

        bool
        applyToExisting( const CommonValues& common, model::Primitive& obj ) override
        {
            if ( !desc_.apply_to_existing )
                return false;
            bool ok = desc_.apply_to_existing( common.material, common.name, obj );
            if ( ok )
                obj.setDisplayName( common.name );
            return ok;
        }

        std::optional<model::Vector3f>
        parsePos()
        {
            auto x = parseField( "x", []( double ) { return true; } );
            auto y = parseField( "y", []( double ) { return true; } );
            auto z = parseField( "z", []( double ) { return true; } );
            if ( !x || !y || !z )
                return std::nullopt;
            return model::Vector3f( static_cast<float>( *x ),
                                    static_cast<float>( *y ),
                                    static_cast<float>( *z ) );
        }

        // exposed for descriptor lambdas
        void
        setFieldValue( const std::string& key, const std::string& v )
        {
            setField( key, v );
        }

        model::SceneManager&
        scene_manager()
        {
            return scene_manager_;
        }

      private:
        const TypeDescriptor& desc_;
        model::SceneManager&  scene_manager_;
    };

    // One alias per type to align with requested names.
    class SphereEditFields : public GenericFields {
      public:
        SphereEditFields( hui::WindowManager*   wm,
                          hui::DialogBox*       parent,
                          const dr4::Vec2f&     size,
                          const TypeDescriptor& d,
                          model::SceneManager&  sm )
            : GenericFields( wm, parent, size, d, sm )
        {
        }
    };
    class AABBEditFields : public GenericFields {
      public:
        AABBEditFields( hui::WindowManager*   wm,
                        hui::DialogBox*       parent,
                        const dr4::Vec2f&     size,
                        const TypeDescriptor& d,
                        model::SceneManager&  sm )
            : GenericFields( wm, parent, size, d, sm )
        {
        }
    };
    class TorusEditFields : public GenericFields {
      public:
        TorusEditFields( hui::WindowManager*   wm,
                         hui::DialogBox*       parent,
                         const dr4::Vec2f&     size,
                         const TypeDescriptor& d,
                         model::SceneManager&  sm )
            : GenericFields( wm, parent, size, d, sm )
        {
        }
    };
    class HexPrismEditFields : public GenericFields {
      public:
        HexPrismEditFields( hui::WindowManager*   wm,
                            hui::DialogBox*       parent,
                            const dr4::Vec2f&     size,
                            const TypeDescriptor& d,
                            model::SceneManager&  sm )
            : GenericFields( wm, parent, size, d, sm )
        {
        }
    };
    class GoursatEditFields : public GenericFields {
      public:
        GoursatEditFields( hui::WindowManager*   wm,
                           hui::DialogBox*       parent,
                           const dr4::Vec2f&     size,
                           const TypeDescriptor& d,
                           model::SceneManager&  sm )
            : GenericFields( wm, parent, size, d, sm )
        {
        }
    };
    class RoundedBoxEditFields : public GenericFields {
      public:
        RoundedBoxEditFields( hui::WindowManager*   wm,
                              hui::DialogBox*       parent,
                              const dr4::Vec2f&     size,
                              const TypeDescriptor& d,
                              model::SceneManager&  sm )
            : GenericFields( wm, parent, size, d, sm )
        {
        }
    };
    class EllipsoidEditFields : public GenericFields {
      public:
        EllipsoidEditFields( hui::WindowManager*   wm,
                             hui::DialogBox*       parent,
                             const dr4::Vec2f&     size,
                             const TypeDescriptor& d,
                             model::SceneManager&  sm )
            : GenericFields( wm, parent, size, d, sm )
        {
        }
    };
    class CapsuleEditFields : public GenericFields {
      public:
        CapsuleEditFields( hui::WindowManager*   wm,
                           hui::DialogBox*       parent,
                           const dr4::Vec2f&     size,
                           const TypeDescriptor& d,
                           model::SceneManager&  sm )
            : GenericFields( wm, parent, size, d, sm )
        {
        }
    };
    class RoundedConeEditFields : public GenericFields {
      public:
        RoundedConeEditFields( hui::WindowManager*   wm,
                               hui::DialogBox*       parent,
                               const dr4::Vec2f&     size,
                               const TypeDescriptor& d,
                               model::SceneManager&  sm )
            : GenericFields( wm, parent, size, d, sm )
        {
        }
    };
    class CappedConeEditFields : public GenericFields {
      public:
        CappedConeEditFields( hui::WindowManager*   wm,
                              hui::DialogBox*       parent,
                              const dr4::Vec2f&     size,
                              const TypeDescriptor& d,
                              model::SceneManager&  sm )
            : GenericFields( wm, parent, size, d, sm )
        {
        }
    };
    class CappedCylinderEditFields : public GenericFields {
      public:
        CappedCylinderEditFields( hui::WindowManager*   wm,
                                  hui::DialogBox*       parent,
                                  const dr4::Vec2f&     size,
                                  const TypeDescriptor& d,
                                  model::SceneManager&  sm )
            : GenericFields( wm, parent, size, d, sm )
        {
        }
    };
    class WedgeEditFields : public GenericFields {
      public:
        WedgeEditFields( hui::WindowManager*   wm,
                         hui::DialogBox*       parent,
                         const dr4::Vec2f&     size,
                         const TypeDescriptor& d,
                         model::SceneManager&  sm )
            : GenericFields( wm, parent, size, d, sm )
        {
        }
    };
    class EllipseEditFields : public GenericFields {
      public:
        EllipseEditFields( hui::WindowManager*   wm,
                           hui::DialogBox*       parent,
                           const dr4::Vec2f&     size,
                           const TypeDescriptor& d,
                           model::SceneManager&  sm )
            : GenericFields( wm, parent, size, d, sm )
        {
        }
    };
    class TriangleEditFields : public GenericFields {
      public:
        TriangleEditFields( hui::WindowManager*   wm,
                            hui::DialogBox*       parent,
                            const dr4::Vec2f&     size,
                            const TypeDescriptor& d,
                            model::SceneManager&  sm )
            : GenericFields( wm, parent, size, d, sm )
        {
        }
    };

    class ObjEditorContent {
      public:
        virtual ~ObjEditorContent() = default;
        virtual bool
        propagateEvent( const hui::Event& ) = 0;
        virtual void
        redraw( dr4::Texture& ) = 0;
        virtual bool
        onApply() = 0;
        virtual void
        prefill() = 0;
        virtual Type
        type() const = 0;
    };

    class ObjEditorContentAdd : public ObjEditorContent {
      public:
        ObjEditorContentAdd( ObjectEditorPanel& panel, Type type, model::SceneManager& sm )
            : panel_( panel ), type_( type ), scene_manager_( sm )
        {
            fields_ = panel_.makeFields( type_ );
        }

        bool
        propagateEvent( const hui::Event& event ) override
        {
            return fields_ ? fields_->propagateEvent( event ) : false;
        }

        void
        redraw( dr4::Texture& target ) override
        {
            if ( fields_ )
                fields_->redraw( target );
        }

        bool
        onApply() override
        {
            if ( !fields_ )
                return false;
            try
            {
                auto common  = fields_->parseCommon();
                auto created = fields_->buildNew( common );
                if ( !created )
                    return false;
                created->setDisplayName( common.name );
                scene_manager_.getObjects().push_back( std::move( created ) );
                auto idx = scene_manager_.getObjects().size() - 1;
                scene_manager_.setTargetObj( scene_manager_.getObjects().back().get() );
                scene_manager_.needUpdate() = true;
                panel_.notifyObjectsChanged();
                panel_.setTarget( idx );
                return true;
            } catch ( ... )
            {
                return false;
            }
        }

        void
        prefill() override
        {
            if ( fields_ )
                fields_->prefillDefaults();
        }

        Type
        type() const override
        {
            return type_;
        }

        ObjEditFields*
        fields() const
        {
            return fields_.get();
        }

        void
        changeType( Type t )
        {
            type_   = t;
            fields_ = panel_.makeFields( type_ );
            fields_->prefillDefaults();
        }

      private:
        ObjectEditorPanel&             panel_;
        Type                           type_;
        model::SceneManager&           scene_manager_;
        std::unique_ptr<ObjEditFields> fields_;
    };

    class ObjEditorContentEdit : public ObjEditorContent {
      public:
        ObjEditorContentEdit( ObjectEditorPanel&   panel,
                              Type                 type,
                              size_t               idx,
                              model::SceneManager& sm )
            : panel_( panel ), type_( type ), idx_( idx ), scene_manager_( sm )
        {
            fields_ = panel_.makeFields( type_ );
        }

        bool
        propagateEvent( const hui::Event& event ) override
        {
            return fields_ ? fields_->propagateEvent( event ) : false;
        }

        void
        redraw( dr4::Texture& target ) override
        {
            if ( fields_ )
                fields_->redraw( target );
        }

        bool
        onApply() override
        {
            if ( !fields_ )
                return false;
            if ( idx_ >= scene_manager_.getObjects().size() )
                return false;
            auto& obj = *scene_manager_.getObjects()[idx_];
            try
            {
                auto common = fields_->parseCommon();
                if ( !fields_->applyToExisting( common, obj ) )
                    return false;
                scene_manager_.setTargetObj( &obj );
                scene_manager_.needUpdate() = true;
                panel_.notifyObjectsChanged();
                return true;
            } catch ( ... )
            {
                return false;
            }
        }

        void
        prefill() override
        {
            if ( !fields_ )
                return;
            auto info = scene_manager_.getObjectInfo( idx_ );
            fields_->prefillExisting( info, *scene_manager_.getObjects()[idx_] );
        }

        Type
        type() const override
        {
            return type_;
        }

        ObjEditFields*
        fields() const
        {
            return fields_.get();
        }

      private:
        ObjectEditorPanel&             panel_;
        Type                           type_;
        size_t                         idx_;
        model::SceneManager&           scene_manager_;
        std::unique_ptr<ObjEditFields> fields_;
    };

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
          type_btn_( wm,
                     { 12.0f, size.y - 34.0f },
                     { 88.0f, 24.0f },
                     "Type",
                     hui::Button::DefaultTheme ),
          add_btn_( wm,
                    { 112.0f, size.y - 34.0f },
                    { 88.0f, 24.0f },
                    "Add",
                    hui::Button::DefaultTheme ),
          copy_btn_( wm,
                     { 12.0f, size.y - 34.0f },
                     { 88.0f, 24.0f },
                     "Copy",
                     hui::Button::DefaultTheme ),
          del_btn_( wm,
                    { 112.0f, size.y - 34.0f },
                    { 88.0f, 24.0f },
                    "Delete",
                    hui::Button::DefaultTheme ),
          apply_btn_( wm,
                      { size.x - 100.0f, size.y - 34.0f },
                      { 88.0f, 24.0f },
                      "Apply",
                      hui::Button::DefaultTheme ),
          current_type_( Type::Sphere ),
          creation_type_( Type::Sphere ),
          size_( size )
    {
        setDraggable( true );
        wireButtons();
        switchToAdd();
    }

    void
    setTarget( std::optional<size_t> idx )
    {
        target_idx_ = idx;
        if ( idx.has_value() )
        {
            auto info     = scene_manager_.getObjectInfo( idx.value() );
            current_type_ = typeFromName( info.type_name );
            switchToEdit( idx.value() );
        } else
        {
            current_type_ = creation_type_;
            switchToAdd();
        }
    }

    void
    notifyObjectsChanged()
    {
        if ( objects_changed_cb_ )
            objects_changed_cb_();
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        if ( !visible_ )
            return false;

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

        if ( content_ && content_->propagateEvent( event ) )
            return true;

        if ( event.apply( &apply_btn_ ) )
            return true;

        return hui::DialogBox::propagateEventToChildren( event );
    }

    void
    RedrawMyTexture() const override
    {
        if ( !visible_ )
            return;

        hui::DialogBox::RedrawMyTexture();

        if ( content_ )
            const_cast<ObjEditorContent*>( content_.get() )->redraw( *texture_ );

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
    }

  private:
    std::string
    typeName( Type t ) const
    {
        for ( const auto& d : descriptors() )
        {
            if ( d.type == t )
                return d.name;
        }
        return "Unknown";
    }

    Type
    typeFromName( const std::string& name ) const
    {
        for ( const auto& d : descriptors() )
        {
            if ( name == d.name )
                return d.type;
        }
        return Type::Sphere;
    }

    std::unique_ptr<ObjEditFields>
    makeFields( Type t )
    {
        const auto& d = descriptor( t );
        switch ( t )
        {
            case Type::Sphere:
                return std::make_unique<SphereEditFields>( wm_, this, size_, d, scene_manager_ );
            case Type::AABB:
                return std::make_unique<AABBEditFields>( wm_, this, size_, d, scene_manager_ );
            case Type::Torus:
                return std::make_unique<TorusEditFields>( wm_, this, size_, d, scene_manager_ );
            case Type::HexPrism:
                return std::make_unique<HexPrismEditFields>( wm_, this, size_, d, scene_manager_ );
            case Type::Goursat:
                return std::make_unique<GoursatEditFields>( wm_, this, size_, d, scene_manager_ );
            case Type::RoundedBox:
                return std::make_unique<RoundedBoxEditFields>( wm_,
                                                               this,
                                                               size_,
                                                               d,
                                                               scene_manager_ );
            case Type::Ellipsoid:
                return std::make_unique<EllipsoidEditFields>( wm_, this, size_, d, scene_manager_ );
            case Type::Capsule:
                return std::make_unique<CapsuleEditFields>( wm_, this, size_, d, scene_manager_ );
            case Type::RoundedCone:
                return std::make_unique<RoundedConeEditFields>( wm_,
                                                                this,
                                                                size_,
                                                                d,
                                                                scene_manager_ );
            case Type::CappedCone:
                return std::make_unique<CappedConeEditFields>( wm_,
                                                               this,
                                                               size_,
                                                               d,
                                                               scene_manager_ );
            case Type::CappedCylinder:
                return std::make_unique<CappedCylinderEditFields>( wm_,
                                                                   this,
                                                                   size_,
                                                                   d,
                                                                   scene_manager_ );
            case Type::Wedge:
                return std::make_unique<WedgeEditFields>( wm_, this, size_, d, scene_manager_ );
            case Type::Ellipse:
                return std::make_unique<EllipseEditFields>( wm_, this, size_, d, scene_manager_ );
            case Type::Triangle:
                return std::make_unique<TriangleEditFields>( wm_, this, size_, d, scene_manager_ );
        }
        return nullptr;
    }

    void
    switchToAdd()
    {
        editing_mode_ = false;
        content_ = std::make_unique<ObjEditorContentAdd>( *this, creation_type_, scene_manager_ );
        content_->prefill();
    }

    void
    switchToEdit( size_t idx )
    {
        editing_mode_ = true;
        content_ =
            std::make_unique<ObjEditorContentEdit>( *this, current_type_, idx, scene_manager_ );
        content_->prefill();
    }

    void
    wireButtons()
    {
        type_btn_.setParent( this );
        add_btn_.setParent( this );
        copy_btn_.setParent( this );
        del_btn_.setParent( this );
        apply_btn_.setParent( this );

        add_btn_.setOnClick( [this]() {
            if ( auto* add = dynamic_cast<ObjEditorContentAdd*>( content_.get() ) )
            {
                add->onApply();
            }
        } );
        type_btn_.setOnClick( [this]() {
            wm_->pushModal( std::make_unique<TypePickerDialog>(
                wm_,
                getRelPos().x + 20.0f,
                getRelPos().y + 20.0f,
                current_type_,
                [this]( Type t ) {
                    creation_type_ = t;
                    current_type_  = t;
                    if ( auto* add = dynamic_cast<ObjEditorContentAdd*>( content_.get() ) )
                    {
                        add->changeType( t );
                    }
                    wm_->popModal();
                },
                [this]() { wm_->popModal(); } ) );
        } );
        copy_btn_.setOnClick( [this]() { copyTarget(); } );
        del_btn_.setOnClick( [this]() { deleteTarget(); } );
        apply_btn_.setOnClick( [this]() {
            if ( content_ )
                content_->onApply();
        } );
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
        current_type_ =
            typeFromName( scene_manager_.getObjectInfo( target_idx_.value() ).type_name );
        scene_manager_.setTargetObj( scene_manager_.getObjects().back().get() );
        switchToEdit( target_idx_.value() );
        if ( objects_changed_cb_ )
            objects_changed_cb_();
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
        switchToAdd();
        if ( objects_changed_cb_ )
            objects_changed_cb_();
    }

    // Type picker dialog
    class TypePickerDialog : public hui::DialogBox {
      public:
        TypePickerDialog( hui::WindowManager*         wm,
                          float                       x,
                          float                       y,
                          Type                        initial,
                          std::function<void( Type )> on_ok,
                          std::function<void()>       on_cancel )
            : DialogBox( wm, x, y, 220.0f, 260.0f, on_cancel, "Choose Type" ),
              list_( wm, { 10.0f, TopBarHeight + 10.0f }, { 200.0f, 180.0f }, 10.0f ),
              ok_( wm, { 20.0f, 210.0f }, { 80.0f, 26.0f } ),
              cancel_( wm, { 120.0f, 210.0f }, { 80.0f, 26.0f } ),
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
                auto btn = std::make_unique<hui::Button>(
                    wm_,
                    dr4::Vec2f{ 0, 0 },
                    dr4::Vec2f{ 180.0f, 32.0f },
                    label,
                    t == selected_ ? hui::Button::Theme{ hui::Button::DefaultTheme.pressed_color,
                                                         hui::Button::DefaultTheme.hovered_color,
                                                         hui::Button::DefaultTheme.pressed_color,
                                                         hui::Button::DefaultTheme.font_color,
                                                         hui::Button::DefaultTheme.font_size }
                                   : hui::Button::DefaultTheme );
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
        }

      private:
        hui::ScrollableListWidget   list_;
        hui::ButtonOk               ok_;
        hui::ButtonCancel           cancel_;
        std::function<void( Type )> on_ok_;
        std::function<void()>       on_cancel_;
        Type                        selected_ = Type::Sphere;
    };

    const TypeDescriptor&
    descriptor( Type t ) const
    {
        const auto& all = descriptors();
        auto        it  = std::find_if( all.begin(), all.end(), [t]( const TypeDescriptor& d ) {
            return d.type == t;
        } );
        if ( it == all.end() )
        {
            return all.front();
        }
        return *it;
    }

    const std::vector<TypeDescriptor>&
    descriptors() const
    {
        if ( !type_descriptors_.empty() )
            return type_descriptors_;
        auto& ds = type_descriptors_;

        auto fields = []( std::initializer_list<std::pair<std::string, std::string>> defs ) {
            return std::vector<std::pair<std::string, std::string>>( defs );
        };

        auto set_pos_defaults = []( GenericFields& f ) {
            f.setFieldValue( "x", "0.00" );
            f.setFieldValue( "y", "0.00" );
            f.setFieldValue( "z", "-10.00" );
        };

        // Lambdas capture this for setField/parseField via GenericFields casting.
        ds.push_back(
            { Type::Sphere,
              "Sphere",
              fields( { { "X", "x" }, { "Y", "y" }, { "Z", "z" }, { "Radius", "p1" } } ),
              [set_pos_defaults, this]() {
                  if ( auto* f = dynamic_cast<GenericFields*>( contentFields() ) )
                  {
                      set_pos_defaults( *f );
                      f->setFieldValue( "p1", "1.00" );
                  }
              },
              [this]( size_t idx ) {
                  if ( auto* f = dynamic_cast<GenericFields*>( contentFields() ) )
                  {
                      auto& obj = scene_manager_.getObjects()[idx];
                      if ( auto* s = dynamic_cast<model::Sphere*>( obj.get() ) )
                      {
                          auto info = scene_manager_.getObjectInfo( idx );
                          f->setFieldValue( "x", fmt2( info.pos.x ) );
                          f->setFieldValue( "y", fmt2( info.pos.y ) );
                          f->setFieldValue( "z", fmt2( info.pos.z ) );
                          f->setFieldValue( "p1", fmt2( s->getRadius() ) );
                      }
                  }
              },
              [this]( const model::Material& m, const std::string& name ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return std::unique_ptr<model::Primitive>();
                  auto pos = f->parsePos();
                  auto r   = f->parseField( "p1", []( double v ) { return v > 0; } );
                  if ( !pos || !r )
                      return std::unique_ptr<model::Primitive>();
                  auto obj = std::make_unique<model::Sphere>( m, *pos, static_cast<float>( *r ) );
                  obj->setDisplayName( name );
                  return obj;
              },
              [this]( const model::Material& m, const std::string& name, model::Primitive& p ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  auto* s = dynamic_cast<model::Sphere*>( &p );
                  if ( !f || !s )
                      return false;
                  auto pos = f->parsePos();
                  auto r   = f->parseField( "p1", []( double v ) { return v > 0; } );
                  if ( !pos || !r )
                      return false;
                  s->setOrigin( *pos );
                  s->setMaterial( m );
                  s->setRadius( static_cast<float>( *r ) );
                  s->setDisplayName( name );
                  return true;
              } } );

        ds.push_back(
            { Type::AABB,
              "AABB",
              fields( { { "X", "x" },
                        { "Y", "y" },
                        { "Z", "z" },
                        { "Half size X", "p1" },
                        { "Half size Y", "p2" },
                        { "Half size Z", "p3" } } ),
              [set_pos_defaults, this]() {
                  if ( auto* f = dynamic_cast<GenericFields*>( contentFields() ) )
                  {
                      set_pos_defaults( *f );
                      f->setFieldValue( "p1", "1.00" );
                      f->setFieldValue( "p2", "1.00" );
                      f->setFieldValue( "p3", "1.00" );
                  }
              },
              [this]( size_t idx ) {
                  if ( auto* f = dynamic_cast<GenericFields*>( contentFields() ) )
                  {
                      auto& obj = scene_manager_.getObjects()[idx];
                      if ( auto* a = dynamic_cast<model::AABB*>( obj.get() ) )
                      {
                          auto info = scene_manager_.getObjectInfo( idx );
                          f->setFieldValue( "x", fmt2( info.pos.x ) );
                          f->setFieldValue( "y", fmt2( info.pos.y ) );
                          f->setFieldValue( "z", fmt2( info.pos.z ) );
                          auto hs = a->getHalfSize();
                          f->setFieldValue( "p1", fmt2( hs.x ) );
                          f->setFieldValue( "p2", fmt2( hs.y ) );
                          f->setFieldValue( "p3", fmt2( hs.z ) );
                      }
                  }
              },
              [this]( const model::Material& m, const std::string& name ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return std::unique_ptr<model::Primitive>();
                  auto pos = f->parsePos();
                  auto p1  = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto p2  = f->parseField( "p2", []( double v ) { return v > 0; } );
                  auto p3  = f->parseField( "p3", []( double v ) { return v > 0; } );
                  if ( !pos || !p1 || !p2 || !p3 )
                      return std::unique_ptr<model::Primitive>();
                  auto obj =
                      std::make_unique<model::AABB>( m,
                                                     *pos,
                                                     model::Vector3f( static_cast<float>( *p1 ),
                                                                      static_cast<float>( *p2 ),
                                                                      static_cast<float>( *p3 ) ) );
                  obj->setDisplayName( name );
                  return obj;
              },
              [this]( const model::Material& m, const std::string& name, model::Primitive& p ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  auto* a = dynamic_cast<model::AABB*>( &p );
                  if ( !f || !a )
                      return false;
                  auto pos = f->parsePos();
                  auto p1  = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto p2  = f->parseField( "p2", []( double v ) { return v > 0; } );
                  auto p3  = f->parseField( "p3", []( double v ) { return v > 0; } );
                  if ( !pos || !p1 || !p2 || !p3 )
                      return false;
                  a->setOrigin( *pos );
                  a->setMaterial( m );
                  a->setHalfSize( model::Vector3f( static_cast<float>( *p1 ),
                                                   static_cast<float>( *p2 ),
                                                   static_cast<float>( *p3 ) ) );
                  a->setDisplayName( name );
                  return true;
              } } );

        ds.push_back(
            { Type::Torus,
              "Torus",
              fields( { { "X", "x" },
                        { "Y", "y" },
                        { "Z", "z" },
                        { "Major radius", "p1" },
                        { "Minor radius", "p2" } } ),
              [this]() {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( f )
                  {
                      f->setFieldValue( "x", "0.00" );
                      f->setFieldValue( "y", "0.00" );
                      f->setFieldValue( "z", "-10.00" );
                      f->setFieldValue( "p1", "2.00" );
                      f->setFieldValue( "p2", "0.80" );
                  }
              },
              [this]( size_t idx ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return;
                  auto& obj = scene_manager_.getObjects()[idx];
                  if ( auto* t = dynamic_cast<model::Torus*>( obj.get() ) )
                  {
                      auto info = scene_manager_.getObjectInfo( idx );
                      f->setFieldValue( "x", fmt2( info.pos.x ) );
                      f->setFieldValue( "y", fmt2( info.pos.y ) );
                      f->setFieldValue( "z", fmt2( info.pos.z ) );
                      f->setFieldValue( "p1", fmt2( t->getMajorRadius() ) );
                      f->setFieldValue( "p2", fmt2( t->getMinorRadius() ) );
                  }
              },
              [this]( const model::Material& m, const std::string& name ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return std::unique_ptr<model::Primitive>();
                  auto pos = f->parsePos();
                  auto p1  = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto p2  = f->parseField( "p2", []( double v ) { return v > 0; } );
                  if ( !pos || !p1 || !p2 )
                      return std::unique_ptr<model::Primitive>();
                  auto obj = std::make_unique<model::Torus>( m,
                                                             *pos,
                                                             static_cast<float>( *p1 ),
                                                             static_cast<float>( *p2 ) );
                  obj->setDisplayName( name );
                  return obj;
              },
              [this]( const model::Material& m, const std::string& name, model::Primitive& p ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  auto* t = dynamic_cast<model::Torus*>( &p );
                  if ( !f || !t )
                      return false;
                  auto pos = f->parsePos();
                  auto p1  = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto p2  = f->parseField( "p2", []( double v ) { return v > 0; } );
                  if ( !pos || !p1 || !p2 )
                      return false;
                  t->setOrigin( *pos );
                  t->setMaterial( m );
                  t->setMajorRadius( static_cast<float>( *p1 ) );
                  t->setMinorRadius( static_cast<float>( *p2 ) );
                  t->setDisplayName( name );
                  return true;
              } } );

        ds.push_back(
            { Type::HexPrism,
              "HexPrism",
              fields( { { "X", "x" },
                        { "Y", "y" },
                        { "Z", "z" },
                        { "Radius", "p1" },
                        { "Height", "p2" } } ),
              [this]() {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( f )
                  {
                      f->setFieldValue( "x", "0.00" );
                      f->setFieldValue( "y", "0.00" );
                      f->setFieldValue( "z", "-10.00" );
                      f->setFieldValue( "p1", "1.00" );
                      f->setFieldValue( "p2", "2.00" );
                  }
              },
              [this]( size_t idx ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return;
                  auto& obj = scene_manager_.getObjects()[idx];
                  if ( auto* h = dynamic_cast<model::HexPrism*>( obj.get() ) )
                  {
                      auto info = scene_manager_.getObjectInfo( idx );
                      f->setFieldValue( "x", fmt2( info.pos.x ) );
                      f->setFieldValue( "y", fmt2( info.pos.y ) );
                      f->setFieldValue( "z", fmt2( info.pos.z ) );
                      f->setFieldValue( "p1", fmt2( h->getRadius() ) );
                      f->setFieldValue( "p2", fmt2( h->getHeight() ) );
                  }
              },
              [this]( const model::Material& m, const std::string& name ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return std::unique_ptr<rta::model::Primitive>();
                  auto pos = f->parsePos();
                  auto p1  = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto p2  = f->parseField( "p2", []( double v ) { return v > 0; } );
                  if ( !pos || !p1 || !p2 )
                      return std::unique_ptr<rta::model::Primitive>();
                  auto obj = std::make_unique<rta::model::HexPrism>( m,
                                                                     *pos,
                                                                     static_cast<float>( *p1 ),
                                                                     static_cast<float>( *p2 ) );
                  obj->setDisplayName( name );
                  return obj;
              },
              [this]( const model::Material& m, const std::string& name, model::Primitive& p ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  auto* h = dynamic_cast<model::HexPrism*>( &p );
                  if ( !f || !h )
                      return false;
                  auto pos = f->parsePos();
                  auto p1  = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto p2  = f->parseField( "p2", []( double v ) { return v > 0; } );
                  if ( !pos || !p1 || !p2 )
                      return false;
                  h->setOrigin( *pos );
                  h->setMaterial( m );
                  h->setRadius( static_cast<float>( *p1 ) );
                  h->setHeight( static_cast<float>( *p2 ) );
                  h->setDisplayName( name );
                  return true;
              } } );

        ds.push_back(
            { Type::Goursat,
              "Goursat",
              fields(
                  { { "X", "x" }, { "Y", "y" }, { "Z", "z" }, { "ka", "ka" }, { "kb", "kb" } } ),
              [this]() {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( f )
                  {
                      f->setFieldValue( "x", "0.00" );
                      f->setFieldValue( "y", "0.00" );
                      f->setFieldValue( "z", "-10.00" );
                      f->setFieldValue( "ka", "1.00" );
                      f->setFieldValue( "kb", "1.00" );
                  }
              },
              [this]( size_t idx ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return;
                  auto& obj = scene_manager_.getObjects()[idx];
                  if ( auto* g = dynamic_cast<model::Goursat*>( obj.get() ) )
                  {
                      auto info = scene_manager_.getObjectInfo( idx );
                      f->setFieldValue( "x", fmt2( info.pos.x ) );
                      f->setFieldValue( "y", fmt2( info.pos.y ) );
                      f->setFieldValue( "z", fmt2( info.pos.z ) );
                      f->setFieldValue( "ka", fmt2( g->getKa() ) );
                      f->setFieldValue( "kb", fmt2( g->getKb() ) );
                  }
              },
              [this]( const model::Material& m, const std::string& name ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return std::unique_ptr<model::Primitive>();
                  auto pos = f->parsePos();
                  auto ka  = f->parseField( "ka", []( double v ) { return v > 0; } );
                  auto kb  = f->parseField( "kb", []( double v ) { return v > 0; } );
                  if ( !pos || !ka || !kb )
                      return std::unique_ptr<model::Primitive>();
                  auto obj = std::make_unique<model::Goursat>( m,
                                                               *pos,
                                                               static_cast<float>( *ka ),
                                                               static_cast<float>( *kb ) );
                  obj->setDisplayName( name );
                  return obj;
              },
              [this]( const model::Material& m, const std::string& name, model::Primitive& p ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  auto* g = dynamic_cast<model::Goursat*>( &p );
                  if ( !f || !g )
                      return false;
                  auto pos = f->parsePos();
                  auto ka  = f->parseField( "ka", []( double v ) { return v > 0; } );
                  auto kb  = f->parseField( "kb", []( double v ) { return v > 0; } );
                  if ( !pos || !ka || !kb )
                      return false;
                  g->setOrigin( *pos );
                  g->setMaterial( m );
                  g->setKa( static_cast<float>( *ka ) );
                  g->setKb( static_cast<float>( *kb ) );
                  g->setDisplayName( name );
                  return true;
              } } );

        ds.push_back(
            { Type::RoundedBox,
              "RoundedBox",
              fields( { { "X", "x" },
                        { "Y", "y" },
                        { "Z", "z" },
                        { "Half size X", "p1" },
                        { "Half size Y", "p2" },
                        { "Half size Z", "p3" },
                        { "Radius", "p4" } } ),
              [this]() {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( f )
                  {
                      f->setFieldValue( "x", "0.00" );
                      f->setFieldValue( "y", "0.00" );
                      f->setFieldValue( "z", "-10.00" );
                      f->setFieldValue( "p1", "1.00" );
                      f->setFieldValue( "p2", "1.00" );
                      f->setFieldValue( "p3", "1.00" );
                      f->setFieldValue( "p4", "0.20" );
                  }
              },
              [this]( size_t idx ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return;
                  auto& obj = scene_manager_.getObjects()[idx];
                  if ( auto* rb = dynamic_cast<model::RoundedBox*>( obj.get() ) )
                  {
                      auto info = scene_manager_.getObjectInfo( idx );
                      f->setFieldValue( "x", fmt2( info.pos.x ) );
                      f->setFieldValue( "y", fmt2( info.pos.y ) );
                      f->setFieldValue( "z", fmt2( info.pos.z ) );
                      auto hs = rb->getHalfSize();
                      f->setFieldValue( "p1", fmt2( hs.x ) );
                      f->setFieldValue( "p2", fmt2( hs.y ) );
                      f->setFieldValue( "p3", fmt2( hs.z ) );
                      f->setFieldValue( "p4", fmt2( rb->getRadius() ) );
                  }
              },
              [this]( const model::Material& m, const std::string& name ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return std::unique_ptr<model::Primitive>();
                  auto pos = f->parsePos();
                  auto p1  = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto p2  = f->parseField( "p2", []( double v ) { return v > 0; } );
                  auto p3  = f->parseField( "p3", []( double v ) { return v > 0; } );
                  auto p4  = f->parseField( "p4", []( double v ) { return v > 0; } );
                  if ( !pos || !p1 || !p2 || !p3 || !p4 )
                      return std::unique_ptr<model::Primitive>();
                  auto obj = std::make_unique<model::RoundedBox>(
                      m,
                      *pos,
                      model::Vector3f( static_cast<float>( *p1 ),
                                       static_cast<float>( *p2 ),
                                       static_cast<float>( *p3 ) ),
                      static_cast<float>( *p4 ) );
                  obj->setDisplayName( name );
                  return obj;
              },
              [this]( const model::Material& m, const std::string& name, model::Primitive& p ) {
                  auto* f  = dynamic_cast<GenericFields*>( contentFields() );
                  auto* rb = dynamic_cast<model::RoundedBox*>( &p );
                  if ( !f || !rb )
                      return false;
                  auto pos = f->parsePos();
                  auto p1  = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto p2  = f->parseField( "p2", []( double v ) { return v > 0; } );
                  auto p3  = f->parseField( "p3", []( double v ) { return v > 0; } );
                  auto p4  = f->parseField( "p4", []( double v ) { return v > 0; } );
                  if ( !pos || !p1 || !p2 || !p3 || !p4 )
                      return false;
                  rb->setOrigin( *pos );
                  rb->setMaterial( m );
                  rb->setHalfSize( model::Vector3f( static_cast<float>( *p1 ),
                                                    static_cast<float>( *p2 ),
                                                    static_cast<float>( *p3 ) ) );
                  rb->setRadius( static_cast<float>( *p4 ) );
                  rb->setDisplayName( name );
                  return true;
              } } );

        ds.push_back(
            { Type::Ellipsoid,
              "Ellipsoid",
              fields( { { "X", "x" },
                        { "Y", "y" },
                        { "Z", "z" },
                        { "Radius X", "p1" },
                        { "Radius Y", "p2" },
                        { "Radius Z", "p3" } } ),
              [this]() {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( f )
                  {
                      f->setFieldValue( "x", "0.00" );
                      f->setFieldValue( "y", "0.00" );
                      f->setFieldValue( "z", "-10.00" );
                      f->setFieldValue( "p1", "1.00" );
                      f->setFieldValue( "p2", "1.50" );
                      f->setFieldValue( "p3", "0.75" );
                  }
              },
              [this]( size_t idx ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return;
                  auto& obj = scene_manager_.getObjects()[idx];
                  if ( auto* e = dynamic_cast<model::Ellipsoid*>( obj.get() ) )
                  {
                      auto info = scene_manager_.getObjectInfo( idx );
                      f->setFieldValue( "x", fmt2( info.pos.x ) );
                      f->setFieldValue( "y", fmt2( info.pos.y ) );
                      f->setFieldValue( "z", fmt2( info.pos.z ) );
                      auto r = e->getRadii();
                      f->setFieldValue( "p1", fmt2( r.x ) );
                      f->setFieldValue( "p2", fmt2( r.y ) );
                      f->setFieldValue( "p3", fmt2( r.z ) );
                  }
              },
              [this]( const model::Material& m, const std::string& name ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return std::unique_ptr<model::Primitive>();
                  auto pos = f->parsePos();
                  auto p1  = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto p2  = f->parseField( "p2", []( double v ) { return v > 0; } );
                  auto p3  = f->parseField( "p3", []( double v ) { return v > 0; } );
                  if ( !pos || !p1 || !p2 || !p3 )
                      return std::unique_ptr<model::Primitive>();
                  auto obj = std::make_unique<model::Ellipsoid>(
                      m,
                      *pos,
                      model::Vector3f( static_cast<float>( *p1 ),
                                       static_cast<float>( *p2 ),
                                       static_cast<float>( *p3 ) ) );
                  obj->setDisplayName( name );
                  return obj;
              },
              [this]( const model::Material& m, const std::string& name, model::Primitive& p ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  auto* e = dynamic_cast<model::Ellipsoid*>( &p );
                  if ( !f || !e )
                      return false;
                  auto pos = f->parsePos();
                  auto p1  = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto p2  = f->parseField( "p2", []( double v ) { return v > 0; } );
                  auto p3  = f->parseField( "p3", []( double v ) { return v > 0; } );
                  if ( !pos || !p1 || !p2 || !p3 )
                      return false;
                  e->setOrigin( *pos );
                  e->setMaterial( m );
                  e->setRadii( model::Vector3f( static_cast<float>( *p1 ),
                                                static_cast<float>( *p2 ),
                                                static_cast<float>( *p3 ) ) );
                  e->setDisplayName( name );
                  return true;
              } } );

        ds.push_back(
            { Type::Capsule,
              "Capsule",
              fields( { { "X", "x" },
                        { "Y", "y" },
                        { "Z", "z" },
                        { "Height", "p1" },
                        { "Radius", "p2" } } ),
              [this]() {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( f )
                  {
                      f->setFieldValue( "x", "0.00" );
                      f->setFieldValue( "y", "0.00" );
                      f->setFieldValue( "z", "-10.00" );
                      f->setFieldValue( "p1", "2.00" );
                      f->setFieldValue( "p2", "0.50" );
                  }
              },
              [this]( size_t idx ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return;
                  auto& obj = scene_manager_.getObjects()[idx];
                  if ( auto* c = dynamic_cast<model::Capsule*>( obj.get() ) )
                  {
                      auto info = scene_manager_.getObjectInfo( idx );
                      f->setFieldValue( "x", fmt2( info.pos.x ) );
                      f->setFieldValue( "y", fmt2( info.pos.y ) );
                      f->setFieldValue( "z", fmt2( info.pos.z ) );
                      auto  pa = c->getPaLocal();
                      auto  pb = c->getPbLocal();
                      float h  = std::abs( ( pb - pa ).y );
                      f->setFieldValue( "p1", fmt2( h ) );
                      f->setFieldValue( "p2", fmt2( c->getRadius() ) );
                  }
              },
              [this]( const model::Material& m, const std::string& name ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return std::unique_ptr<model::Primitive>();
                  auto pos = f->parsePos();
                  auto h   = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto r   = f->parseField( "p2", []( double v ) { return v > 0; } );
                  if ( !pos || !h || !r )
                      return std::unique_ptr<model::Primitive>();
                  float fh = static_cast<float>( *h );
                  auto  obj =
                      std::make_unique<model::Capsule>( m,
                                                        *pos,
                                                        model::Vector3f{ 0.0f, -0.5f * fh, 0.0f },
                                                        model::Vector3f{ 0.0f, 0.5f * fh, 0.0f },
                                                        static_cast<float>( *r ) );
                  obj->setDisplayName( name );
                  return obj;
              },
              [this]( const model::Material& m, const std::string& name, model::Primitive& p ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  auto* c = dynamic_cast<model::Capsule*>( &p );
                  if ( !f || !c )
                      return false;
                  auto pos = f->parsePos();
                  auto h   = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto r   = f->parseField( "p2", []( double v ) { return v > 0; } );
                  if ( !pos || !h || !r )
                      return false;
                  float fh = static_cast<float>( *h );
                  c->setOrigin( *pos );
                  c->setMaterial( m );
                  c->setPaLocal( { 0.0f, -0.5f * fh, 0.0f } );
                  c->setPbLocal( { 0.0f, 0.5f * fh, 0.0f } );
                  c->setRadius( static_cast<float>( *r ) );
                  c->setDisplayName( name );
                  return true;
              } } );

        ds.push_back(
            { Type::RoundedCone,
              "RoundedCone",
              fields( { { "X", "x" },
                        { "Y", "y" },
                        { "Z", "z" },
                        { "Height", "p1" },
                        { "Radius A", "p2" },
                        { "Radius B", "p3" } } ),
              [this]() {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( f )
                  {
                      f->setFieldValue( "x", "0.00" );
                      f->setFieldValue( "y", "0.00" );
                      f->setFieldValue( "z", "-10.00" );
                      f->setFieldValue( "p1", "2.00" );
                      f->setFieldValue( "p2", "1.00" );
                      f->setFieldValue( "p3", "0.50" );
                  }
              },
              [this]( size_t idx ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return;
                  auto& obj = scene_manager_.getObjects()[idx];
                  if ( auto* rc = dynamic_cast<model::RoundedCone*>( obj.get() ) )
                  {
                      auto info = scene_manager_.getObjectInfo( idx );
                      f->setFieldValue( "x", fmt2( info.pos.x ) );
                      f->setFieldValue( "y", fmt2( info.pos.y ) );
                      f->setFieldValue( "z", fmt2( info.pos.z ) );
                      auto  pa = rc->getPaLocal();
                      auto  pb = rc->getPbLocal();
                      float h  = std::abs( ( pb - pa ).y );
                      f->setFieldValue( "p1", fmt2( h ) );
                      f->setFieldValue( "p2", fmt2( rc->getRadiusA() ) );
                      f->setFieldValue( "p3", fmt2( rc->getRadiusB() ) );
                  }
              },
              [this]( const model::Material& m, const std::string& name ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return std::unique_ptr<model::Primitive>();
                  auto pos = f->parsePos();
                  auto h   = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto ra  = f->parseField( "p2", []( double v ) { return v > 0; } );
                  auto rb  = f->parseField( "p3", []( double v ) { return v > 0; } );
                  if ( !pos || !h || !ra || !rb )
                      return std::unique_ptr<model::Primitive>();
                  float fh  = static_cast<float>( *h );
                  auto  obj = std::make_unique<model::RoundedCone>(
                      m,
                      *pos,
                      model::Vector3f{ 0.0f, -0.5f * fh, 0.0f },
                      model::Vector3f{ 0.0f, 0.5f * fh, 0.0f },
                      static_cast<float>( *ra ),
                      static_cast<float>( *rb ) );
                  obj->setDisplayName( name );
                  return obj;
              },
              [this]( const model::Material& m, const std::string& name, model::Primitive& p ) {
                  auto* f  = dynamic_cast<GenericFields*>( contentFields() );
                  auto* rc = dynamic_cast<model::RoundedCone*>( &p );
                  if ( !f || !rc )
                      return false;
                  auto pos = f->parsePos();
                  auto h   = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto ra  = f->parseField( "p2", []( double v ) { return v > 0; } );
                  auto rb  = f->parseField( "p3", []( double v ) { return v > 0; } );
                  if ( !pos || !h || !ra || !rb )
                      return false;
                  float fh = static_cast<float>( *h );
                  rc->setOrigin( *pos );
                  rc->setMaterial( m );
                  rc->setPaLocal( { 0.0f, -0.5f * fh, 0.0f } );
                  rc->setPbLocal( { 0.0f, 0.5f * fh, 0.0f } );
                  rc->setRadiusA( static_cast<float>( *ra ) );
                  rc->setRadiusB( static_cast<float>( *rb ) );
                  rc->setDisplayName( name );
                  return true;
              } } );

        ds.push_back(
            { Type::CappedCone,
              "CappedCone",
              fields( { { "X", "x" },
                        { "Y", "y" },
                        { "Z", "z" },
                        { "Height", "p1" },
                        { "Radius A", "p2" },
                        { "Radius B", "p3" } } ),
              [this]() {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( f )
                  {
                      f->setFieldValue( "x", "0.00" );
                      f->setFieldValue( "y", "0.00" );
                      f->setFieldValue( "z", "-10.00" );
                      f->setFieldValue( "p1", "2.00" );
                      f->setFieldValue( "p2", "1.00" );
                      f->setFieldValue( "p3", "0.50" );
                  }
              },
              [this]( size_t idx ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return;
                  auto& obj = scene_manager_.getObjects()[idx];
                  if ( auto* cc = dynamic_cast<model::CappedCone*>( obj.get() ) )
                  {
                      auto info = scene_manager_.getObjectInfo( idx );
                      f->setFieldValue( "x", fmt2( info.pos.x ) );
                      f->setFieldValue( "y", fmt2( info.pos.y ) );
                      f->setFieldValue( "z", fmt2( info.pos.z ) );
                      auto  pa = cc->getPaLocal();
                      auto  pb = cc->getPbLocal();
                      float h  = std::abs( ( pb - pa ).y );
                      f->setFieldValue( "p1", fmt2( h ) );
                      f->setFieldValue( "p2", fmt2( cc->getRadiusA() ) );
                      f->setFieldValue( "p3", fmt2( cc->getRadiusB() ) );
                  }
              },
              [this]( const model::Material& m, const std::string& name ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return std::unique_ptr<model::Primitive>();
                  auto pos = f->parsePos();
                  auto h   = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto ra  = f->parseField( "p2", []( double v ) { return v > 0; } );
                  auto rb  = f->parseField( "p3", []( double v ) { return v > 0; } );
                  if ( !pos || !h || !ra || !rb )
                      return std::unique_ptr<model::Primitive>();
                  float fh  = static_cast<float>( *h );
                  auto  obj = std::make_unique<model::CappedCone>(
                      m,
                      *pos,
                      model::Vector3f{ 0.0f, -0.5f * fh, 0.0f },
                      model::Vector3f{ 0.0f, 0.5f * fh, 0.0f },
                      static_cast<float>( *ra ),
                      static_cast<float>( *rb ) );
                  obj->setDisplayName( name );
                  return obj;
              },
              [this]( const model::Material& m, const std::string& name, model::Primitive& p ) {
                  auto* f  = dynamic_cast<GenericFields*>( contentFields() );
                  auto* cc = dynamic_cast<model::CappedCone*>( &p );
                  if ( !f || !cc )
                      return false;
                  auto pos = f->parsePos();
                  auto h   = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto ra  = f->parseField( "p2", []( double v ) { return v > 0; } );
                  auto rb  = f->parseField( "p3", []( double v ) { return v > 0; } );
                  if ( !pos || !h || !ra || !rb )
                      return false;
                  float fh = static_cast<float>( *h );
                  cc->setOrigin( *pos );
                  cc->setMaterial( m );
                  cc->setPaLocal( { 0.0f, -0.5f * fh, 0.0f } );
                  cc->setPbLocal( { 0.0f, 0.5f * fh, 0.0f } );
                  cc->setRadiusA( static_cast<float>( *ra ) );
                  cc->setRadiusB( static_cast<float>( *rb ) );
                  cc->setDisplayName( name );
                  return true;
              } } );

        ds.push_back(
            { Type::CappedCylinder,
              "CappedCylinder",
              fields( { { "X", "x" },
                        { "Y", "y" },
                        { "Z", "z" },
                        { "Height", "p1" },
                        { "Radius", "p2" } } ),
              [this]() {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( f )
                  {
                      f->setFieldValue( "x", "0.00" );
                      f->setFieldValue( "y", "0.00" );
                      f->setFieldValue( "z", "-10.00" );
                      f->setFieldValue( "p1", "2.00" );
                      f->setFieldValue( "p2", "0.50" );
                  }
              },
              [this]( size_t idx ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return;
                  auto& obj = scene_manager_.getObjects()[idx];
                  if ( auto* c = dynamic_cast<model::CappedCylinder*>( obj.get() ) )
                  {
                      auto info = scene_manager_.getObjectInfo( idx );
                      f->setFieldValue( "x", fmt2( info.pos.x ) );
                      f->setFieldValue( "y", fmt2( info.pos.y ) );
                      f->setFieldValue( "z", fmt2( info.pos.z ) );
                      auto  a = c->getALocal();
                      auto  b = c->getBLocal();
                      float h = std::abs( ( b - a ).y );
                      f->setFieldValue( "p1", fmt2( h ) );
                      f->setFieldValue( "p2", fmt2( c->getRadius() ) );
                  }
              },
              [this]( const model::Material& m, const std::string& name ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return std::unique_ptr<model::Primitive>();
                  auto pos = f->parsePos();
                  auto h   = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto r   = f->parseField( "p2", []( double v ) { return v > 0; } );
                  if ( !pos || !h || !r )
                      return std::unique_ptr<model::Primitive>();
                  float fh  = static_cast<float>( *h );
                  auto  obj = std::make_unique<model::CappedCylinder>(
                      m,
                      *pos,
                      model::Vector3f{ 0.0f, -0.5f * fh, 0.0f },
                      model::Vector3f{ 0.0f, 0.5f * fh, 0.0f },
                      static_cast<float>( *r ) );
                  obj->setDisplayName( name );
                  return obj;
              },
              [this]( const model::Material& m, const std::string& name, model::Primitive& p ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  auto* c = dynamic_cast<model::CappedCylinder*>( &p );
                  if ( !f || !c )
                      return false;
                  auto pos = f->parsePos();
                  auto h   = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto r   = f->parseField( "p2", []( double v ) { return v > 0; } );
                  if ( !pos || !h || !r )
                      return false;
                  float fh = static_cast<float>( *h );
                  c->setOrigin( *pos );
                  c->setMaterial( m );
                  c->setALocal( { 0.0f, -0.5f * fh, 0.0f } );
                  c->setBLocal( { 0.0f, 0.5f * fh, 0.0f } );
                  c->setRadius( static_cast<float>( *r ) );
                  c->setDisplayName( name );
                  return true;
              } } );

        ds.push_back(
            { Type::Wedge,
              "Wedge",
              fields( { { "X", "x" },
                        { "Y", "y" },
                        { "Z", "z" },
                        { "Half size X", "p1" },
                        { "Half size Y", "p2" },
                        { "Half size Z", "p3" } } ),
              [this]() {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( f )
                  {
                      f->setFieldValue( "x", "0.00" );
                      f->setFieldValue( "y", "0.00" );
                      f->setFieldValue( "z", "-10.00" );
                      f->setFieldValue( "p1", "1.00" );
                      f->setFieldValue( "p2", "1.00" );
                      f->setFieldValue( "p3", "1.00" );
                  }
              },
              [this]( size_t idx ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return;
                  auto& obj = scene_manager_.getObjects()[idx];
                  if ( auto* w = dynamic_cast<model::Wedge*>( obj.get() ) )
                  {
                      auto info = scene_manager_.getObjectInfo( idx );
                      f->setFieldValue( "x", fmt2( info.pos.x ) );
                      f->setFieldValue( "y", fmt2( info.pos.y ) );
                      f->setFieldValue( "z", fmt2( info.pos.z ) );
                      auto s = w->getS();
                      f->setFieldValue( "p1", fmt2( s.x ) );
                      f->setFieldValue( "p2", fmt2( s.y ) );
                      f->setFieldValue( "p3", fmt2( s.z ) );
                  }
              },
              [this]( const model::Material& m, const std::string& name ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return std::unique_ptr<model::Primitive>();
                  auto pos = f->parsePos();
                  auto p1  = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto p2  = f->parseField( "p2", []( double v ) { return v > 0; } );
                  auto p3  = f->parseField( "p3", []( double v ) { return v > 0; } );
                  if ( !pos || !p1 || !p2 || !p3 )
                      return std::unique_ptr<model::Primitive>();
                  auto obj = std::make_unique<model::Wedge>(
                      m,
                      *pos,
                      model::Vector3f( static_cast<float>( *p1 ),
                                       static_cast<float>( *p2 ),
                                       static_cast<float>( *p3 ) ) );
                  obj->setDisplayName( name );
                  return obj;
              },
              [this]( const model::Material& m, const std::string& name, model::Primitive& p ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  auto* w = dynamic_cast<model::Wedge*>( &p );
                  if ( !f || !w )
                      return false;
                  auto pos = f->parsePos();
                  auto p1  = f->parseField( "p1", []( double v ) { return v > 0; } );
                  auto p2  = f->parseField( "p2", []( double v ) { return v > 0; } );
                  auto p3  = f->parseField( "p3", []( double v ) { return v > 0; } );
                  if ( !pos || !p1 || !p2 || !p3 )
                      return false;
                  w->setOrigin( *pos );
                  w->setMaterial( m );
                  w->setS( model::Vector3f( static_cast<float>( *p1 ),
                                            static_cast<float>( *p2 ),
                                            static_cast<float>( *p3 ) ) );
                  w->setDisplayName( name );
                  return true;
              } } );

        ds.push_back(
            { Type::Ellipse,
              "Ellipse",
              fields( { { "X", "x" },
                        { "Y", "y" },
                        { "Z", "z" },
                        { "U.x", "ux" },
                        { "U.y", "uy" },
                        { "U.z", "uz" },
                        { "V.x", "vx" },
                        { "V.y", "vy" },
                        { "V.z", "vz" } } ),
              [this]() {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( f )
                  {
                      f->setFieldValue( "x", "0.00" );
                      f->setFieldValue( "y", "0.00" );
                      f->setFieldValue( "z", "-10.00" );
                      f->setFieldValue( "ux", "1.00" );
                      f->setFieldValue( "uy", "0.00" );
                      f->setFieldValue( "uz", "0.00" );
                      f->setFieldValue( "vx", "0.00" );
                      f->setFieldValue( "vy", "1.00" );
                      f->setFieldValue( "vz", "0.00" );
                  }
              },
              [this]( size_t idx ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return;
                  auto& obj = scene_manager_.getObjects()[idx];
                  if ( auto* e = dynamic_cast<model::Ellipse*>( obj.get() ) )
                  {
                      auto info = scene_manager_.getObjectInfo( idx );
                      f->setFieldValue( "x", fmt2( info.pos.x ) );
                      f->setFieldValue( "y", fmt2( info.pos.y ) );
                      f->setFieldValue( "z", fmt2( info.pos.z ) );
                      auto u = e->getU();
                      auto v = e->getV();
                      f->setFieldValue( "ux", fmt2( u.x ) );
                      f->setFieldValue( "uy", fmt2( u.y ) );
                      f->setFieldValue( "uz", fmt2( u.z ) );
                      f->setFieldValue( "vx", fmt2( v.x ) );
                      f->setFieldValue( "vy", fmt2( v.y ) );
                      f->setFieldValue( "vz", fmt2( v.z ) );
                  }
              },
              [this]( const model::Material& m, const std::string& name ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return std::unique_ptr<model::Primitive>();
                  auto pos = f->parsePos();
                  auto ux  = f->parseField( "ux", []( double ) { return true; } );
                  auto uy  = f->parseField( "uy", []( double ) { return true; } );
                  auto uz  = f->parseField( "uz", []( double ) { return true; } );
                  auto vx  = f->parseField( "vx", []( double ) { return true; } );
                  auto vy  = f->parseField( "vy", []( double ) { return true; } );
                  auto vz  = f->parseField( "vz", []( double ) { return true; } );
                  if ( !pos || !ux || !uy || !uz || !vx || !vy || !vz )
                      return std::unique_ptr<model::Primitive>();
                  auto obj = std::make_unique<model::Ellipse>(
                      m,
                      *pos,
                      model::Vector3f( static_cast<float>( *ux ),
                                       static_cast<float>( *uy ),
                                       static_cast<float>( *uz ) ),
                      model::Vector3f( static_cast<float>( *vx ),
                                       static_cast<float>( *vy ),
                                       static_cast<float>( *vz ) ) );
                  obj->setDisplayName( name );
                  return obj;
              },
              [this]( const model::Material& m, const std::string& name, model::Primitive& p ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  auto* e = dynamic_cast<model::Ellipse*>( &p );
                  if ( !f || !e )
                      return false;
                  auto pos = f->parsePos();
                  auto ux  = f->parseField( "ux", []( double ) { return true; } );
                  auto uy  = f->parseField( "uy", []( double ) { return true; } );
                  auto uz  = f->parseField( "uz", []( double ) { return true; } );
                  auto vx  = f->parseField( "vx", []( double ) { return true; } );
                  auto vy  = f->parseField( "vy", []( double ) { return true; } );
                  auto vz  = f->parseField( "vz", []( double ) { return true; } );
                  if ( !pos || !ux || !uy || !uz || !vx || !vy || !vz )
                      return false;
                  e->setOrigin( *pos );
                  e->setMaterial( m );
                  e->setU( model::Vector3f( static_cast<float>( *ux ),
                                            static_cast<float>( *uy ),
                                            static_cast<float>( *uz ) ) );
                  e->setV( model::Vector3f( static_cast<float>( *vx ),
                                            static_cast<float>( *vy ),
                                            static_cast<float>( *vz ) ) );
                  e->setDisplayName( name );
                  return true;
              } } );

        ds.push_back(
            { Type::Triangle,
              "Triangle",
              fields( { { "V0.x", "v0x" },
                        { "V0.y", "v0y" },
                        { "V0.z", "v0z" },
                        { "V1.x", "v1x" },
                        { "V1.y", "v1y" },
                        { "V1.z", "v1z" },
                        { "V2.x", "v2x" },
                        { "V2.y", "v2y" },
                        { "V2.z", "v2z" } } ),
              [this]() {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( f )
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
                          f->setFieldValue( p.first, p.second );
                  }
              },
              [this]( size_t idx ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return;
                  auto& obj = scene_manager_.getObjects()[idx];
                  if ( auto* t = dynamic_cast<model::Triangle*>( obj.get() ) )
                  {
                      auto set = [&]( const char* key, const model::Vector3f& v, int idx ) {
                          float val = idx == 0 ? v.x : ( idx == 1 ? v.y : v.z );
                          f->setFieldValue( key, fmt2( val ) );
                      };
                      auto v0 = t->getV0();
                      auto v1 = t->getV1();
                      auto v2 = t->getV2();
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
              },
              [this]( const model::Material& m, const std::string& name ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  if ( !f )
                      return std::unique_ptr<model::Primitive>();
                  auto fetch = [&]( const char* key ) {
                      return f->parseField( key, []( double ) { return true; } );
                  };
                  auto v0x = fetch( "v0x" ), v0y = fetch( "v0y" ), v0z = fetch( "v0z" );
                  auto v1x = fetch( "v1x" ), v1y = fetch( "v1y" ), v1z = fetch( "v1z" );
                  auto v2x = fetch( "v2x" ), v2y = fetch( "v2y" ), v2z = fetch( "v2z" );
                  if ( !v0x || !v0y || !v0z || !v1x || !v1y || !v1z || !v2x || !v2y || !v2z )
                      return std::unique_ptr<model::Primitive>();
                  auto obj = std::make_unique<model::Triangle>(
                      m,
                      model::Vector3f( static_cast<float>( *v0x ),
                                       static_cast<float>( *v0y ),
                                       static_cast<float>( *v0z ) ),
                      model::Vector3f( static_cast<float>( *v1x ),
                                       static_cast<float>( *v1y ),
                                       static_cast<float>( *v1z ) ),
                      model::Vector3f( static_cast<float>( *v2x ),
                                       static_cast<float>( *v2y ),
                                       static_cast<float>( *v2z ) ) );
                  obj->setDisplayName( name );
                  return obj;
              },
              [this]( const model::Material& m, const std::string& name, model::Primitive& p ) {
                  auto* f = dynamic_cast<GenericFields*>( contentFields() );
                  auto* t = dynamic_cast<model::Triangle*>( &p );
                  if ( !f || !t )
                      return false;
                  auto fetch = [&]( const char* key ) {
                      return f->parseField( key, []( double ) { return true; } );
                  };
                  auto v0x = fetch( "v0x" ), v0y = fetch( "v0y" ), v0z = fetch( "v0z" );
                  auto v1x = fetch( "v1x" ), v1y = fetch( "v1y" ), v1z = fetch( "v1z" );
                  auto v2x = fetch( "v2x" ), v2y = fetch( "v2y" ), v2z = fetch( "v2z" );
                  if ( !v0x || !v0y || !v0z || !v1x || !v1y || !v1z || !v2x || !v2y || !v2z )
                      return false;
                  t->setMaterial( m );
                  t->setV0World( model::Vector3f( static_cast<float>( *v0x ),
                                                  static_cast<float>( *v0y ),
                                                  static_cast<float>( *v0z ) ) );
                  t->setV1World( model::Vector3f( static_cast<float>( *v1x ),
                                                  static_cast<float>( *v1y ),
                                                  static_cast<float>( *v1z ) ) );
                  t->setV2World( model::Vector3f( static_cast<float>( *v2x ),
                                                  static_cast<float>( *v2y ),
                                                  static_cast<float>( *v2z ) ) );
                  t->setDisplayName( name );
                  return true;
              } } );

        return ds;
    }

    ObjEditFields*
    contentFields() const
    {
        if ( auto* add = dynamic_cast<ObjEditorContentAdd*>( content_.get() ) )
            return add->fields();
        if ( auto* edit = dynamic_cast<ObjEditorContentEdit*>( content_.get() ) )
            return edit->fields();
        return nullptr;
    }

  private:
    model::SceneManager&                scene_manager_;
    std::optional<size_t>               target_idx_;
    std::function<void()>               objects_changed_cb_;
    std::unique_ptr<ObjEditorContent>   content_;
    mutable std::vector<TypeDescriptor> type_descriptors_;

    hui::Button                type_btn_;
    hui::Button                add_btn_;
    hui::Button                copy_btn_;
    hui::Button                del_btn_;
    hui::Button                apply_btn_;
    std::unique_ptr<dr4::Text> label_text_;

    Type       current_type_;
    Type       creation_type_;
    bool       editing_mode_ = false;
    dr4::Vec2f size_;
};

} // namespace view
} // namespace rta
