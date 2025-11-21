#pragma once

#include "custom-hui-impl/widget.hpp"
#include "dr4/texture.hpp"
#include "zemax/config.hpp"
#include "zemax/model/primitives/primitive.hpp"

#include <cassert>
#include <iomanip>

namespace zemax {
namespace view {

class ObjInfoPanel : public hui::Widget {
  public:
    ObjInfoPanel( cum::PluginManager* pm, const dr4::Font* font, float w, float h )
        : hui::Widget( pm, 0.0f, 0.0f, w, h )
    {
        text_.reset( pm->getWindow()->CreateText() );
        rect_.reset( pm->getWindow()->CreateRectangle() );

        text_->SetFont( font );
        text_->SetColor( Config::Scene::ObjInfoPanel::FontColor );
        text_->SetFontSize( Config::Scene::ObjInfoPanel::FontSize );

        rect_->SetSize( { w, h } );
        rect_->SetFillColor( Config::Scene::ObjInfoPanel::FillColor );
        rect_->SetBorderColor( Config::Scene::ObjInfoPanel::OutlineColor );
        rect_->SetBorderThickness( -Config::Scene::ObjInfoPanel::OutlineThickness );
    }

    ObjInfoPanel( cum::PluginManager* pm, const dr4::Font* font, const dr4::Vec2f& size )
        : ObjInfoPanel( pm, font, size.x, size.y )
    {
    }

    void
    update( model::Primitive* obj )
    {
        setType( obj );
        setCoords( obj );
        setColor( obj );
        setReflection( obj );

        text_->SetText( type_ + '\n' + coords_ + '\n' + color_ + '\n' + reflection_ );

        // text_.setString( type_ + '\n' + coords_ + '\n' + color_ + '\n' + reflection_ );
        // text_.setString( type_ );
    }

    void
    setFont( const dr4::Font* font )
    {
        text_->SetFont( font );
    }

    bool
    isVisible() const
    {
        return is_visible_;
    }

    void
    setVisible( bool state )
    {
        is_visible_ = state;
    }

  private:
    void
    setType( model::Primitive* obj )
    {
        type_.clear();
        type_ += type_template_;
        type_ += obj->getName();
    }

    void
    setReflection( model::Primitive* obj )
    {
        reflection_.clear();

        std::ostringstream oss;
        oss << reflection_template_ << std::fixed << std::setprecision( 2 )
            << obj->getMaterial().reflection_factor;

        reflection_ = oss.str();
    }

    void
    setCoords( model::Primitive* obj )
    {
        coords_.clear();
        auto obj_origin = obj->getOrigin();

        std::ostringstream oss;
        oss << coords_template_ << "( " << std::fixed << std::setprecision( 1 ) << obj_origin.x
            << ", " << obj_origin.y << ", " << obj_origin.z << " )";

        coords_ = oss.str();
    }

    void
    setColor( model::Primitive* obj )
    {
        color_.clear();
        auto obj_color = obj->getMaterial().color;

        color_ += color_template_;
        color_ += "( ";
        color_ += std::to_string( obj_color.r );
        color_ += ", ";
        color_ += std::to_string( obj_color.g );
        color_ += ", ";
        color_ += std::to_string( obj_color.b );
        color_ += " )";
    }

    void
    RedrawMyTexture() const
    {
        texture_->Clear( { 0, 0, 0, 0 } );

        if ( isVisible() )
        {
            texture_->Draw( *rect_ );
            texture_->Draw( *text_ );
        }
    }

  private:
    const char* type_template_       = "Type:       ";
    const char* coords_template_     = "Coords:     ";
    const char* color_template_      = "Color:      ";
    const char* reflection_template_ = "Reflection: ";

    std::string type_;
    std::string coords_;
    std::string color_;
    std::string reflection_;

    std::unique_ptr<dr4::Rectangle> rect_;
    std::unique_ptr<dr4::Text>      text_;

    bool is_visible_ = false;
};

} // namespace view
} // namespace zemax
