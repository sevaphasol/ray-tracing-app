#include "rta/view/rta.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include "hui/container_widget.hpp"
#include "hui/widget.hpp"
#include "rta/view/annotator/snapshot_annotator.hpp"
#include "rta/view/panels/control/camera_control_panel.hpp"
#include "rta/view/panels/control/objects_control_panel.hpp"
#include "rta/view/panels/editor/object_editor_panel.hpp"
#include "rta/view/rendering/scene.hpp"
#include "rta/view/utils/scene_objects_list.hpp"

rta::view::RayTracingApp::RayTracingApp( hui::WindowManager* wm,
                                         dr4::Window*        window,
                                         float               toolbar_height )
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

bool
rta::view::RayTracingApp::propagateEventToChildren( const hui::Event& event )
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
rta::view::RayTracingApp::RedrawMyTexture() const
{
    texture_->Clear( { 0, 0, 0 } );

    const_cast<RayTracingApp*>( this )->syncAnnotatorWithScene();

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

rta::view::Scene&
rta::view::RayTracingApp::scene()
{
    return scene_;
}

rta::view::CameraControlPanel&
rta::view::RayTracingApp::cameraPanel()
{
    return camera_panel_;
}

rta::view::ObjectsControlPanel&
rta::view::RayTracingApp::objectPanel()
{
    return panel_;
}

rta::view::SceneObjectsListModal&
rta::view::RayTracingApp::objectsList()
{
    return obj_list_;
}

rta::view::ObjectEditorPanel&
rta::view::RayTracingApp::objectEditor()
{
    return editor_;
}

rta::view::SnapshotAnnotator&
rta::view::RayTracingApp::annotator()
{
    return snp_annotator_;
}

void
rta::view::RayTracingApp::syncAnnotatorWithScene()
{
    auto content_pos  = scene_.contentOffset();
    auto content_size = scene_.contentSize();
    snp_annotator_.setParent( this );
    snp_annotator_.setRelPos( scene_.getRelPos() + content_pos );
    snp_annotator_.setSize( content_size );
}
