#include "scrollable_list_widget.hpp"
#include "window_manager.hpp"
#include <algorithm>
#include <cassert>

namespace hui {

ScrollableListWidget::ScrollableListWidget( hui::WindowManager* wm,
                                            const dr4::Vec2f&   pos,
                                            const dr4::Vec2f&   content_size,
                                            float               scrollbar_width )
    : ContainerWidget( wm, pos, { content_size.x + scrollbar_width, content_size.y } ),
      scroll_bar_( wm, 0.0f, 0.0f, scrollbar_width, content_size.y )
{
    scroll_bar_.setParent( this );
}

void
ScrollableListWidget::addItem( std::unique_ptr<Widget> item )
{
    total_items_y_ += item->getSize().y;
    item->setParent( this );
    items_.push_back( std::move( item ) );
    rebuildLayout();
}

void
ScrollableListWidget::rebuildLayout()
{
    float y = 0.0f;
    for ( auto& item : items_ )
    {
        item->setRelPos( { scroll_bar_.getSize().x, y } );
        y += item->getSize().y;
    }

    float total_content_height = y;
    float visible_height       = getSize().y;
}

void
ScrollableListWidget::updateContentPosition()
{
    rebuildLayout();

    float max_offset = std::max( 0.0f, total_items_y_ - getSize().y );
    float offset     = scroll_bar_.getScrollFactor() * max_offset;

    for ( size_t i = 0; i < items_.size(); ++i )
    {
        items_[i]->setRelPos( { scroll_bar_.getSize().x, i * items_[i]->getSize().y - offset } );
    }
}

bool
ScrollableListWidget::propagateEventToChildren( const Event& event )
{
    if ( scroll_bar_.isScrolled() )
    {
        updateContentPosition();
    }

    if ( event.apply( &scroll_bar_ ) )
    {
        return true;
    }

    for ( auto& item : items_ )
    {
        if ( event.apply( item.get() ) )
        {
            return true;
        }
    }

    return false;
}

void
ScrollableListWidget::RedrawMyTexture() const
{
    texture_->Clear( { 0, 0, 0, 0 } );

    // std::cerr << "items_.size() = " << items_.size() << std::endl;

    for ( const auto& item : items_ )
    {
        item->Redraw();
    }

    scroll_bar_.Redraw();
}

} // namespace hui
