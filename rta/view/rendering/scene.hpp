#pragma once

#include "dr4/math/color.hpp"
#include "dr4/texture.hpp"
#include "hui/widget.hpp"
#include "rta/model/rendering/vector2.hpp"
#include "rta/model/rendering/vector3.hpp"

#include "hui/window_manager.hpp"

#include "hui/closable_panel.hpp"
#include "rta/model/primitives/material.hpp"
#include "rta/model/primitives/primitive.hpp"
#include "rta/model/rendering/camera.hpp"
#include "rta/model/rendering/scene_manager.hpp"

#include <cstddef>
#include <iomanip>
#include <memory>
#include <sstream>
#include <thread>
#include <vector>

namespace rta {
namespace view {

class Scene : public hui::ClosablePanel {
  public:
    using SelectionChangedCb = std::function<void( std::optional<size_t> )>;

  private:
    bool                            need_update_ = true;
    SelectionChangedCb              on_selection_changed_;
    std::unique_ptr<dr4::Text>      camera_pos_text_;
    std::unique_ptr<dr4::Rectangle> select_rect_;
    std::unique_ptr<dr4::Rectangle> border_;
    model::SceneManager             model_;
    dr4::Color                      background_color_;
    std::unique_ptr<dr4::Image>     pixels_;

  public:
    ~Scene() = default;

    explicit Scene( hui::WindowManager*         wm,
                    const dr4::Vec2f&           pos,
                    const dr4::Vec2f&           size,
                    const dr4::Color&           background_color,
                    const rta::model::Vector3f& camera_pos );

    void
    setOnSelectionChanged( SelectionChangedCb cb );

    model::SceneManager&
    getModel();

    virtual bool
    onIdle( const hui::Event& event ) override final;

    virtual bool
    onMousePress( const hui::Event& event ) override final;

  private:
    void
    update();

    void
    setCameraPosString();

  public:
    void
    RedrawMyTexture() const override final;

    void
    show();

    void
    hide();

    void
    clearSelection();

    bool
    isVisible() const;

    std::optional<size_t>
    findIndexForObj( model::Primitive* obj ) const;
};

} // namespace view
} // namespace rta
