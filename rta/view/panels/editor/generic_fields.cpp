#include "rta/view/panels/editor/generic_fields.hpp"
#include "dr4/math/vec2.hpp"
#include <algorithm>
#include <stdexcept>

namespace rta {
namespace view {

ObjEditFields::ObjEditFields( hui::WindowManager* wm,
                              hui::DialogBox*     parent,
                              const dr4::Vec2f&   size,
                              const char*         type_name )
    : wm_( wm ),
      parent_( parent ),
      size_( size ),
      color_picker_( wm,
                     dr4::Vec2f{ 20.0f, TopBarHeight + 320.0f },
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
    color_label_->SetPos(
        dr4::Vec2f{ color_picker_.getRelPos().x, color_picker_.getRelPos().y - 18.0f } );
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
}

void
ObjEditFields::prefillDefaults()
{
    if ( auto* f = findField( "name" ) )
    {
        f->input->setString( type_name_ );
    }
    color_picker_.setColor( { 118, 185, 0, 255 } );
    current_color_ = { 118, 185, 0, 255 };
    setSliderDefaults();
}

void
ObjEditFields::prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj )
{
    (void)obj;
    if ( auto* f = findField( "name" ) )
    {
        f->input->setString( info.display_name.empty() ? info.type_name : info.display_name );
    }
    dr4::Color c{ info.material.color.r,
                  info.material.color.g,
                  info.material.color.b,
                  info.material.color.a };
    color_picker_.setColor( c );
    current_color_ = c;
    slider_reflection_.setFactor( std::clamp( info.material.reflection_factor, 0.0f, 1.0f ) );
    slider_refraction_.setFactor( std::clamp( info.material.refraction_factor, 0.0f, 1.0f ) );
    slider_eta_.setFactor(
        std::clamp( ( info.material.refraction_eta - eta_min_ ) / ( eta_max_ - eta_min_ ),
                    0.0f,
                    1.0f ) );
    updateSliderLabels();
}

ObjEditFields::CommonValues
ObjEditFields::parseCommon()
{
    auto* f_name = findField( "name" );

    auto  color = color_picker_.getColor();
    float vf    = sliderReflectionValue();
    float vref  = sliderRefractionValue();
    float veta  = sliderEtaValue();
    if ( vref != 0.0f && veta <= 0.0f )
    {
        setEtaErrorHighlight( true );
        throw std::runtime_error( "eta must be > 0 when refraction enabled" );
    }
    setEtaErrorHighlight( false );

    return CommonValues{
        f_name ? std::string( f_name->input->getString().value_or( "" ) ) : std::string(),
        model::Material( model::Color( color.r, color.g, color.b, color.a ), vf, vref, veta ) };
}

float
ObjEditFields::layoutFields( const std::vector<FieldDef>& defs, float start_y )
{
    const float margin_x   = 12.0f;
    const float line_h     = 24.0f;
    const float label_w    = 120.0f;
    const float field_h    = 18.0f;
    const float field_w    = size_.x - margin_x - label_w - 20.0f;
    const float slider_gap = 12.0f;

    auto*       win  = wm_->getWindow();
    const auto* font = win->GetDefaultFont();

    float cur_y = start_y;
    for ( const auto& d : defs )
    {
        auto* t = win->CreateText();
        t->SetFont( font );
        t->SetFontSize( 15 );
        t->SetText( d.label );
        t->SetColor( { 220, 220, 220, 255 } );

        auto input =
            std::make_unique<hui::InputText>( wm_, margin_x + label_w, cur_y, field_w, field_h );
        input->setParent( parent_ );

        form_fields_.push_back(
            { d.key, std::unique_ptr<dr4::Text>( t ), std::move( input ), { margin_x, cur_y } } );
        cur_y += line_h;
    }

    slider_origin_y_ = cur_y + slider_gap;
    layoutSliders();
    return slider_origin_y_;
}

ObjEditFields::FieldRef*
ObjEditFields::findField( const std::string& key )
{
    for ( auto& f : form_fields_ )
    {
        if ( f.key == key )
        {
            return &f;
        }
    }
    return nullptr;
}

void
ObjEditFields::setField( const std::string& key, const std::string& val )
{
    if ( auto* f = findField( key ) )
    {
        f->input->setString( val );
    }
}

bool
ObjEditFields::propagateEvent( const hui::Event& event )
{
    for ( auto& f : form_fields_ )
    {
        if ( event.apply( f.input.get() ) )
        {
            return true;
        }
    }
    if ( event.apply( &color_picker_ ) )
    {
        return true;
    }
    if ( event.apply( &slider_reflection_ ) )
    {
        return true;
    }
    if ( event.apply( &slider_refraction_ ) )
    {
        return true;
    }
    if ( event.apply( &slider_eta_ ) )
    {
        return true;
    }
    return false;
}

void
ObjEditFields::redraw( dr4::Texture& target )
{
    for ( auto& f : form_fields_ )
    {
        f.label->SetPos( f.label_pos );
        f.label->DrawOn( target );
        f.input->Redraw();
    }

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

    dr4::Vec2f color_offset =
        eta_label_->GetPos() + dr4::Vec2f{ 0.0f, slider_eta_.getSize().y + 10.0f };

    dr4::Vec2f label_picker_delta = color_picker_.getRelPos() - color_label_->GetPos();

    color_label_->SetPos( color_offset );
    color_label_->DrawOn( target );

    color_picker_.setRelPos( color_offset + label_picker_delta );
    color_picker_.Redraw();
}

float
ObjEditFields::sliderReflectionValue() const
{
    return slider_reflection_.getFactor();
}

float
ObjEditFields::sliderRefractionValue() const
{
    return slider_refraction_.getFactor();
}

float
ObjEditFields::sliderEtaValue() const
{
    float t = slider_eta_.getFactor();
    return eta_min_ + t * ( eta_max_ - eta_min_ );
}

void
ObjEditFields::setSliderDefaults()
{
    slider_reflection_.setFactor( 0.5f );
    slider_refraction_.setFactor( 0.0f );
    slider_eta_.setFactor( 0.0f );
    updateSliderLabels();
}

void
ObjEditFields::updateSliderLabels()
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
ObjEditFields::setEtaErrorHighlight( bool error )
{
    dr4::Color err{ 255, 64, 64, 255 };
    dr4::Color ok{ 220, 220, 220, 255 };
    if ( eta_label_ )
    {
        eta_label_->SetColor( error ? err : ok );
    }
    if ( eta_value_ )
    {
        eta_value_->SetColor( error ? err : ok );
    }
}

void
ObjEditFields::layoutSliders()
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
        {
            label->SetPos( { start_x, cur_y } );
        }
        if ( value )
        {
            value->SetPos( { start_x + label_w + slider_w + 8.0f, cur_y } );
        }
        slider.setSize( { slider_w, slider_h } );
        slider.setRelPos( { start_x + label_w, cur_y - slider_up } );
        cur_y += row_h;
    };

    place( reflection_label_.get(), reflection_value_.get(), slider_reflection_ );
    place( refraction_label_.get(), refraction_value_.get(), slider_refraction_ );
    place( eta_label_.get(), eta_value_.get(), slider_eta_ );
    updateSliderLabels();
}

float
ObjEditFields::layoutNameField()
{
    FieldDef name_def{ "Name", "name" };
    return layoutFields( { name_def }, TopBarHeight + 42.0f );
}

std::optional<model::Vector3f>
ObjEditFields::parsePos()
{
    auto x = parseField( "x", []( double ) { return true; } );
    auto y = parseField( "y", []( double ) { return true; } );
    auto z = parseField( "z", []( double ) { return true; } );
    if ( !x || !y || !z )
    {
        return std::nullopt;
    }
    return model::Vector3f( static_cast<float>( *x ),
                            static_cast<float>( *y ),
                            static_cast<float>( *z ) );
}

} // namespace view
} // namespace rta
