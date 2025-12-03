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

    // auto* pp_plugin  = manager.LoadFromFile( "plugins/libpiska.so" );
    // auto* pp_plugin  = manager.LoadFromFile( "plugins/libgeomprim.so" );
    auto* pp_plugin  = manager.LoadFromFile( "plugins/libplugin_pp.so" );
    auto* dr4_plugin = manager.LoadFromFile( "plugins/libplugin_dr4.so" );

    cum::PluginManager pm;

    pm.setupDR4( dr4_plugin );
    pm.setupPP( pp_plugin );

    pm.getWindow()->SetSize( { zemax::Config::Window::Width, zemax::Config::Window::Height } );
    pm.getWindow()->SetTitle( "Test" );
    hui::WindowManager wm( &pm, zemax::Config::Window::BackgroundColor );

    auto* font = pm.getWindow()->CreateFont();
    font->LoadFromFile( "assets/JetBrainsMono-Regular.ttf" );
    pm.getWindow()->SetDefaultFont( font );

    wm.addWidget( std::make_unique<zemax::view::Zemax>( &pm, font ) );

    wm.run();

    return 0;
}
