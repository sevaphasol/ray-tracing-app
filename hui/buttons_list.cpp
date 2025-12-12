#include "buttons_list.hpp"
#include "button.hpp"
#include <algorithm>

namespace hui {

ButtonsList::ButtonsList( WindowManager*    wm,
                          const dr4::Vec2f& pos,
                          const dr4::Vec2f& size,
                          Theme             theme )
    : ContainerWidget( wm, pos, size ), theme_( theme )
{
}

ButtonsList::ButtonsList( WindowManager*    wm,
                          const dr4::Vec2f& pos,
                          const dr4::Vec2f& size,
                          float             pad )
    : ButtonsList( wm, pos, size, { pad, Button::Theme::Default() } )
{
}

Button*
ButtonsList::addButton( std::unique_ptr<Button> button )
{
    if ( !button )
    {
        return nullptr;
    }

    button->setParent( this );
    btns_.push_back( std::move( button ) );
    rebuildLayout();

    return btns_.back().get();
}

void
ButtonsList::clearButtons()
{
    btns_.clear();
    setSize( { getSize().x, 0.0f } );
}

void
ButtonsList::setPad( float pad )
{
    theme_.pad = pad;
    rebuildLayout();
}

float
ButtonsList::getPad() const
{
    return theme_.pad;
}

std::vector<std::unique_ptr<Button>>&
ButtonsList::getButtons()
{
    return btns_;
}

const std::vector<std::unique_ptr<Button>>&
ButtonsList::getButtons() const
{
    return btns_;
}

void
ButtonsList::rebuildLayout()
{
    float y     = 0.0f;
    float max_w = 0.0f;

    size_t btns_size = btns_.size();

    for ( size_t i = 0; i < btns_size; ++i )
    {
        auto& btn = btns_[i];

        btn->setRelPos( { 0.0f, y } );
        max_w = std::max( max_w, btn->getSize().x );
        y += btn->getSize().y;

        if ( i + 1 < btns_.size() )
        {
            y += theme_.pad;
        }
    }

    float new_h = y;
    float new_w = std::max( getSize().x, max_w );

    setSize( { new_w, new_h } );
}

bool
ButtonsList::propagateEventToChildren( const Event& event )
{
    const bool is_mouse_move = dynamic_cast<const hui::MouseMoveEvent*>( &event ) != nullptr;

    bool handled = false;
    for ( auto& btn : btns_ )
    {
        if ( event.apply( btn.get() ) )
        {
            handled = true;
            if ( !is_mouse_move )
            {
                return true;
            }
        }
    }

    return handled;
}

void
ButtonsList::RedrawMyTexture() const
{
    for ( const auto& btn : btns_ )
    {
        btn->Redraw();
    }
}

} // namespace hui
