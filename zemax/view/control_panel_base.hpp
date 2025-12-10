#pragma once

#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/closable_panel.hpp"
#include "custom-hui-impl/window_manager.hpp"
#include <memory>
#include <vector>

namespace zemax {
namespace view {

// Lightweight base panel with common button plumbing for control panels.
class ControlPanelBase : public hui::ClosablePanel {
  public:
    ControlPanelBase( hui::WindowManager* wm,
                      const dr4::Vec2f&   pos,
                      const dr4::Vec2f&   size,
                      const dr4::Vec2f&   button_size,
                      const std::string&  title )
        : hui::ClosablePanel( wm, pos.x, pos.y, size.x, size.y, title ), button_size_( button_size )
    {
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        if ( !visible_ )
        {
            return false;
        }
        for ( const auto& btn : buttons_ )
        {
            if ( event.apply( btn.get() ) )
                return true;
        }
        return hui::DialogBox::propagateEventToChildren( event );
    }

    bool
    onIdle( const hui::Event& event ) override final
    {
        if ( !visible_ )
            return false;
        propagateEventToChildren( event );
        return false;
    }

    void
    RedrawMyTexture() const override
    {
        if ( !visible_ )
            return;
        hui::DialogBox::RedrawMyTexture();
        for ( const auto& btn : buttons_ )
            btn->Redraw();
    }

  protected:
    hui::Button*
    addButton( const dr4::Vec2f& pos, const std::string& title, const hui::Button::Theme& theme )
    {
        buttons_.push_back( std::make_unique<hui::Button>( wm_, pos, button_size_, title, theme ) );
        buttons_.back()->setParent( this );
        return buttons_.back().get();
    }

    std::vector<std::unique_ptr<hui::Button>> buttons_;
    dr4::Vec2f                                button_size_;
};

} // namespace view
} // namespace zemax
