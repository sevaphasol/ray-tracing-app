#pragma once

#include "hui/button.hpp"
#include "hui/closable_panel.hpp"
#include "hui/window_manager.hpp"
#include <memory>
#include <vector>

namespace rta {
namespace view {

class BaseControlPanel : public hui::ClosablePanel {
  public:
    BaseControlPanel( hui::WindowManager* wm,
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
    propagateEventToChildren( const hui::Event& event ) override
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
    RedrawMyTexture() const override
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

  protected:
    void
    addClickButton( const dr4::Vec2f&           pos,
                    const std::string&          title,
                    const hui::Button::Theme&   theme,
                    const hui::Button::ClickCb& cb )
    {
        addButton( pos, title, theme )->setOnClick( cb );
    }

    void
    addHoldPressButton( const dr4::Vec2f&               pos,
                        const std::string&              title,
                        const hui::Button::Theme&       theme,
                        const hui::Button::HoldPressCb& cb )
    {
        addButton( pos, title, theme )->setOnHoldPress( cb );
    }

  private:
    hui::Button*
    addButton( const dr4::Vec2f& pos, const std::string& title, const hui::Button::Theme& theme )
    {
        auto btn = std::make_unique<hui::Button>( wm_, pos, btn_size_, title, theme );

        btn->setParent( this );
        btns_.push_back( std::move( btn ) );

        return btns_.back().get();
    }

    std::vector<std::unique_ptr<hui::Button>> btns_;
    dr4::Vec2f                                btn_size_;
};

} // namespace view
} // namespace rta
