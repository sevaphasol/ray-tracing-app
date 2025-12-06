#pragma once

#include "dr4/event.hpp"

namespace hui {

class Widget;

struct Event
{
    Event() { info = {}; }
    Event( const dr4::Event& info ) : info( info ) {}

    dr4::Event info;

    virtual bool
    apply( Widget* widget ) const
    {
        return false;
    }
};

struct IdleEvent : public Event
{
    IdleEvent() = default;
    IdleEvent( const dr4::Event& info ) : Event( info ) {}
    bool
    apply( Widget* widget ) const override final;
};
struct TextEnteredEvent : public Event
{
    TextEnteredEvent() = default;
    TextEnteredEvent( const dr4::Event& info ) : Event( info ) {}
    bool
    apply( Widget* widget ) const override final;
};
struct KeyPressEvent : public Event
{
    KeyPressEvent() = default;
    KeyPressEvent( const dr4::Event& info ) : Event( info ) {}
    bool
    apply( Widget* widget ) const override final;
};
struct KeyReleaseEvent : public Event
{
    KeyReleaseEvent() = default;
    KeyReleaseEvent( const dr4::Event& info ) : Event( info ) {}
    bool
    apply( Widget* widget ) const override final;
};
struct MousePressEvent : public Event
{
    MousePressEvent() = default;
    MousePressEvent( const dr4::Event& info ) : Event( info ) {}
    bool
    apply( Widget* widget ) const override final;
};
struct MouseReleaseEvent : public Event
{
    MouseReleaseEvent() = default;
    MouseReleaseEvent( const dr4::Event& info ) : Event( info ) {}
    bool
    apply( Widget* widget ) const override final;
};
struct MouseMoveEvent : public Event
{
    MouseMoveEvent() = default;
    MouseMoveEvent( const dr4::Event& info ) : Event( info ) {}
    bool
    apply( Widget* widget ) const override final;
};

} // namespace hui
