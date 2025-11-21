#include "widget.hpp"
#include "container_widget.hpp"
#include "dr4/math/rect.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/mouse_buttons.hpp"
#include "event.hpp"
#include "plugin_manager.hpp"
#include <cassert>

namespace hui {

Widget::Widget( cum::PluginManager* pm, float x, float y, float w, float h ) : size_( w, h )
{
    pm_ = pm;

    window_ = pm->getWindow();

    texture_ = window_->CreateTexture();

    texture_->SetSize( { w, h } );

    setRelPos( x, y );
    setSize( size_ );
}

Widget::Widget( cum::PluginManager* pm, const dr4::Vec2f& pos, const dr4::Vec2f& size )
    : Widget( pm, pos.x, pos.y, size.x, size.y )
{
}

bool
Widget::isDraggable() const
{
    return is_draggable_;
}

void
Widget::setDraggable( bool state )
{
    is_draggable_ = state;
}

bool
Widget::onIdle( const Event& event )
{
    return false;
}

bool
Widget::isHovered() const
{
    return is_hovered_;
}

bool
Widget::onTextEnter( const Event& event )
{
    return false;
}

bool
Widget::onKeyPress( const Event& event )
{
    return false;
}

bool
Widget::onKeyRelease( const Event& event )
{
    return false;
}

bool
Widget::onMousePress( const Event& event )
{
    if ( event.info.mouseButton.button == dr4::MouseButtonType::LEFT && is_hovered_ )
    {
        is_pressed_ = true;

        ContainerWidget* parent_container = dynamic_cast<ContainerWidget*>( parent_ );

        if ( parent_container != nullptr )
        {
            parent_container->bringToFront( this );
        }

        if ( is_draggable_ )
        {
            is_dragging_ = true;

            dr4::Vec2f mouse_pos( event.info.mouseButton.pos.x, event.info.mouseButton.pos.y );
            drag_offset_ = mouse_pos - getAbsPos();
        }

        return true;
    }

    return false;
}

bool
Widget::onMouseRelease( const Event& event )
{
    is_pressed_ = false;

    if ( is_dragging_ && event.info.mouseButton.button == dr4::MouseButtonType::LEFT )
    {
        is_dragging_ = false;
        return true;
    }

    return false;
}

bool
Widget::onMouseMove( const Event& event )
{
    dr4::Vec2f mouse_pos( event.info.mouseMove.pos.x, event.info.mouseMove.pos.y );
    is_hovered_ = pointInside( mouse_pos );

    if ( !is_hovered_ )
    {
        is_pressed_ = false;
    }

    if ( is_dragging_ )
    {
        setRelPos( getRelPos() + event.info.mouseMove.rel );

        // setRelPos( mouse_pos - drag_offset_ - getParentAbsPos() );
        return true;
    }

    return is_hovered_;
}

bool
Widget::onMe( dr4::Vec2f rel ) const
{
    return dr4::Rect2f( pos_, size_ ).Contains( rel );
}

dr4::Vec2f
Widget::getSize() const
{
    return size_;
}

void
Widget::setSize( const dr4::Vec2f& size )
{
    size_ = size;
}

dr4::Vec2f
Widget::getRelPos() const
{
    return pos_;
}

void
Widget::setRelPos( const dr4::Vec2f& pos )
{
    pos_ = pos;
    texture_->SetPos( pos );
}

void
Widget::setRelPos( float x, float y )
{
    texture_->SetPos( dr4::Vec2f{ x, y } );

    setRelPos( dr4::Vec2f{ x, y } );
}

dr4::Vec2f
Widget::getParentAbsPos() const
{
    return ( parent_ != nullptr ) ? parent_->getAbsPos() : dr4::Vec2f( 0, 0 );
}

dr4::Vec2f
Widget::getAbsPos() const
{
    dr4::Vec2f rel_pos        = getRelPos();
    dr4::Vec2f parent_abs_pos = getParentAbsPos();

    return parent_abs_pos + rel_pos;
}

bool
Widget::pointInside( const dr4::Vec2f& point ) const
{
    dr4::Vec2f abs_pos = getAbsPos();
    dr4::Vec2f sz      = getSize();

    return ( ( point.x >= abs_pos.x && point.x <= abs_pos.x + sz.x ) &&
             ( point.y >= abs_pos.y && point.y <= abs_pos.y + sz.y ) );
}

Widget*
Widget::getParent()
{
    return parent_;
}

void
Widget::setParent( Widget* new_parent )
{
    parent_ = new_parent;
}

void
Widget::RedrawMyTexture() const
{
}

void
Widget::DrawOnParentTexture() const
{
    assert( parent_ );

    parent_->getTexture()->Draw( *texture_ );
}

void
Widget::Redraw() const
{
    texture_->Clear( { 0, 0, 0, 0 } );
    RedrawMyTexture();
    DrawOnParentTexture();
}

dr4::Texture*
Widget::getTexture()
{
    return texture_;
}

const dr4::Texture*
Widget::getTexture() const
{
    return texture_;
}

} // namespace hui
