#pragma once

#include "container_widget.hpp"
#include "dr4/math/color.hpp"
#include "dr4/window.hpp"
#include "widget.hpp"
#include <memory>

namespace hui {

class WindowManager {
  public:
    WindowManager( cum::Manager* pm, dr4::Window* win, const dr4::Color& color );
    ~WindowManager() = default;

    void
    run();

    void
    setFramerateLimit( float fps_limit );

    void
    setDeltaTime( float delta_time );

    void
    setBackgroundColor( const dr4::Color& color );

    void
    addWidget( std::unique_ptr<hui::Widget> widget );

  private:
    void
    handleEvents();
    void
    draw();

  private:
    std::unique_ptr<dr4::Window> window_;
    hui::VectorContainerWidget   desktop_;
    dr4::Color                   background_color_;
    float                        delta_time_ = 0.0f;
};

} // namespace hui
