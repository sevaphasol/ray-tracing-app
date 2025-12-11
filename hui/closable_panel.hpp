#pragma once

#include "hui/dialog_box.hpp"

namespace hui {

// Simple reusable panel with a top bar and a working close button.
class ClosablePanel : public DialogBox {
  public:
    ClosablePanel( hui::WindowManager* wm,
                   float               x,
                   float               y,
                   float               w,
                   float               h,
                   const std::string&  title )
        : DialogBox( wm, x, y, w, h, [this]() { this->hide(); }, title )
    {
    }

    void
    show()
    {
        visible_ = true;
    }

    void
    hide()
    {
        visible_ = false;
    }

    bool
    isVisible() const
    {
        return visible_;
    }

    dr4::Vec2f
    contentOffset() const
    {
        return { 0.0f, TopBarHeight };
    }

    dr4::Vec2f
    contentSize() const
    {
        return { size_.x, size_.y - TopBarHeight };
    }

  protected:
    bool visible_ = true;
};

} // namespace hui
