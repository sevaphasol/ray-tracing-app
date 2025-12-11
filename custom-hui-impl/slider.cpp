#include "slider.hpp"
#include "window_manager.hpp"
#include <algorithm>

namespace hui {

Slider::Slider( hui::WindowManager*  wm,
                const dr4::Vec2f&    pos,
                const dr4::Vec2f&    size,
                const Slider::Theme& theme )
    : Widget( wm, pos, size ), theme_( theme )
{
    track_.reset( wm_->getWindow()->CreateRectangle() );
    fill_.reset( wm_->getWindow()->CreateRectangle() );
    handle_.reset( wm_->getWindow()->CreateRectangle() );

    track_->SetFillColor( theme_.track_color );
    track_->SetBorderThickness( 0.0f );

    fill_->SetFillColor( theme_.fill_color );
    fill_->SetBorderThickness( 0.0f );

    handle_->SetFillColor( theme_.handle_color );
    handle_->SetBorderThickness( 0.0f );

    setSize( size );
    setRelPos( pos );
}

float
Slider::getFactor() const
{
    return factor_;
}

void
Slider::setFactor( float t )
{
    factor_ = clamp01( t );
    Redraw();
}

void
Slider::setOnChange( ChangeCallback cb )
{
    on_change_ = std::move( cb );
}

void
Slider::setSize( const dr4::Vec2f& size )
{
    Widget::setSize( size );
    track_->SetSize( { size_.x, theme_.track_height } );
    fill_->SetSize( { size_.x * factor_, theme_.track_height } );
    handle_->SetSize( { theme_.handle_width, size_.y } );
    setRelPos( pos_ );
}

void
Slider::setRelPos( const dr4::Vec2f& pos )
{
    Widget::setRelPos( pos );
    const float track_y = ( size_.y - theme_.track_height ) * 0.5f;
    track_->SetPos( { 0.0f, track_y } );
    fill_->SetPos( { 0.0f, track_y } );
    handle_->SetPos( { factor_ * ( size_.x - theme_.handle_width ), 0.0f } );
}

bool
Slider::onMousePress( const Event& event )
{
    if ( event.info.mouseButton.button != dr4::MouseButtonType::LEFT )
        return false;

    if ( !pointInside( event.info.mouseButton.pos ) )
        return false;

    dragging_ = true;
    is_hovered_ = true;
    updateFromPos( event.info.mouseButton.pos.x );
    return true;
}

bool
Slider::onMouseMove( const Event& event )
{
    if ( dragging_ )
    {
        updateFromPos( event.info.mouseMove.pos.x );
        return true;
    }

    if ( onMe( event.info.mouseMove.pos ) )
    {
        handle_->SetFillColor( theme_.handle_hover_color );
        return true;
    }
    handle_->SetFillColor( theme_.handle_color );
    return false;
}

bool
Slider::onMouseRelease( const Event& event )
{
    if ( event.info.mouseButton.button != dr4::MouseButtonType::LEFT )
        return false;

    bool was_dragging = dragging_;
    dragging_         = false;
    return was_dragging || pointInside( event.info.mouseButton.pos );
}

void
Slider::RedrawMyTexture() const
{
    fill_->SetSize( { size_.x * factor_, theme_.track_height } );
    handle_->SetPos( { factor_ * ( size_.x - theme_.handle_width ), 0.0f } );

    texture_->Draw( *track_ );
    texture_->Draw( *fill_ );
    texture_->Draw( *handle_ );
}

float
Slider::clamp01( float v ) const
{
    return std::clamp( v, 0.0f, 1.0f );
}

void
Slider::updateFromPos( float mouse_x )
{
    float local_x = mouse_x - getAbsPos().x;
    float t       = clamp01( local_x / size_.x );
    if ( t != factor_ )
    {
        factor_ = t;
        if ( on_change_ )
            on_change_( factor_ );
        Redraw();
    }
}

} // namespace hui
