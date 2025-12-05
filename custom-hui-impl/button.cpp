#include "button.hpp"
#include "dr4/math/color.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include "plugin_manager.hpp"
#include "text_expand.hpp"
#include <cstdio>

namespace hui {

Button::Button( cum::Manager*      pm,
                dr4::Window*       win,
                const dr4::Vec2f&  pos,
                const dr4::Vec2f&  size,
                const dr4::Color&  default_color,
                const dr4::Color&  hovered_color,
                const dr4::Color&  pressed_color,
                const std::string& title,
                const dr4::Color&  font_color,
                size_t             font_size )
    : Widget( pm, win, pos, size ),
      default_color_( default_color ),
      hovered_color_( hovered_color ),
      pressed_color_( pressed_color )
{
    background_.reset( win->CreateRectangle() );
    label_.reset( win->CreateText() );

    background_->SetSize( size );

    background_->SetFillColor( default_color );
    label_->SetFont( win->GetDefaultFont() );
    label_->SetText( title );
    label_->SetFontSize( font_size );
    label_->SetColor( font_color );

    moveInCenterOfRect( *label_, size, pos );
}

void
Button::setRelPos( const dr4::Vec2f& pos )
{
    setRelPos( pos );
    moveInCenterOfRect( *label_, size_, pos );
}

void
Button::setSize( const dr4::Vec2f& size )
{
    size_ = size;
    background_->SetSize( size );
    moveInCenterOfRect( *label_, size, pos_ );
}

void
Button::setLabelText( const std::string& text )
{
    label_->SetText( text );
}

void
Button::setLabelFont( const dr4::Font* font, size_t size )
{
    label_->SetFont( font );
    label_->SetFontSize( size );
}

void
Button::setBackgroundColor( const dr4::Color& color )
{
    background_->SetFillColor( color );
}

bool
Button::isPressed() const
{
    return is_pressed_;
}

bool
Button::isPressedJustNow() const
{
    return is_pressed_just_now_;
}

bool
Button::onIdle( const Event& event )
{
    is_pressed_just_now_ = false;

    updateVisuals();
    return false;
}

bool
Button::onMousePress( const Event& event )
{
    if ( Widget::onMousePress( event ) )
    {
        is_pressed_just_now_ = true;
        return true;
    }

    return false;
}

void
Button::updateVisuals()
{
    if ( is_pressed_ )
    {
        background_->SetFillColor( pressed_color_ );
    } else if ( is_hovered_ )
    {
        background_->SetFillColor( hovered_color_ );
    } else
    {
        background_->SetFillColor( default_color_ );
    }
}

void
Button::RedrawMyTexture() const
{
    texture_->Draw( *background_ );
    texture_->Draw( *label_ );
}

} // namespace hui
