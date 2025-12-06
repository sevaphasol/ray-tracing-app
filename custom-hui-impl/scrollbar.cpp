#include "scrollbar.hpp"
#include "dr4/math/color.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/mouse_buttons.hpp"
#include "dr4/window.hpp"
#include "plugin_manager.hpp"

#include <algorithm>
#include <cassert>

namespace hui {

Thumb::Thumb( cum::Manager*     pm,
              dr4::Window*      win,
              ScrollBar*        owner,
              const dr4::Vec2f& pos,
              const dr4::Vec2f& size )
    : hui::Widget( pm, win, pos, size ), owner_( owner )
{
    parent_ = owner;

    rect_.reset( win->CreateRectangle() );

    setDraggable( true );

    rect_->SetSize( size );
    rect_->SetFillColor( detail::ScrollBar::Thumb::Color::Default );
}

bool
Thumb::onIdle( const Event& event )
{
    updateVisuals();

    return false;
}

bool
Thumb::onMousePress( const Event& event )
{
    if ( is_hovered_ && event.info.mouseButton.button == dr4::MouseButtonType::LEFT )
    {
        is_pressed_  = true;
        is_dragging_ = true;
        dr4::Vec2f mouse_pos( event.info.mouseButton.pos.x, event.info.mouseButton.pos.y );
        drag_offset_ = mouse_pos - getAbsPos();

        return true;
    }

    return false;
}

bool
Thumb::onMouseMove( const Event& event )
{
    dr4::Vec2f mouse_pos( event.info.mouseMove.pos.x, event.info.mouseMove.pos.y );
    is_hovered_ = pointInside( mouse_pos );

    if ( !is_hovered_ )
    {
        is_pressed_ = false;
    }

    if ( is_dragging_ )
    {
        auto new_pos = owner_->clampThumbPosition( mouse_pos - getParentAbsPos() - drag_offset_ );
        owner_->onThumbMove( new_pos.y - getRelPos().y );
    }

    return is_hovered_;
}

bool
Thumb::onMouseRelease( const Event& event )
{
    return Widget::onMouseRelease( event );
}

void
Thumb::updateVisuals()
{
    if ( is_pressed_ || is_dragging_ )
    {
        rect_->SetFillColor( detail::ScrollBar::Thumb::Color::Pressed );
    } else if ( is_hovered_ )
    {
        rect_->SetFillColor( detail::ScrollBar::Thumb::Color::Hover );
    } else
    {
        rect_->SetFillColor( detail::ScrollBar::Thumb::Color::Default );
    }
}

void
Thumb::RedrawMyTexture() const
{
    // core::Transform widget_transform = transform.combine( getTransform() );

    texture_->Draw( *rect_ );

    // window.draw( rect_, widet_transform );
}

Arrow::Arrow( cum::Manager*     pm,
              dr4::Window*      win,
              ScrollBar*        owner,
              const dr4::Vec2f& pos,
              const dr4::Vec2f& size,
              bool              is_up )
    : hui::Widget( pm, win, pos, size ), is_up_( is_up ), owner_( owner )
{
    parent_ = owner;

    rect_.reset( win->CreateRectangle() );

    rect_->SetSize( size );
    rect_->SetFillColor( detail::ScrollBar::ArrowField::Color::Default );

    setUpTriangle();
}

bool
Arrow::onIdle( const Event& event )
{
    updateVisuals();

    return false;
}

void
Arrow::setUpTriangle()
{
    updateTriangleColor( detail::ScrollBar::ArrowField::Triangle::Color::Default );
    updateTrianglePosition();
}

bool
Arrow::onMousePress( const Event& event )
{
    if ( is_hovered_ && event.info.mouseButton.button == dr4::MouseButtonType::LEFT )
    {
        is_pressed_ = true;

        owner_->onArrowClick( is_up_ );
        return true;
    }

    return false;
}

bool
Arrow::onMouseMove( const Event& event )
{
    dr4::Vec2f mouse_pos( event.info.mouseMove.pos.x, event.info.mouseMove.pos.y );
    is_hovered_ = pointInside( mouse_pos );

    if ( !is_hovered_ )
    {
        is_pressed_ = false;
    }

    if ( is_dragging_ )
    {
        setRelPos( mouse_pos - drag_offset_ - getParentAbsPos() );
        return true;
    }

    return is_hovered_;

    return Widget::onMouseMove( event );
}

bool
Arrow::onMouseRelease( const Event& event )
{
    return Widget::onMouseRelease( event );
}

void
Arrow::updateTriangleColor( const dr4::Color& color )
{
    // for ( auto& vertex : triangle_ )
    // {
    // vertex.color = color;
    // }
}

void
Arrow::updateTrianglePosition()
{
    // if ( is_up_ )
    // {
    // // // triangle_[0].position = dr4::Vec2f( size_.x / 2, size_.y / 3 );
    // // // triangle_[1].position = dr4::Vec2f( size_.x / 3, 2 * size_.y / 3 );
    // // // triangle_[2].position = dr4::Vec2f( 2 * size_.x / 3, 2 * size_.y / 3 );
    // // } else
    // {
    // // // triangle_[0].position = dr4::Vec2f( size_.x / 2, 2 * size_.y / 3 );
    // // triangle_[1].position = dr4::Vec2f( size_.x / 3, size_.y / 3 );
    // // triangle_[2].position = dr4::Vec2f( 2 * size_.x / 3, size_.y / 3 );
    // }
}

void
Arrow::updateVisuals()
{
    if ( is_pressed_ )
    {
        rect_->SetFillColor( detail::ScrollBar::ArrowField::Color::Pressed );
        updateTriangleColor( detail::ScrollBar::ArrowField::Triangle::Color::Pressed );
    } else if ( is_hovered_ )
    {
        rect_->SetFillColor( detail::ScrollBar::ArrowField::Color::Hover );
        updateTriangleColor( detail::ScrollBar::ArrowField::Triangle::Color::Hover );
    } else
    {
        rect_->SetFillColor( detail::ScrollBar::ArrowField::Color::Default );
        updateTriangleColor( detail::ScrollBar::ArrowField::Triangle::Color::Default );
    }
}

void
Arrow::RedrawMyTexture() const
{
    texture_->Draw( *rect_ );
}

// void
// Arrow::draw( dr4::Window& window, dr4::Transform transform ) const
// {
// core::Transform widget_transform = transform.combine( getTransform() );
//
// window.draw( rect_, widget_transform );
// window.draw( triangle_, 3, dr4::PrimitiveType::Triangles, widget_transform );
// }

ScrollBar::ScrollBar( cum::Manager* pm, dr4::Window* win, float x, float y, float w, float h )
    : ScrollBar( pm, win, dr4::Vec2f( x, y ), dr4::Vec2f( w, h ) )
{
}

ScrollBar::ScrollBar( cum::Manager* pm, dr4::Window* win, const dr4::Vec2f& pos, dr4::Vec2f size )
    : hui::ContainerWidget( pm, win, pos, size ),
      thumb_( pm,
              win,
              this,
              dr4::Vec2f( 0.0f, size.y * detail::ScrollBar::ArrowField::SizeCoef ),
              dr4::Vec2f( size.x, size.y * detail::ScrollBar::Thumb::SizeCoef ) ),
      up_arrow_( pm,
                 win,
                 this,
                 dr4::Vec2f( 0.0f, 0.0f ),
                 dr4::Vec2f( size.x, size.y * detail::ScrollBar::ArrowField::SizeCoef ),
                 true ),
      down_arrow_( pm,
                   win,
                   this,
                   dr4::Vec2f( 0.0f, size.y * ( 1 - detail::ScrollBar::ArrowField::SizeCoef ) ),
                   dr4::Vec2f( size.x, size.y * detail::ScrollBar::ArrowField::SizeCoef ),
                   false )
{
    border_.reset( win->CreateRectangle() );

    border_->SetSize( getSize() );
    border_->SetFillColor( { 48, 48, 48, 223 } );
    border_->SetBorderColor( { 32, 32, 32, 255 } );
    // border_->SetBorderThickness( -4.0f );
}

void
ScrollBar::bringToFront( Widget* child )
{
}

bool
ScrollBar::propagateEventToChildren( const Event& event )
{
    if ( event.apply( &thumb_ ) )
    {
        return true;
    }

    if ( event.apply( &up_arrow_ ) )
    {
        return true;
    }

    if ( event.apply( &down_arrow_ ) )
    {
        return true;
    }

    return false;
}

double
ScrollBar::getScrollFactor() const
{
    return scroll_factor_;
}

bool
ScrollBar::isScrolled()
{
    if ( is_scrolled_ )
    {
        is_scrolled_ = false;
        return true;
    }

    return false;
}

dr4::Vec2f
ScrollBar::clampThumbPosition( const dr4::Vec2f& vector )
{
    return { std::clamp( vector.x, 0.0f, size_.x - thumb_.getSize().x ),
             std::clamp( vector.y,
                         up_arrow_.getSize().y,
                         size_.y - thumb_.getSize().y - up_arrow_.getSize().y ) };
}

void
ScrollBar::onThumbMove( float vertical_delta )
{
    float norm_delta =
        vertical_delta / ( size_.y - thumb_.getSize().y - 2 * up_arrow_.getSize().y );

    scroll_factor_ = std::clamp( scroll_factor_ + norm_delta, 0.0, 1.0 );

    is_scrolled_ = true;

    updateThumbPosition();
}

void
ScrollBar::onArrowClick( bool is_up )
{
    scroll_factor_ = std::clamp( scroll_factor_ + ( is_up ? -0.1 : 0.1 ), 0.0, 1.0 );

    is_scrolled_ = true;

    updateThumbPosition();
}

void
ScrollBar::updateThumbPosition()
{
    float thumb_y = up_arrow_.getSize().y +
                    scroll_factor_ * ( size_.y - thumb_.getSize().y - 2 * up_arrow_.getSize().y );

    thumb_.setRelPos( dr4::Vec2f( 0.0, thumb_y ) );
}

void
ScrollBar::RedrawMyTexture() const
{
    // g::Transform widget_transform = transform.combine( getTransform() );

    texture_->Draw( *border_ );

    thumb_.Redraw();
    up_arrow_.Redraw();
    down_arrow_.Redraw();

    // texture_->Draw(thumb_);

    // window.draw( border_, widget_transform );
    // window.draw( thumb_, widget_transform );
    // window.draw( up_arrow_, widget_transform );
    // window.draw( down_arrow_, widget_transform );
}

} // namespace hui
