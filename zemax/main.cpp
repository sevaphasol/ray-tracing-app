#include "cum/ifc/dr4.hpp"
#include "cum/manager.hpp"
#include "custom-hui-impl/tool_bar.hpp"
#include "custom-hui-impl/window_manager.hpp"
#include "dr4/math/vec2.hpp"
#include "zemax/config.hpp"
#include "zemax/view/zemax.hpp"

#include <dlfcn.h>
#include <memory>

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

    auto zemax = std::make_unique<zemax::view::Zemax>( &wm, window, 28.0f );

    auto toolbar = std::make_unique<hui::ToolBar>( &wm, 28.0f );

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

    wm.addWidget( std::move( zemax ) );
    wm.addWidget( std::move( toolbar ) );

    wm.run();

    return 0;
}
