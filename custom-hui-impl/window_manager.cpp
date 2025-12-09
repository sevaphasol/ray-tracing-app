#include "window_manager.hpp"
#include "cum/manager.hpp"
#include "dr4/event.hpp"
#include "dr4/window.hpp"
#include "event.hpp"
#include <memory>

namespace hui {

WindowManager::WindowManager( cum::Manager* pm, dr4::Window* win )
    : pm_( pm ), win_( win ), desktop_( this, { 0, 0 }, win->GetSize() )
{
}

void
WindowManager::run()
{
    win_->Open();
    while ( win_->IsOpen() )
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

hui::Widget*
WindowManager::getTopModal() const
{
    if ( modal_widgets_.empty() )
        return nullptr;
    return modal_widgets_.back().get();
}

void
WindowManager::pushModal( std::unique_ptr<hui::Widget> wgt )
{
    // wgt->setParent( &desktop_ );
    modal_widgets_.push_back( std::move( wgt ) );
}

void
WindowManager::popModal()
{
    if ( !modal_widgets_.empty() )
        modal_widgets_.pop_back();
}

void
WindowManager::handleEvents()
{
    hui::Widget* target_wgt = !modal_widgets_.empty() ? modal_widgets_.back().get() : &desktop_;

    std::optional<dr4::Event> dr4_event;
    while ( ( dr4_event = win_->PollEvent() ).has_value() )
    {
        if ( !target_wgt )
            break;

        switch ( dr4_event->type )
        {
            case dr4::Event::Type::QUIT:
                win_->Close();
                break;
            case dr4::Event::Type::TEXT_EVENT:
                hui::TextEnteredEvent( *dr4_event ).apply( target_wgt );
                break;
            case dr4::Event::Type::KEY_DOWN:
                hui::KeyPressEvent( *dr4_event ).apply( target_wgt );
                break;
            case dr4::Event::Type::KEY_UP:
                hui::KeyReleaseEvent( *dr4_event ).apply( target_wgt );
                break;
            case dr4::Event::Type::MOUSE_DOWN:
                hui::MousePressEvent( *dr4_event ).apply( target_wgt );
                break;
            case dr4::Event::Type::MOUSE_UP:
                // std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' << __func__
                // << std::endl;
                hui::MouseReleaseEvent( *dr4_event ).apply( target_wgt );
                // std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' << __func__
                // << std::endl;
                break;
            case dr4::Event::Type::MOUSE_MOVE:
                hui::MouseMoveEvent( *dr4_event ).apply( target_wgt );
                break;
            default:
                break;
        }
    }

    hui::IdleEvent().apply( &desktop_ );
    for ( auto& modal : modal_widgets_ )
        hui::IdleEvent().apply( modal.get() );
}

void
WindowManager::draw()
{
    win_->Clear( background_color_ );

    // 1. Основной desktop
    desktop_.Redraw();
    win_->Draw( *desktop_.getTexture() );

    for ( auto& modal : modal_widgets_ )
    {
        modal->getTexture()->Clear( { 0, 0, 0, 0 } );
        modal->RedrawMyTexture();
        win_->Draw( *modal->getTexture() );
    }

    win_->Display();
}

} // namespace hui
