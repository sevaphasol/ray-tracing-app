#pragma once

#include "zemax/view/obj_info_box.hpp"
#include <string>

namespace zemax {
namespace view {

// Simple reusable panel with a top bar and a working close button.
// Derived widgets should check isVisible() in their event/redraw handlers.
class ClosablePanel : public ObjInfoBox {
  public:
    ClosablePanel( hui::WindowManager* wm,
                   float               x,
                   float               y,
                   float               w,
                   float               h,
                   const std::string&  title )
        : ObjInfoBox( wm, x, y, w, h, [this]() { this->hide(); }, title )
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

    // Helpers for derived widgets to compute client area under the top bar.
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

} // namespace view
} // namespace zemax
