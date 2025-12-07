#include "scrollable_widget.hpp"
#include "scrollbar.hpp"
#include "window_manager.hpp"
#include <algorithm>

namespace hui {

ScrollableWidget::ScrollableWidget( hui::WindowManager* wm,
                                    const dr4::Vec2f&   pos,
                                    const dr4::Vec2f&   content_size,
                                    float               scrollbar_width )
    : ContainerWidget( wm, pos, { content_size.x + scrollbar_width, content_size.y } ),
      scroll_bar_( wm, 0.0f, 0.0f, scrollbar_width, content_size.y )
{
}

void
ScrollableWidget::setContent( std::unique_ptr<Widget> content )
{
    content_ = std::move( content );
    if ( content_ )
    {
        content_->setParent( this );
        content_->setRelPos( { 0.0f, 0.0f } );
    }
    needs_redraw_ = true;
}

void
ScrollableWidget::updateContentPosition()
{
    if ( !content_ )
    {
        return;
    }

    float content_height = content_->getSize().y;
    float view_height    = getSize().y;
    float max_offset     = std::max( 0.0f, content_height - view_height );
    float offset         = static_cast<float>( scroll_bar_.getScrollFactor() * max_offset );

    content_->setRelPos( { 0.0f, -offset } );
    needs_redraw_ = true;
}

bool
ScrollableWidget::propagateEventToChildren( const Event& event )
{
    if ( scroll_bar_.isScrolled() )
    {
        updateContentPosition();
    }

    if ( event.apply( &scroll_bar_ ) )
    {
        return true;
    }

    if ( content_ )
    {
        if ( event.apply( content_.get() ) )
        {
            return true;
        }
    }

    return false;
}

void
ScrollableWidget::RedrawMyTexture() const
{
    if ( !needs_redraw_ )
    {
        return;
    }

    texture_->Clear( { 0, 0, 0, 0 } );

    if ( content_ )
    {
        content_->Redraw();
    }

    scroll_bar_.Redraw();

    needs_redraw_ = false;
}

} // namespace hui
