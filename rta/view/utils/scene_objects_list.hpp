#pragma once
#include "hui/button.hpp"
#include "hui/dialog_box.hpp"
#include "hui/label.hpp"
#include "hui/scrollable_buttons_list_widget.hpp"
#include "rta/model/primitives/impls/aabb.hpp"
#include "rta/model/primitives/impls/plane.hpp"
#include "rta/model/primitives/impls/sphere.hpp"
#include "rta/model/rendering/scene_manager.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace rta {
namespace view {

class SceneObjectsListModal : public hui::DialogBox {
  public:
    using CloseCb = std::function<void()>;
    SceneObjectsListModal( hui::WindowManager*           wm,
                           float                         x,
                           float                         y,
                           float                         w,
                           float                         h,
                           model::SceneManager&          scene_manager,
                           CloseCb                       close_cb,
                           std::function<void( size_t )> on_select );

    void
    refresh();

    bool
    propagateEventToChildren( const hui::Event& event ) override;

    void
    RedrawMyTexture() const override;

    void
    show();
    void
    hide();
    bool
    isVisible() const;

  private:
    void
    rebuildListItems();

  private:
    model::SceneManager&                              scene_manager_;
    CloseCb                                           close_cb_;
    std::unique_ptr<hui::ScrollableButtonsListWidget> list_;
    std::function<void( size_t )>                     on_select_;
    bool                                              visible_ = true;
};

} // namespace view
} // namespace rta
