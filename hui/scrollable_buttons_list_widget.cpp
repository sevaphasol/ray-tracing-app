#include "scrollable_buttons_list_widget.hpp"
#include "buttons_list.hpp"

namespace hui {

ScrollableButtonsListWidget::ScrollableButtonsListWidget( WindowManager*    wm,
                                                          const dr4::Vec2f& pos,
                                                          const dr4::Vec2f& content_size,
                                                          float             scrollbar_width,
                                                          float             padding )
    : ScrollableWidget( wm, pos, content_size, scrollbar_width )
{
    auto buttons_list =
        std::make_unique<ButtonsList>( wm, dr4::Vec2f{ 0.0f, 0.0f }, content_size, padding );
    buttons_list_ = buttons_list.get();
    setContent( std::move( buttons_list ) );
}

Button*
ScrollableButtonsListWidget::addButton( std::unique_ptr<Button> button )
{
    if ( buttons_list_ == nullptr )
    {
        return nullptr;
    }
    return buttons_list_->addButton( std::move( button ) );
}

void
ScrollableButtonsListWidget::clearButtons()
{
    if ( buttons_list_ == nullptr )
    {
        return;
    }
    buttons_list_->clearButtons();
    buttons_list_->setRelPos( { 0.0f, 0.0f } );
}

void
ScrollableButtonsListWidget::setPadding( float padding )
{
    if ( buttons_list_ == nullptr )
    {
        return;
    }
    buttons_list_->setPad( padding );
}

float
ScrollableButtonsListWidget::getPadding() const
{
    return ( buttons_list_ != nullptr ) ? buttons_list_->getPad() : 0.0f;
}

void
ScrollableButtonsListWidget::rebuildLayout()
{
    if ( buttons_list_ != nullptr )
    {
        buttons_list_->rebuildLayout();
    }
}

ButtonsList*
ScrollableButtonsListWidget::getButtonsList()
{
    return buttons_list_;
}

const ButtonsList*
ScrollableButtonsListWidget::getButtonsList() const
{
    return buttons_list_;
}

} // namespace hui
