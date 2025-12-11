#pragma once

#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include "hui/container_widget.hpp"
#include "hui/widget.hpp"
#include "rta/view/camera_control_panel.hpp"
#include "rta/view/control_panel.hpp"
#include "rta/view/object_editor_panel.hpp"
#include "rta/view/scene.hpp"
#include "rta/view/scene_objects_list.hpp"
#include "rta/view/snapshot_annotator.hpp"

namespace rta {
namespace view {

class rta : public hui::ContainerWidget {
  public:
    explicit rta( hui::WindowManager* wm, dr4::Window* window, float toolbar_height )
        : hui::ContainerWidget( wm,
                                { 0, toolbar_height },
                                { window->GetSize().x, window->GetSize().y - toolbar_height } ),
          scene_( wm,
                  { 475.0f, 50.0f },
                  { 1200.0f, 800.0f },
                  { 10, 10, 10, 255 },
                  { 0.0f, 0.0f, 0.0f } ),
          camera_panel_( wm, scene_.getModel() ),
          panel_( wm, scene_.getModel() ),
          snp_annotator_( wm, { 475.0f, 50.0f }, { 1200.0f, 800.0f } ),
          obj_list_(
              wm,
              1725,
              camera_panel_.getRelPos().y,
              camera_panel_.getSize().x,
              200,
              scene_.getModel(),
              []() { return; },
              [this]( size_t idx ) {
                  scene_.getModel().setTargetObj( scene_.getModel().getObjects()[idx].get() );
                  editor_.setTarget( idx );
              } ),
          editor_( wm, scene_.getModel(), { 1725.0f, 275.0f }, { 380.0f, 575.0f }, [this]() {
              obj_list_.refresh();
          } )
    {
        scene_.setParent( this );
        camera_panel_.setParent( this );
        panel_.setParent( this );
        obj_list_.setParent( this );
        editor_.setParent( this );

        syncAnnotatorWithScene();

        scene_.setOnSelectionChanged(
            [this]( std::optional<size_t> idx ) { editor_.setTarget( idx ); } );
    }

    ~rta() = default;

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

        const_cast<rta*>( this )->syncAnnotatorWithScene();

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
} // namespace rta
