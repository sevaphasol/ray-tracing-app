#include "scrollbar.hpp"
#include "dr4/math/color.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/mouse_buttons.hpp"
#include "plugin_manager.hpp"

#include <algorithm>
#include <cassert>

namespace hui {

namespace detail {

struct ScrollBar
{
    static inline const dr4::Vec2f Size = { 15.0f, 110.0f };

    struct ArrowField
    {
        static constexpr float SizeCoef = 0.1;

        static inline const dr4::Vec2f Size = { ScrollBar::Size.x, ScrollBar::Size.y* SizeCoef };

        struct Color
        {
            static const inline dr4::Color Default = { 96 + 32, 96 + 32, 96 + 32, 255 };
            static const inline dr4::Color Hover   = { 64 + 32, 64 + 32, 64 + 32, 255 };
            static const inline dr4::Color Pressed = { 32 + 32, 32 + 32, 32 + 32, 255 };
        };

        struct Triangle
        {
            struct Color
            {
                static const inline dr4::Color Default = { 96, 96, 96, 255 };
                static const inline dr4::Color Hover   = { 64, 64, 64, 255 };
                static const inline dr4::Color Pressed = { 32, 32, 32, 255 };
            };

            struct Up
            {
                static inline const dr4::Vec2f Pos = { 0.0f, 0.0f };

                static inline const dr4::Vec2f Triangle[] = {
                    dr4::Vec2f( ScrollBar::ArrowField::Size.x / 2,
                                ScrollBar::ArrowField::Size.y / 3 ),
                    dr4::Vec2f( ScrollBar::ArrowField::Size.x / 3,
                                2 * ScrollBar::ArrowField::Size.y / 3 ),
                    dr4::Vec2f( 2 * ScrollBar::ArrowField::Size.x / 3,
                                2 * ScrollBar::ArrowField::Size.y / 3 ) };
            };

            struct Down
            {
                static inline const dr4::Vec2f Pos = { 0.0f, ScrollBar::Size.y * ( 1 - SizeCoef ) };

                static inline const dr4::Vec2f Triangle[] = {
                    dr4::Vec2f( ScrollBar::ArrowField::Size.x / 2,
                                2 * ScrollBar::ArrowField::Size.y / 3 ),
                    dr4::Vec2f( ScrollBar::ArrowField::Size.x / 3,
                                ScrollBar::ArrowField::Size.y / 3 ),
                    dr4::Vec2f( 2 * ScrollBar::ArrowField::Size.x / 3,
                                ScrollBar::ArrowField::Size.y / 3 ) };
            };
        };
    };

    struct Thumb
    {
        static constexpr float SizeCoef = 0.3;

        struct Color
        {
            static const inline dr4::Color Default = { 96 + 32, 96 + 32, 96 + 32, 100 };
            static const inline dr4::Color Hover   = { 64 + 32, 64 + 32, 64 + 32, 200 };
            static const inline dr4::Color Pressed = { 32 + 32, 32 + 32, 32 + 32, 255 };
        };

        static inline const dr4::Vec2f Size = { ScrollBar::Size.x, ScrollBar::Size.y* SizeCoef };

        static inline const dr4::Vec2f StartPos = { 0.0f,
                                                    ScrollBar::Size.y * ( 1 - SizeCoef ) -
                                                        ArrowField::Size.y };
    };
};

} // namespace detail

Thumb::Thumb( cum::PluginManager* pm,
              ScrollBar*          owner,
              const dr4::Vec2f&   pos,
              const dr4::Vec2f&   size )
    : hui::Widget( pm, pos, size ), owner_( owner )
{
    parent_ = owner;

    rect_.reset( pm->getWindow()->CreateRectangle() );

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
    if ( is_pressed_ )
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

Arrow::Arrow( cum::PluginManager* pm,
              ScrollBar*          owner,
              const dr4::Vec2f&   pos,
              const dr4::Vec2f&   size,
              bool                is_up )
    : hui::Widget( pm, pos, size ), is_up_( is_up ), owner_( owner )
{
    parent_ = owner;

    rect_.reset( pm->getWindow()->CreateRectangle() );

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

ScrollBar::ScrollBar( cum::PluginManager* pm, float x, float y )
    : ScrollBar( pm, dr4::Vec2f( x, y ) )
{
}

ScrollBar::ScrollBar( cum::PluginManager* pm, const dr4::Vec2f& pos )
    : hui::ContainerWidget( pm, pos, detail::ScrollBar::Size ),
      thumb_( pm,
              this,
              detail::ScrollBar::Thumb::StartPos,
              dr4::Vec2f( detail::ScrollBar::Size.x,
                          detail::ScrollBar::Size.y * detail::ScrollBar::Thumb::SizeCoef ) ),
      up_arrow_( pm,
                 this,
                 dr4::Vec2f( 0.0f, 0.0f ),
                 dr4::Vec2f( detail::ScrollBar::Size.x,
                             detail::ScrollBar::Size.y * detail::ScrollBar::ArrowField::SizeCoef ),
                 true ),
      down_arrow_(
          pm,
          this,
          dr4::Vec2f( 0.0f,
                      detail::ScrollBar::Size.y * ( 1 - detail::ScrollBar::ArrowField::SizeCoef ) ),
          dr4::Vec2f( detail::ScrollBar::Size.x,
                      detail::ScrollBar::Size.y * detail::ScrollBar::ArrowField::SizeCoef ),
          false )
{
    border_.reset( pm->getWindow()->CreateRectangle() );

    border_->SetSize( getSize() );
    border_->SetFillColor( { 64, 64, 64, 128 } );
    border_->SetBorderColor( { 32, 32, 32, 255 } );
    border_->SetBorderThickness( -4.0f );
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

    scroll_factor_ = std::clamp( scroll_factor_ - norm_delta, 0.0, 1.0 );

    is_scrolled_ = true;

    updateThumbPosition();
}

void
ScrollBar::onArrowClick( bool is_up )
{
    scroll_factor_ = std::clamp( scroll_factor_ + ( is_up ? 0.1 : -0.1 ), 0.0, 1.0 );

    is_scrolled_ = true;

    updateThumbPosition();
}

void
ScrollBar::updateThumbPosition()
{
    float thumb_y =
        up_arrow_.getSize().y +
        ( 1 - scroll_factor_ ) * ( size_.y - thumb_.getSize().y - 2 * up_arrow_.getSize().y );

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
