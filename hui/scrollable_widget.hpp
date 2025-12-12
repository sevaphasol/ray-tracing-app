#pragma once

#include "container_widget.hpp"
#include "scrollbar.hpp"
#include "window_manager.hpp"
#include <memory>

namespace hui {

class ScrollableWidget : public ContainerWidget {
  public:
    ScrollableWidget( hui::WindowManager* wm,
                      const dr4::Vec2f&   pos,
                      const dr4::Vec2f&   content_size,
                      float               scrollbar_width );

    void
    setContent( std::unique_ptr<Widget> content );

    void
    setScrollFactor( double factor );

    void
    RedrawMyTexture() const override;
    bool
    propagateEventToChildren( const Event& event ) override final;

  private:
    void
    updateContentPosition();

  private:
    std::unique_ptr<Widget> content_;
    ScrollBar               scroll_bar_;
    mutable bool            needs_redraw_ = true;
};

} // namespace hui
