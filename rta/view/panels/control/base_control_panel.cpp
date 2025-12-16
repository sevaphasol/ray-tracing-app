#include "rta/view/panels/control/base_control_panel.hpp"
#include "hui/button.hpp"
#include "hui/closable_panel.hpp"
#include "hui/window_manager.hpp"
#include <memory>
#include <vector>

namespace rta {
namespace view {

rta::view::BaseControlPanel::BaseControlPanel( hui::WindowManager* wm,
                                               const dr4::Vec2f&   pos,
                                               const dr4::Vec2f&   size,
                                               const std::string&  title,
                                               const dr4::Vec2f&   btn_size,
                                               size_t              btns_count_ )
    : hui::ClosablePanel( wm, pos.x, pos.y, size.x, size.y, title ), btn_size_( btn_size )
{
    btns_.reserve( btns_count_ );
}

bool
rta::view::BaseControlPanel::propagateEventToChildren( const hui::Event& event )
{
    if ( !visible_ )
    {
        return false;
    }

    for ( const auto& btn : btns_ )
    {
        if ( event.apply( btn.get() ) )
        {
            return true;
        }
    }

    return hui::DialogBox::propagateEventToChildren( event );
}

void
rta::view::BaseControlPanel::RedrawMyTexture() const
{
    if ( !visible_ )
    {
        return;
    }

    hui::DialogBox::RedrawMyTexture();

    for ( const auto& btn : btns_ )
    {
        btn->Redraw();
    }
}

void
rta::view::BaseControlPanel::addClickButton( const dr4::Vec2f&           pos,
                                             const std::string&          title,
                                             const hui::Button::Theme&   theme,
                                             const hui::Button::ClickCb& cb )
{
    auto* btn = addButton( pos, title, theme );
    btn->setOnClick( cb );
    btn->setFont( wm_->getNerdFont() );
    btn->setTitle( title );
}

void
rta::view::BaseControlPanel::addHoldPressButton( const dr4::Vec2f&               pos,
                                                 const std::string&              title,
                                                 const hui::Button::Theme&       theme,
                                                 const hui::Button::HoldPressCb& cb )
{
    auto* btn = addButton( pos, title, theme );
    btn->setOnHoldPress( cb );
    btn->setFont( wm_->getNerdFont() );
    btn->setTitle( title );
}

hui::Button*
rta::view::BaseControlPanel::addButton( const dr4::Vec2f&         pos,
                                        const std::string&        title,
                                        const hui::Button::Theme& theme )
{
    auto btn = std::make_unique<hui::Button>( wm_, pos, btn_size_, title, theme );

    btn->setParent( this );
    btns_.push_back( std::move( btn ) );

    return btns_.back().get();
}

} // namespace view
} // namespace rta
