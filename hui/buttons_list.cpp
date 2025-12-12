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
    // !FIXME 1.0 set because sfml cannot create texture with zero-size
    setSize( { getSize().x, 1.0f } );
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
    layoutButtons();
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

VerticalButtonsList::VerticalButtonsList( WindowManager*    wm,
                                          const dr4::Vec2f& pos,
                                          const dr4::Vec2f& size,
                                          Theme             theme )
    : ButtonsList( wm, pos, size, theme )
{
}

VerticalButtonsList::VerticalButtonsList( WindowManager*    wm,
                                          const dr4::Vec2f& pos,
                                          const dr4::Vec2f& size,
                                          float             pad )
    : ButtonsList( wm, pos, size, pad )
{
}

void
VerticalButtonsList::layoutButtons()
{
    float y     = 0.0f;
    float max_w = 0.0f;

    const size_t btns_size = btns_.size();

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

    float new_h = ( btns_size == 0 ) ? 1.0f : y;
    float new_w = std::max( getSize().x, max_w );

    setSize( { new_w, new_h } );
}

HorizontalButtonsList::HorizontalButtonsList( WindowManager*    wm,
                                              const dr4::Vec2f& pos,
                                              const dr4::Vec2f& size,
                                              Theme             theme )
    : ButtonsList( wm, pos, size, theme )
{
}

HorizontalButtonsList::HorizontalButtonsList( WindowManager*    wm,
                                              const dr4::Vec2f& pos,
                                              const dr4::Vec2f& size,
                                              float             pad )
    : ButtonsList( wm, pos, size, pad )
{
}

void
HorizontalButtonsList::setAutoWidthByLabel( bool enabled )
{
    auto_width_by_label_ = enabled;
    rebuildLayout();
}

void
HorizontalButtonsList::setLabelPadding( float padding )
{
    label_padding_ = padding;
    rebuildLayout();
}

float
HorizontalButtonsList::getLabelPadding() const
{
    return label_padding_;
}

void
HorizontalButtonsList::layoutButtons()
{
    float x     = 0.0f;
    float max_h = 0.0f;
    float pad   = theme_.pad;
    float lp    = label_padding_;

    for ( size_t i = 0; i < btns_.size(); ++i )
    {
        auto& btn = btns_[i];

        if ( auto_width_by_label_ )
        {
            dr4::Vec2f bounds = btn->getLabelBounds();
            float      width  = bounds.x + 2.0f * lp;
            float      height = std::max( btn->getSize().y, bounds.y + 2.0f * lp );
            btn->setSize( { width, height } );
        }

        btn->setRelPos( { x, 0.0f } );
        x += btn->getSize().x;
        max_h = std::max( max_h, btn->getSize().y );

        if ( i + 1 < btns_.size() )
        {
            x += pad;
        }
    }

    float new_w = std::max( getSize().x, ( btns_.empty() ) ? 1.0f : x );
    float new_h = std::max( getSize().y, max_h > 0.0f ? max_h : 1.0f );

    setSize( { new_w, new_h } );
}

} // namespace hui
