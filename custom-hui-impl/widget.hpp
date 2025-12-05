#pragma once

#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include "dr4/window.hpp"
#include "event.hpp"
#include "plugin_manager.hpp"

// #include "gfx/core/drawable.hpp"
// #include "gfx/core/transformable.hpp"
// #include "gfx/core/vector2.hpp"

#include <memory>
#include <vector>

namespace hui {

class Widget {
  public:
    explicit Widget( cum::Manager* pm, dr4::Window* win, float x, float y, float w, float h );
    explicit Widget( cum::Manager*     pm,
                     dr4::Window*      win,
                     const dr4::Vec2f& pos,
                     const dr4::Vec2f& size );
    virtual ~Widget() = default;

    virtual bool
    onIdle( const hui::Event& event );
    virtual bool
    onTextEnter( const hui::Event& event );
    virtual bool
    onKeyPress( const hui::Event& event );
    virtual bool
    onKeyRelease( const hui::Event& event );
    virtual bool
    onMousePress( const hui::Event& event );
    virtual bool
    onMouseRelease( const hui::Event& event );
    virtual bool
    onMouseMove( const hui::Event& event );

    virtual bool
    onMe( dr4::Vec2f rel ) const;

    bool
    isHovered() const;

    bool
    isDraggable() const;
    void
    setDraggable( bool state );

    Widget*
    getParent();
    void
    setParent( Widget* new_parent );

    dr4::Vec2f
    getSize() const;
    virtual void
    setSize( const dr4::Vec2f& size );
    dr4::Vec2f
    getRelPos() const;
    virtual void
    setRelPos( const dr4::Vec2f& pos );
    virtual void
    setRelPos( float x, float y );
    dr4::Vec2f
    getAbsPos() const;
    dr4::Vec2f
    getParentAbsPos() const;
    bool
    pointInside( const dr4::Vec2f& point ) const;

    dr4::Texture*
    getTexture();

    const dr4::Texture*
    getTexture() const;

  public:
    virtual void
    RedrawMyTexture() const;

    void
    DrawOnParentTexture() const;

    void
    Redraw() const;

  protected:
    cum::Manager* pm_;

    dr4::Window* window_;

    dr4::Texture* texture_;

    Widget* parent_ = nullptr;

    dr4::Vec2f size_;
    dr4::Vec2f pos_;

    bool is_pressed_ = false;
    bool is_hovered_ = false;

    bool       is_draggable_ = false;
    bool       is_dragging_  = false;
    dr4::Vec2f drag_offset_  = { 0.0f, 0.0f };
};

} // namespace hui
