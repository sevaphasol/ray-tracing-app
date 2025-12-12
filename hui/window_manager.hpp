#pragma once
#include "container_widget.hpp"
#include "cum/manager.hpp"
#include "dr4/math/color.hpp"
#include "dr4/window.hpp"
#include "widget.hpp"
#include <memory>
#include <vector>

namespace hui {

class WindowManager {
  public:
    WindowManager( cum::Manager* pm, dr4::Window* win );
    ~WindowManager() = default;
    void
    run();
    void
    setBackgroundColor( const dr4::Color& color );
    void
    addWidget( std::unique_ptr<hui::Widget> widget );
    void
    pushModal( std::unique_ptr<hui::Widget> wgt );
    void
    popModal();
    hui::Widget*
    getTopModal() const;
    dr4::Window*
    getWindow()
    {
        return win_.get();
    }
    cum::Manager*
    getPluginManager()
    {
        return pm_;
    }
    void
    setFocused( Widget* wgt )
    {
        focused_wgt_ = wgt;
    }
    void
    setCaptured( Widget* wgt )
    {
        captured_wgt_ = wgt;
    }
    void
    setHovered( Widget* wgt )
    {
        hovered_wgt_ = wgt;
    }
    Widget*
    getFocused()
    {
        return focused_wgt_;
    }
    Widget*
    getCaptured()
    {
        return captured_wgt_;
    }
    Widget*
    getHovered()
    {
        return hovered_wgt_;
    }

  private:
    void
    handleEvents();
    void
    draw();

  private:
    Widget*                                   focused_wgt_  = nullptr;
    Widget*                                   captured_wgt_ = nullptr;
    Widget*                                   hovered_wgt_  = nullptr;
    std::vector<std::unique_ptr<hui::Widget>> modal_widgets_;
    std::unique_ptr<dr4::Window>              win_;
    cum::Manager*                             pm_ = nullptr;
    hui::VectorContainerWidget                desktop_;
    dr4::Color                                background_color_;
    float                                     delta_time_ = 0.0f;
};

} // namespace hui
