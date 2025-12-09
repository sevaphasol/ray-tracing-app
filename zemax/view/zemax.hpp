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
#include "zemax/view/object_editor_panel.hpp"
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
          camera_panel_( wm,
                         scene_.getModel(),
                         Config::CameraPanel::Position,
                         Config::CameraPanel::Size ),
          panel_( wm,
                  scene_.getModel(),
                  Config::ControlPanel::Position,
                  Config::ControlPanel::Size ),
        snp_annotator_( wm, Config::Scene::Position, Config::Scene::Size ),
          obj_list_( wm,
                     1725,
                     Config::CameraPanel::Position.y,
                     Config::CameraPanel::Size.x,
                     200,
                     scene_.getModel(),
                     []() { return; },
                     [this]( size_t idx ) {
                         scene_.getModel().setTargetObj( scene_.getModel().getObjects()[idx].get() );
                         editor_.setTarget( idx );
                     } ),
         editor_( wm,
                  scene_.getModel(),
                  { Config::CameraPanel::Position.x, Config::ControlPanel::Position.y + 340.0f },
                  { Config::ControlPanel::Size.x, 320.0f } )
    {
        scene_.setParent( this );
        camera_panel_.setParent( this );
        panel_.setParent( this );
        obj_list_.setParent( this );
        editor_.setParent( this );

        syncAnnotatorWithScene();

        scene_.setOnSelectionChanged( [this]( std::optional<size_t> idx ) {
            editor_.setTarget( idx );
        } );
    }

    ~Zemax() = default;

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        syncAnnotatorWithScene();

        if ( event.apply( &snp_annotator_ ) )
        {
            return true;
        }

        if ( camera_panel_.isVisible() && event.apply( &camera_panel_ ) )
        {
            return true;
        }

        if ( panel_.isVisible() && event.apply( &panel_ ) )
        {
            return true;
        }

        if ( scene_.isVisible() && event.apply( &scene_ ) )
        {
            return true;
        }

        if ( obj_list_.isVisible() && event.apply( &obj_list_ ) )
        {
            return true;
        }

        if ( editor_.isVisible() && event.apply( &editor_ ) )
        {
            return true;
        }

        return false;
    }

    void
    RedrawMyTexture() const override
    {
        texture_->Clear( { 0, 0, 0 } );

        const_cast<Zemax*>( this )->syncAnnotatorWithScene();

        scene_.Redraw();
        if ( camera_panel_.isVisible() )
        {
            camera_panel_.Redraw();
        }
        if ( panel_.isVisible() )
        {
            panel_.Redraw();
        }
        snp_annotator_.Redraw();
        if ( obj_list_.isVisible() )
        {
            obj_list_.Redraw();
        }
        if ( editor_.isVisible() )
        {
            editor_.Redraw();
        }
    }

    Scene&
    scene()
    {
        return scene_;
    }

    CameraControlPanel&
    cameraPanel()
    {
        return camera_panel_;
    }

    ControlPanel&
    objectPanel()
    {
        return panel_;
    }

    SceneObjectsListModal&
    objectsList()
    {
        return obj_list_;
    }

    ObjectEditorPanel&
    objectEditor()
    {
        return editor_;
    }

    SnapshotAnnotator&
    annotator()
    {
        return snp_annotator_;
    }

  private:
    void
    syncAnnotatorWithScene()
    {
        auto content_pos  = scene_.contentOffset();
        auto content_size = scene_.contentSize();
        snp_annotator_.setParent( this );
        snp_annotator_.setRelPos( scene_.getRelPos() + content_pos );
        snp_annotator_.setSize( content_size );
    }

    Scene                 scene_;
    CameraControlPanel    camera_panel_;
    ControlPanel          panel_;
    SceneObjectsListModal obj_list_;
    ObjectEditorPanel     editor_;
    SnapshotAnnotator     snp_annotator_;
};

} // namespace view
} // namespace zemax
