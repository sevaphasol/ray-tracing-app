#pragma once

#include "custom-hui-impl/container_widget.hpp"
#include "custom-hui-impl/widget.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include "zemax/config.hpp"
#include "zemax/view/camera_control_panel.hpp"
#include "zemax/view/control_panel.hpp"
#include "zemax/view/scene.hpp"
#include "zemax/view/scene_objects_list.hpp"
#include "zemax/view/snapshot_annotator.hpp"

namespace zemax {
namespace view {

class Zemax : public hui::ContainerWidget {
  public:
    explicit Zemax( hui::WindowManager* wm, dr4::Window* window, float toolbar_height )
        : hui::ContainerWidget(
              wm,
              { 0, toolbar_height },
              { Config::Window::Width, Config::Window::Height - toolbar_height } ),
          scene_( wm,
                  Config::Scene::Position,
                  Config::Scene::Size,
                  Config::Scene::BackgroundColor,
                  Config::Camera::Position ),
          camera_panel_(
              wm, scene_.getModel(), Config::CameraPanel::Position, Config::CameraPanel::Size ),
          panel_( wm, scene_.getModel(), Config::ControlPanel::Position, Config::ControlPanel::Size ),
          snp_annotator_( wm, Config::Scene::Position, Config::Scene::Size ),
          obj_list_( wm,
                     1725,
                     Config::CameraPanel::Position.y,
                     Config::CameraPanel::Size.x,
                     200,
                     scene_.getModel(),
                     []() { return; } )
    {
        // // fprintf( stderr, "debug in %s:%d:%s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__ );
        scene_.setParent( this );
        camera_panel_.setParent( this );
        panel_.setParent( this );
        snp_annotator_.setParent( this );
        obj_list_.setParent( this );
    }

    ~Zemax() = default;

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        if ( event.apply( &snp_annotator_ ) )
        {
            return true;
        }

        if ( event.apply( &camera_panel_ ) )
        {
            return true;
        }

        if ( event.apply( &panel_ ) )
        {
            return true;
        }

        if ( event.apply( &scene_ ) )
        {
            return true;
        }

        if ( event.apply( &obj_list_ ) )
        {
            return true;
        }

        return false;
    }

    void
    RedrawMyTexture() const override
    {
        texture_->Clear( { 0, 0, 0 } );

        scene_.Redraw();
        camera_panel_.Redraw();
        panel_.Redraw();
        snp_annotator_.Redraw();
        obj_list_.Redraw();
    }

  private:
    Scene                 scene_;
    CameraControlPanel    camera_panel_;
    ControlPanel          panel_;
    SceneObjectsListModal obj_list_;
    SnapshotAnnotator     snp_annotator_;
};

} // namespace view
} // namespace zemax
