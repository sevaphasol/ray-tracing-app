#include "event.hpp"
#include "widget.hpp"

namespace hui {

bool
IdleEvent::apply( Widget* widget ) const
{
    return widget->onIdle( *this );
}

bool
TextEnteredEvent::apply( Widget* widget ) const
{
    return widget->onTextEnter( *this );
}

bool
KeyPressEvent::apply( Widget* widget ) const
{
    return widget->onKeyPress( *this );
}

bool
KeyReleaseEvent::apply( Widget* widget ) const
{
    return widget->onKeyRelease( *this );
}

bool
MousePressEvent::apply( Widget* widget ) const
{
    return widget->onMousePress( *this );
}

bool
MouseReleaseEvent::apply( Widget* widget ) const
{
    return widget->onMouseRelease( *this );
}

bool
MouseMoveEvent::apply( Widget* widget ) const
{
    return widget->onMouseMove( *this );
}

} // namespace ui
