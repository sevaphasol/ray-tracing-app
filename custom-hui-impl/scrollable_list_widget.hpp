#pragma once

#include "container_widget.hpp"
#include "scrollbar.hpp"
#include <memory>
#include <vector>

namespace hui {

class ScrollableListWidget : public ContainerWidget {
  public:
    ScrollableListWidget( cum::Manager*     pm,
                          dr4::Window*      win,
                          const dr4::Vec2f& pos,
                          const dr4::Vec2f& content_size,
                          float             scrollbar_width );

    void
    addItem( std::unique_ptr<Widget> item );

    void
    rebuildLayout();

    void
    RedrawMyTexture() const override;
    bool
    propagateEventToChildren( const Event& event ) override final;

  private:
    void
    updateContentPosition();

  private:
    std::vector<std::unique_ptr<Widget>> items_;
    ScrollBar                            scroll_bar_;
    float                                total_items_y_ = 0.0f;
};

} // namespace hui
