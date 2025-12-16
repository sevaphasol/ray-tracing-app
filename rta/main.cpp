#include "cum/ifc/dr4.hpp"
#include "cum/manager.hpp"
#include "dr4/math/vec2.hpp"
#include "hui/window_manager.hpp"
#include "rta/view/rta.hpp"
#include <dlfcn.h>
#include <memory>
#include <string>

int
main()
{
    cum::Manager manager;

    manager.LoadFromFile( "plugins/libplugin_dr4.so" );
    manager.LoadFromFile( "plugins/libplugin_pp.so" );

    cum::DR4BackendPlugin* dr4_plugin = manager.GetAnyOfType<cum::DR4BackendPlugin>();
    dr4::Window*           window     = dr4_plugin->CreateWindow();

    window->SetSize( { 2030.0f, 940.0f } );
    window->SetTitle( "Test" );
    hui::WindowManager wm( &manager, window );

    auto* default_font = window->CreateFont();
    default_font->LoadFromFile( "assets/SF-Pro-Text-Regular.ttf" );
    window->SetDefaultFont( default_font );

    auto* nerd_font = window->CreateFont();
    nerd_font->LoadFromFile( "assets/CaskaydiaCoveNerdFontMono-Regular.ttf" );
    wm.setNerdFont( nerd_font );

    auto  rta     = std::make_unique<rta::view::RayTracingApp>( &wm, window );
    auto* rta_ptr = rta.get();

    wm.addWidget( std::move( rta ) );

    wm.run();

    return 0;
}
