#include "window_manager.hpp"
#include "dr4/event.hpp"
#include "event.hpp"
#include "plugin_manager.hpp"
#include <iostream>

namespace hui {

WindowManager::WindowManager( cum::PluginManager* pm, const dr4::Color& color )
    : window_( pm->getWindow() ),
      background_color_( color ),
      desktop_( pm, { 0, 0 }, pm->getWindow()->GetSize() )
{
}

void
WindowManager::run()
{
    window_->Open();

    while ( window_->IsOpen() )
    {
        handleEvents();
        draw();
    }
}

void
WindowManager::setBackgroundColor( const dr4::Color& color )
{
    background_color_ = color;
}

void
WindowManager::addWidget( std::unique_ptr<hui::Widget> widget )
{
    desktop_.addChild( std::move( widget ) );
}

void
WindowManager::setDeltaTime( float delta_time )
{
    delta_time_ = delta_time;
}

void
WindowManager::handleEvents()
{
    std::optional<dr4::Event> dr4_event = {};
    while ( ( dr4_event = window_->PollEvent() ).has_value() )
    {
        switch ( dr4_event.value().type )
        {
            case dr4::Event::Type::QUIT:
                window_->Close();
                break;
            case dr4::Event::Type::TEXT_EVENT:
                hui::TextEnteredEvent( dr4_event.value() ).apply( &desktop_ );
                break;
            case dr4::Event::Type::KEY_DOWN:
                hui::KeyPressEvent( dr4_event.value() ).apply( &desktop_ );
                break;
            case dr4::Event::Type::KEY_UP:
                hui::KeyReleaseEvent( dr4_event.value() ).apply( &desktop_ );
                break;
            case dr4::Event::Type::MOUSE_DOWN:
                hui::MousePressEvent( dr4_event.value() ).apply( &desktop_ );
                break;
            case dr4::Event::Type::MOUSE_UP:
                hui::MouseReleaseEvent( dr4_event.value() ).apply( &desktop_ );
                break;
            case dr4::Event::Type::MOUSE_MOVE:
                hui::MouseMoveEvent( dr4_event.value() ).apply( &desktop_ );
                break;
            default:
                break;
        }
    }

    hui::IdleEvent().apply( &desktop_ );
}

void
WindowManager::draw()
{
    window_->Clear( background_color_ );
    desktop_.Redraw();
    window_->Draw( *desktop_.getTexture() );
    window_->Display();
}

} // namespace hui
