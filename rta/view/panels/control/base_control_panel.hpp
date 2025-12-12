#pragma once

#include "hui/button.hpp"
#include "hui/closable_panel.hpp"
#include "hui/window_manager.hpp"
#include <memory>
#include <vector>

namespace rta {
namespace view {

class BaseControlPanel : public hui::ClosablePanel {
  private:
    std::vector<std::unique_ptr<hui::Button>> btns_;
    dr4::Vec2f                                btn_size_;

  public:
    BaseControlPanel( hui::WindowManager* wm,
                      const dr4::Vec2f&   pos,
                      const dr4::Vec2f&   size,
                      const std::string&  title,
                      const dr4::Vec2f&   btn_size,
                      size_t              btns_count_ );

    bool
    propagateEventToChildren( const hui::Event& event ) override;

    void
    RedrawMyTexture() const override;

  protected:
    void
    addClickButton( const dr4::Vec2f&           pos,
                    const std::string&          title,
                    const hui::Button::Theme&   theme,
                    const hui::Button::ClickCb& cb );

    void
    addHoldPressButton( const dr4::Vec2f&               pos,
                        const std::string&              title,
                        const hui::Button::Theme&       theme,
                        const hui::Button::HoldPressCb& cb );

  private:
    hui::Button*
    addButton( const dr4::Vec2f& pos, const std::string& title, const hui::Button::Theme& theme );
};

} // namespace view
} // namespace rta
