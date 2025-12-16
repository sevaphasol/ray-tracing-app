#pragma once

#include "custom-hui/container_widget.hpp"
#include "custom-hui/widget.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include "rta/view/annotator/snapshot_annotator.hpp"
#include "rta/view/panels/control/camera_control_panel.hpp"
#include "rta/view/panels/control/objects_control_panel.hpp"
#include "rta/view/panels/editor/object_editor_panel.hpp"
#include "rta/view/rendering/scene.hpp"
#include "rta/view/utils/scene_objects_list.hpp"

namespace rta {
namespace view {

class RayTracingApp : public hui::ContainerWidget {
  public:
    explicit RayTracingApp( hui::WindowManager* wm,
                            dr4::Window*        window,
                            float               toolbar_height = 35.0f );

    ~RayTracingApp() = default;

    bool
    propagateEventToChildren( const hui::Event& event ) override;

    void
    RedrawMyTexture() const override;

    void
    setupToolbar();

    Scene&
    scene();
    CameraControlPanel&
    cameraPanel();
    ObjectsControlPanel&
    objectPanel();
    SceneObjectsListModal&
    objectsList();
    ObjectEditorPanel&
    objectEditor();
    SnapshotAnnotator&
    annotator();

  private:
    void
    syncAnnotatorWithScene();

    Scene                 scene_;
    CameraControlPanel    camera_panel_;
    ObjectsControlPanel   panel_;
    SceneObjectsListModal obj_list_;
    ObjectEditorPanel     editor_;
    SnapshotAnnotator     snp_annotator_;
};

} // namespace view
} // namespace rta
