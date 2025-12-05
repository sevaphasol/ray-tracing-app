#include "cum/ifc/dr4.hpp"
#include "cum/manager.hpp"
#include "custom-hui-impl/plugin_manager.hpp"
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

    manager.LoadFromFile( "plugins/libplugin_pp.so" );
    manager.LoadFromFile( "plugins/libpiska.so" );
    // manager.LoadFromFile( "plugins/libgeomprim.so" );
    manager.LoadFromFile( "plugins/libplugin_dr4.so" );

    cum::DR4BackendPlugin* dr4_plugin = manager.GetAnyOfType<cum::DR4BackendPlugin>();
    dr4::Window*           window     = dr4_plugin->CreateWindow();

    window->SetSize( { zemax::Config::Window::Width, zemax::Config::Window::Height } );
    window->SetTitle( "Test" );
    hui::WindowManager wm( &manager, window, zemax::Config::Window::BackgroundColor );

    auto* font = window->CreateFont();
    font->LoadFromFile( "assets/JetBrainsMono-Regular.ttf" );
    window->SetDefaultFont( font );

    wm.addWidget( std::make_unique<zemax::view::Zemax>( &manager, window, font ) );

    wm.run();

    return 0;
}
