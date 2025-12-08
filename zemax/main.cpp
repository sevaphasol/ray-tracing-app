#include "cum/ifc/dr4.hpp"
#include "cum/manager.hpp"
#include "custom-hui-impl/tool_bar.hpp"
#include "custom-hui-impl/window_manager.hpp"
#include "dr4/math/vec2.hpp"
#include "zemax/config.hpp"
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

    window->SetSize( { zemax::Config::Window::Width, zemax::Config::Window::Height } );
    window->SetTitle( "Test" );
    hui::WindowManager wm( &manager, window );

    auto* font = window->CreateFont();
    font->LoadFromFile( "assets/JetBrainsMono-Regular.ttf" );
    window->SetDefaultFont( font );

    auto zemax      = std::make_unique<zemax::view::Zemax>( &wm, window, 28.0f );
    auto* zemax_ptr = zemax.get();

    auto toolbar     = std::make_unique<hui::ToolBar>( &wm, 28.0f );
    auto* toolbar_ptr = toolbar.get();

    toolbar->addMenu( "File",
                      {
                          { "New", []() { fprintf( stderr, "New\n" ); } },
                          { "Open", []() { fprintf( stderr, "Open\n" ); } },
                          { "Exit", [&]() { wm.getWindow()->Close(); } },
                      } );

    toolbar->addMenu( "Edit",
                      {
                          { "Undo", []() { fprintf( stderr, "Undo\n" ); } },
                          { "Redo", []() { fprintf( stderr, "Redo\n" ); } },
                      } );

    auto fmt_label = []( bool visible, const char* title ) {
        return std::string( visible ? "[x] " : "[ ] " ) + title;
    };

    toolbar->addMenu( "View",
        {
            { fmt_label( zemax_ptr->scene().isVisible(), "Scene" ), [&]() {
                  bool new_state = !zemax_ptr->scene().isVisible();
                  if ( new_state )
                      zemax_ptr->scene().show();
                  else
                      zemax_ptr->scene().hide();
                  toolbar_ptr->setMenuItemLabel( "View", 0, fmt_label( new_state, "Scene" ) );
              } },
            { fmt_label( zemax_ptr->cameraPanel().isVisible(), "Camera Controls" ), [&]() {
                  bool new_state = !zemax_ptr->cameraPanel().isVisible();
                  if ( new_state )
                      zemax_ptr->cameraPanel().show();
                  else
                      zemax_ptr->cameraPanel().hide();
                  toolbar_ptr->setMenuItemLabel(
                      "View", 1, fmt_label( new_state, "Camera Controls" ) );
              } },
            { fmt_label( zemax_ptr->objectPanel().isVisible(), "Object Controls" ), [&]() {
                  bool new_state = !zemax_ptr->objectPanel().isVisible();
                  if ( new_state )
                      zemax_ptr->objectPanel().show();
                  else
                      zemax_ptr->objectPanel().hide();
                  toolbar_ptr->setMenuItemLabel(
                      "View", 2, fmt_label( new_state, "Object Controls" ) );
              } },
            { fmt_label( zemax_ptr->objectsList().isVisible(), "Objects List" ), [&]() {
                  bool new_state = !zemax_ptr->objectsList().isVisible();
                  if ( new_state )
                      zemax_ptr->objectsList().show();
                  else
                      zemax_ptr->objectsList().hide();
                  toolbar_ptr->setMenuItemLabel(
                      "View", 3, fmt_label( new_state, "Objects List" ) );
              } },
            { fmt_label( zemax_ptr->objectEditor().isVisible(), "Object Editor" ), [&]() {
                  bool new_state = !zemax_ptr->objectEditor().isVisible();
                  if ( new_state )
                      zemax_ptr->objectEditor().show();
                  else
                      zemax_ptr->objectEditor().hide();
                  toolbar_ptr->setMenuItemLabel(
                      "View", 4, fmt_label( new_state, "Object Editor" ) );
              } },
        } );

    wm.addWidget( std::move( zemax ) );
    wm.addWidget( std::move( toolbar ) );

    wm.run();

    return 0;
}
