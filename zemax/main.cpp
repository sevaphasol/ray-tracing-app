#include "cum/ifc/dr4.hpp"
#include "cum/manager.hpp"
#include "custom-hui-impl/file_dialog_box.hpp"
#include "custom-hui-impl/tool_bar.hpp"
#include "custom-hui-impl/window_manager.hpp"
#include "dr4/math/vec2.hpp"
#include "zemax/view/plugin_manager.hpp"
#include "zemax/view/zemax.hpp"

#include <dlfcn.h>
#include <memory>
#include <string>

int
main()
{
    cum::Manager manager;

    // manager.LoadFromFile( "plugins/libplugin_dr4_Vova.so" );
    manager.LoadFromFile( "plugins/libplugin_dr4.so" );

    manager.LoadFromFile( "plugins/libplugin_pp.so" );
    manager.LoadFromFile( "plugins/libplugin_pp_Vova.so" );
    // manager.LoadFromFile( "plugins/libplugin_pp_Artem_Text.so" );
    // manager.LoadFromFile( "plugins/libplugin_pp_Artem_Line.so" );
    // manager.LoadFromFile( "plugins/libplugin_pp_Artem.so" );
    // manager.LoadFromFile( "plugins/libplugin_pp_Denchick.so" );

    cum::DR4BackendPlugin* dr4_plugin = manager.GetAnyOfType<cum::DR4BackendPlugin>();
    dr4::Window*           window     = dr4_plugin->CreateWindow();

    window->SetSize( { 2150.0f, 900.0f } );
    window->SetTitle( "Test" );
    hui::WindowManager wm( &manager, window );

    auto* font = window->CreateFont();
    font->LoadFromFile( "assets/JetBrainsMono-Regular.ttf" );
    window->SetDefaultFont( font );

    auto  zemax     = std::make_unique<zemax::view::Zemax>( &wm, window, 28.0f );
    auto* zemax_ptr = zemax.get();

    auto  toolbar     = std::make_unique<hui::ToolBar>( &wm, 28.0f );
    auto* toolbar_ptr = toolbar.get();

    const std::string scene_file_default = "scene.txt";

    toolbar->addMenu( "File",
                      {
                          { "New",
                            [zemax_ptr]() {
                                zemax_ptr->scene().getModel().clear();
                                zemax_ptr->scene().clearSelection();
                            } },
                          { "Open",
                            [zemax_ptr, &wm]() {
                                wm.pushModal( std::make_unique<hui::FileDialogBox>(
                                    &wm,
                                    600,
                                    300,
                                    400,
                                    160,
                                    "Open Scene",
                                    "scene.txt",
                                    [zemax_ptr, &wm]( const std::string& path ) {
                                        if ( zemax_ptr->scene().getModel().loadFromFile( path ) )
                                        {
                                            zemax_ptr->scene().clearSelection();
                                        }
                                        wm.popModal();
                                    },
                                    [&wm]() { wm.popModal(); } ) );
                            } },
                          { "Save",
                            [zemax_ptr, &wm, scene_file_default]() {
                                wm.pushModal( std::make_unique<hui::FileDialogBox>(
                                    &wm,
                                    600,
                                    300,
                                    400,
                                    160,
                                    "Save Scene",
                                    scene_file_default,
                                    [zemax_ptr, &wm]( const std::string& path ) {
                                        zemax_ptr->scene().getModel().saveToFile( path );
                                        wm.popModal();
                                    },
                                    [&wm]() { wm.popModal(); } ) );
                            } },
                          { "Exit", [&]() { wm.getWindow()->Close(); } },
                      } );

    auto fmt_label = []( bool visible, const char* title ) {
        return std::string( visible ? "[x] " : "[ ] " ) + title;
    };

    toolbar->addMenu( "Annotator",
                      { { "Plugins",
                          [&wm, zemax_ptr]() {
                              float x = 120.0f;
                              float y = 30.0f;
                              wm.pushModal( std::make_unique<zemax::view::PluginManagerDialog>(
                                  &wm,
                                  x,
                                  y,
                                  340.0f,
                                  320.0f,
                                  &zemax_ptr->annotator() ) );
                          } },
                        { fmt_label( zemax_ptr->annotator().isColorPickerVisible(), "RGB Picker" ),
                          [&, toolbar_ptr]() {
                              if ( !zemax_ptr->annotator().isVisible() )
                                  return;
                              bool new_state = !zemax_ptr->annotator().isColorPickerVisible();
                              zemax_ptr->annotator().setColorPickerVisible( new_state );
                              toolbar_ptr->setMenuItemLabel( "Annotator",
                                                             1,
                                                             fmt_label( new_state, "RGB Picker" ) );
                          } } } );

    toolbar->addMenu(
        "View",
        {
            { fmt_label( zemax_ptr->scene().isVisible(), "Scene" ),
              [&]() {
                  bool new_state = !zemax_ptr->scene().isVisible();
                  if ( new_state )
                      zemax_ptr->scene().show();
                  else
                      zemax_ptr->scene().hide();
                  toolbar_ptr->setMenuItemLabel( "View", 0, fmt_label( new_state, "Scene" ) );
              } },
            { fmt_label( zemax_ptr->cameraPanel().isVisible(), "Camera Controls" ),
              [&]() {
                  bool new_state = !zemax_ptr->cameraPanel().isVisible();
                  if ( new_state )
                      zemax_ptr->cameraPanel().show();
                  else
                      zemax_ptr->cameraPanel().hide();
                  toolbar_ptr->setMenuItemLabel( "View",
                                                 1,
                                                 fmt_label( new_state, "Camera Controls" ) );
              } },
            { fmt_label( zemax_ptr->objectPanel().isVisible(), "Object Controls" ),
              [&]() {
                  bool new_state = !zemax_ptr->objectPanel().isVisible();
                  if ( new_state )
                      zemax_ptr->objectPanel().show();
                  else
                      zemax_ptr->objectPanel().hide();
                  toolbar_ptr->setMenuItemLabel( "View",
                                                 2,
                                                 fmt_label( new_state, "Object Controls" ) );
              } },
            { fmt_label( zemax_ptr->objectsList().isVisible(), "Objects List" ),
              [&]() {
                  bool new_state = !zemax_ptr->objectsList().isVisible();
                  if ( new_state )
                      zemax_ptr->objectsList().show();
                  else
                      zemax_ptr->objectsList().hide();
                  toolbar_ptr->setMenuItemLabel( "View",
                                                 3,
                                                 fmt_label( new_state, "Objects List" ) );
              } },
            { fmt_label( zemax_ptr->objectEditor().isVisible(), "Object Editor" ),
              [&]() {
                  bool new_state = !zemax_ptr->objectEditor().isVisible();
                  if ( new_state )
                      zemax_ptr->objectEditor().show();
                  else
                      zemax_ptr->objectEditor().hide();
                  toolbar_ptr->setMenuItemLabel( "View",
                                                 4,
                                                 fmt_label( new_state, "Object Editor" ) );
              } },
            { fmt_label( zemax_ptr->annotator().isVisible(), "Annotator" ),
              [&]() {
                  bool new_state = !zemax_ptr->annotator().isVisible();
                  if ( new_state )
                      zemax_ptr->annotator().show();
                  else
                      zemax_ptr->annotator().hide();
                  toolbar_ptr->setMenuItemLabel( "View", 5, fmt_label( new_state, "Annotator" ) );
              } },
        } );

    wm.addWidget( std::move( zemax ) );
    wm.addWidget( std::move( toolbar ) );

    wm.run();

    return 0;
}
