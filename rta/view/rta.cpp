#include "rta/view/rta.hpp"
#include "custom-hui/container_widget.hpp"
#include "custom-hui/file_dialog_box.hpp"
#include "custom-hui/message_box.hpp"
#include "custom-hui/tool_bar.hpp"
#include "custom-hui/widget.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include "rta/view/annotator/snapshot_annotator.hpp"
#include "rta/view/panels/control/camera_control_panel.hpp"
#include "rta/view/panels/control/objects_control_panel.hpp"
#include "rta/view/panels/editor/object_editor_panel.hpp"
#include "rta/view/rendering/scene.hpp"
#include "rta/view/utils/plugin_manager.hpp"
#include "rta/view/utils/scene_objects_list.hpp"

rta::view::RayTracingApp::RayTracingApp( hui::WindowManager* wm,
                                         dr4::Window*        window,
                                         float               toolbar_height )
    : hui::ContainerWidget( wm,
                            { 0, toolbar_height },
                            { window->GetSize().x, window->GetSize().y - toolbar_height } ),
      scene_( wm,
              { 375.0f, 50.0f },
              { 1200.0f, 800.0f },
              { 10, 10, 10, 255 },
              { 0.0f, 0.0f, 0.0f } ),
      camera_panel_( wm, scene_.getModel() ),
      panel_( wm, scene_.getModel() ),
      snp_annotator_( wm, { 475.0f, 50.0f }, { 1200.0f, 800.0f } ),
      obj_list_(
          wm,
          1605,
          camera_panel_.getRelPos().y,
          camera_panel_.getSize().x + 85,
          200,
          scene_.getModel(),
          []() { return; },
          [this]( size_t idx ) {
              scene_.getModel().setTargetObj( scene_.getModel().getObjects()[idx].get() );
              editor_.setTarget( idx );
          } ),
      editor_( wm, scene_.getModel(), { 1605.0f, 275.0f }, { 380.0f, 575.0f }, [this]() {
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

    setupToolbar();
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

void
rta::view::RayTracingApp::setupToolbar()
{
    auto  toolbar     = std::make_unique<hui::ToolBar>( wm_ );
    auto* toolbar_ptr = toolbar.get();

    const std::string scene_file_default = "scene.json";

    toolbar->addMenu( "File",
                      {
                          { "New",
                            [this]() {
                                scene_.getModel().clear();
                                scene_.clearSelection();
                            } },
                          { "Open",
                            [this, scene_file_default]() {
                                wm_->pushModal( std::make_unique<hui::FileDialogBox>(
                                    wm_,
                                    600,
                                    300,
                                    400,
                                    160,
                                    "Open Scene",
                                    scene_file_default,
                                    [this]( const std::string& path ) {
                                        if ( scene_.getModel().loadFromFile( path ) )
                                        {
                                            scene_.clearSelection();
                                        }
                                        wm_->popModal();
                                    },
                                    [this]() { wm_->popModal(); } ) );
                            } },
                          { "Save",
                            [this, scene_file_default]() {
                                wm_->pushModal( std::make_unique<hui::FileDialogBox>(
                                    wm_,
                                    600,
                                    300,
                                    400,
                                    160,
                                    "Save Scene",
                                    scene_file_default,
                                    [this]( const std::string& path ) {
                                        scene_.getModel().saveToFile( path );
                                        wm_->popModal();
                                    },
                                    [this]() { wm_->popModal(); } ) );
                            } },
                          { "Exit", [&]() { wm_->getWindow()->Close(); } },
                      } );

    auto fmt_label = []( bool visible, const char* title ) {
        return std::string( visible ? "[x] " : "[ ] " ) + title;
    };

    toolbar_ptr->addMenu(
        "Annotator",
        { { "Tools",
            [this, fmt_label, toolbar_ptr]() {
                float x = 120.0f;
                float y = 30.0f;
                if ( !snp_annotator_.hasActivePlugin() )
                {
                    wm_->pushModal(
                        std::make_unique<hui::MessageBox>( wm_,
                                                           x,
                                                           y,
                                                           300.0f,
                                                           160.0f,
                                                           "No plugin",
                                                           "No active plugin is selected." ) );
                    return;
                }
                wm_->pushModal(
                    std::make_unique<rta::view::ToolSelectorDialog>( wm_,
                                                                     x,
                                                                     y,
                                                                     320.0f,
                                                                     300.0f,
                                                                     &snp_annotator_ ) );
            } },
          { "Plugins",
            [this, fmt_label, toolbar_ptr]() {
                float x = 120.0f;
                float y = 30.0f;
                wm_->pushModal(
                    std::make_unique<rta::view::PluginManagerDialog>( wm_,
                                                                      x,
                                                                      y,
                                                                      340.0f,
                                                                      320.0f,
                                                                      &snp_annotator_ ) );
            } },
          { fmt_label( snp_annotator_.isColorPickerVisible(), "RGB Picker" ),
            [this, fmt_label, toolbar_ptr]() {
                bool new_state = !snp_annotator_.isColorPickerVisible();
                snp_annotator_.setColorPickerVisible( new_state );
                toolbar_ptr->setMenuItemLabel( "Annotator",
                                               2,
                                               fmt_label( new_state, "RGB Picker" ) );
            } } } );

    toolbar->addMenu(
        "View",
        {
            { fmt_label( scene_.isVisible(), "Scene" ),
              [&, this, fmt_label, toolbar_ptr]() {
                  bool new_state = !scene_.isVisible();
                  if ( new_state )
                      scene_.show();
                  else
                      scene_.hide();
                  toolbar_ptr->setMenuItemLabel( "View", 0, fmt_label( new_state, "Scene" ) );
              } },
            { fmt_label( camera_panel_.isVisible(), "Camera Controls" ),
              [&, this, fmt_label, toolbar_ptr]() {
                  bool new_state = !camera_panel_.isVisible();
                  if ( new_state )
                      camera_panel_.show();
                  else
                      camera_panel_.hide();
                  toolbar_ptr->setMenuItemLabel( "View",
                                                 1,
                                                 fmt_label( new_state, "Camera Controls" ) );
              } },
            { fmt_label( panel_.isVisible(), "Object Controls" ),
              [this, fmt_label, toolbar_ptr]() {
                  bool new_state = !panel_.isVisible();
                  if ( new_state )
                      panel_.show();
                  else
                      panel_.hide();
                  toolbar_ptr->setMenuItemLabel( "View",
                                                 2,
                                                 fmt_label( new_state, "Object Controls" ) );
              } },
            { fmt_label( obj_list_.isVisible(), "Objects List" ),
              [this, fmt_label, toolbar_ptr]() {
                  bool new_state = !obj_list_.isVisible();
                  if ( new_state )
                      obj_list_.show();
                  else
                      obj_list_.hide();
                  toolbar_ptr->setMenuItemLabel( "View",
                                                 3,
                                                 fmt_label( new_state, "Objects List" ) );
              } },
            { fmt_label( editor_.isVisible(), "Object Editor" ),
              [this, fmt_label, toolbar_ptr]() {
                  bool new_state = !editor_.isVisible();
                  if ( new_state )
                      editor_.show();
                  else
                      editor_.hide();
                  toolbar_ptr->setMenuItemLabel( "View",
                                                 4,
                                                 fmt_label( new_state, "Object Editor" ) );
              } },
            { fmt_label( snp_annotator_.isVisible(), "Annotator" ),
              [this, fmt_label, toolbar_ptr]() {
                  bool new_state = !snp_annotator_.isVisible();
                  if ( new_state )
                      snp_annotator_.show();
                  else
                      snp_annotator_.hide();
                  toolbar_ptr->setMenuItemLabel( "View", 5, fmt_label( new_state, "Annotator" ) );
              } },
        } );

    wm_->addWidget( std::move( toolbar ) );
}
