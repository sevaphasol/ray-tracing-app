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
    setPadding( float padding );

    float
    getPadding() const;

    void
    rebuildLayout();

    ButtonsList*
    getButtonsList();
    const ButtonsList*
    getButtonsList() const;

  private:
    ButtonsList* buttons_list_ = nullptr;
};

} // namespace hui
