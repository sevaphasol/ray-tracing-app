#pragma once

#include "buttons_list.hpp"
#include "scrollable_widget.hpp"
#include <memory>

namespace hui {

class ScrollableButtonsListWidget : public ScrollableWidget {
  public:
    ScrollableButtonsListWidget( WindowManager*   wm,
                                 const dr4::Vec2f& pos,
                                 const dr4::Vec2f& content_size,
                                 float             scrollbar_width,
                                 float             padding = 0.0f );

    Button*
    addButton( std::unique_ptr<Button> button );

    void
    clearButtons();

    void
    setPad( float padding );

    float
    getPad() const;

    void
    rebuildLayout();

    VerticalButtonsList*
    getButtonsList();
    const VerticalButtonsList*
    getButtonsList() const;

  private:
    VerticalButtonsList* buttons_list_ = nullptr;
};

} // namespace hui
