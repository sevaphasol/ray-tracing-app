#pragma once

#include "custom-hui/dialog_box.hpp"
#include "custom-hui/input_text.hpp"
#include "custom-hui/slider.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include "rta/model/primitives/material.hpp"
#include "rta/model/rendering/scene_manager.hpp"
#include "rta/view/utils/rgb_picker.hpp"
#include <functional>
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace rta {
namespace view {

struct FieldDef
{
    std::string label;
    std::string key;
};

class ObjEditFields {
    static constexpr int TopBarHeight = 25.0f;

  public:
    struct CommonValues
    {
        std::string     name;
        model::Material material;
    };

    ObjEditFields( hui::WindowManager* wm,
                   hui::DialogBox*     parent,
                   const dr4::Vec2f&   size,
                   const char*         type_name );
    virtual ~ObjEditFields() = default;

    virtual void
    prefillDefaults();
    virtual void
    prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj );

    virtual std::unique_ptr<model::Primitive>
    buildNew( const CommonValues& ) = 0;
    virtual bool
    applyToExisting( const CommonValues&, model::Primitive& ) = 0;

    bool
    propagateEvent( const hui::Event& event );
    void
    redraw( dr4::Texture& target );

    CommonValues
    parseCommon();
    float
    layoutNameField();
    float
    layoutFields( const std::vector<FieldDef>& defs, float start_y );
    void
    setField( const std::string& key, const std::string& val );
    std::optional<model::Vector3f>
    parsePos();

    template<typename Validator>
    std::optional<double>
    parseField( const std::string& key, Validator v )
    {
        return parse( findField( key ), v );
    }

    hui::DialogBox*
    parent() const
    {
        return parent_;
    }

  protected:
    struct FieldRef
    {
        std::string                     key;
        std::unique_ptr<dr4::Text>      label;
        std::unique_ptr<hui::InputText> input;
        dr4::Vec2f                      label_pos;
    };

    FieldRef*
    findField( const std::string& key );

    template<typename Validator>
    static std::optional<double>
    parse( FieldRef* f, Validator v )
    {
        if ( !f )
        {
            return std::nullopt;
        }
        auto val = f->input->getDouble();
        if ( !val.has_value() || !v( *val ) )
        {
            f->input->setColor( { 255, 64, 64, 255 } );
            return std::nullopt;
        }
        f->input->setColor( { 255, 255, 255, 255 } );
        return *val;
    }

    static std::string
    fmt2( double v )
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision( 2 ) << v;
        return ss.str();
    }

    float
    sliderReflectionValue() const;
    float
    sliderRefractionValue() const;
    float
    sliderEtaValue() const;
    void
    setSliderDefaults();
    void
    updateSliderLabels();
    void
    setEtaErrorHighlight( bool error );
    void
    layoutSliders();

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

} // namespace view
} // namespace rta
