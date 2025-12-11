#include "button.hpp"
#include "container_widget.hpp"
#include "dr4/math/color.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include "window_manager.hpp"

namespace hui {

namespace {
void
centerLabel( dr4::Text& text, const dr4::Vec2f& rect_size, const dr4::Vec2f& /*rect_pos*/ )
{
    const auto& bounds = text.GetBounds();
    text.SetPos( rect_size.x * 0.5f - bounds.x * 0.5f, rect_size.y * 0.5f - bounds.y * 0.5f );
}
} // namespace

Button::Button( hui::WindowManager*  wm,
                const dr4::Vec2f&    pos,
                const dr4::Vec2f&    size,
                const std::string&   title,
                const Button::Theme& theme )
    : Widget( wm, pos, size ), theme_( theme )
{
    background_.reset( wm->getWindow()->CreateRectangle() );
    label_.reset( wm->getWindow()->CreateText() );

    background_->SetSize( size );
    background_->SetFillColor( theme.default_color );

    label_->SetFont( wm->getWindow()->GetDefaultFont() );
    label_->SetText( title );
    label_->SetFontSize( theme.font_size );
    label_->SetColor( theme.font_color );

    centerLabel( *label_, size, pos );
}

const Button::Theme&
Button::getTheme() const
{
    return theme_;
}

void
Button::setTheme( const Button::Theme& theme )
{
    theme_ = theme;
}

void
Button::setRelPos( const dr4::Vec2f& pos )
{
    Widget::setRelPos( pos );
    centerLabel( *label_, size_, pos_ );
}

void
Button::setSize( const dr4::Vec2f& size )
{
    Widget::setSize( size );
    background_->SetSize( size );
    centerLabel( *label_, size_, pos_ );
}

void
Button::setLabelText( const std::string& text )
{
    label_->SetText( text );
    centerLabel( *label_, size_, pos_ );
}

void
Button::setFont( const dr4::Font* font )
{
    if ( font )
    {
        label_->SetFont( font );
        centerLabel( *label_, size_, pos_ );
    }
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

void
Button::setOnClick( ClickCb callback )
{
    on_click_ = std::move( callback );
}

void
Button::setOnHoldPress( HoldPressCb callback )
{
    on_hold_press_ = std::move( callback );
}

bool
Button::onMousePress( const Event& event )
{
    if ( event.info.mouseButton.button == dr4::MouseButtonType::LEFT && is_hovered_ )
    {
        is_pressed_ = true;

        ContainerWidget* parent_container = dynamic_cast<ContainerWidget*>( parent_ );
        if ( parent_container != nullptr )
        {
            parent_container->bringToFront( this );
        }

        background_->SetFillColor( theme_.pressed_color );

        return true;
    }
    return false;
}

bool
Button::onMouseMove( const Event& event )
{
    if ( !Widget::onMouseMove( event ) || !is_hovered_ )
    {
        background_->SetFillColor( theme_.default_color );
        return false;
    }

    background_->SetFillColor( theme_.hovered_color );
    return true;
}

bool
Button::onMouseRelease( const Event& event )
{
    if ( event.info.mouseButton.button == dr4::MouseButtonType::LEFT )
    {
        bool was_pressed = is_pressed_;
        is_pressed_      = false;

        background_->SetFillColor( is_hovered_ ? theme_.hovered_color : theme_.default_color );

        if ( was_pressed && is_hovered_ )
        {
            if ( on_click_ )
            {
                on_click_();
            }
        }

        return was_pressed;
    }
    return false;
}

bool
Button::onIdle( const Event& event )
{
    if ( is_pressed_ )
    {
        if ( on_hold_press_ )
        {
            on_hold_press_();
        }
    }

    return false;
}

void
Button::RedrawMyTexture() const
{
    texture_->Draw( *background_ );
    texture_->Draw( *label_ );
}

} // namespace hui
