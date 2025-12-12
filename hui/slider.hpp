#pragma once

#include "dr4/math/color.hpp"
#include "dr4/math/vec2.hpp"
#include "widget.hpp"
#include <functional>

namespace hui {

class Slider : public hui::Widget {
  public:
    struct Theme
    {
        dr4::Color track_color;
        dr4::Color fill_color;
        dr4::Color handle_color;
        dr4::Color handle_hover_color;
        float      handle_width;
        float      track_height;
    };

    static const inline Theme DefaultTheme{ { 45, 45, 45, 220 },
                                            { 100, 150, 0, 220 },
                                            { 220, 220, 220, 255 },
                                            { 255, 255, 255, 255 },
                                            10.0f,
                                            6.0f };

    using ChangeCallback = std::function<void( float )>;

    Slider( hui::WindowManager* wm,
            const dr4::Vec2f&   pos,
            const dr4::Vec2f&   size,
            const Theme&        theme = DefaultTheme );

    float
    getFactor() const;
    void
    setFactor( float t );

    void
    setOnChange( ChangeCallback cb );

    void
    setSize( const dr4::Vec2f& size ) override;
    void
    setRelPos( const dr4::Vec2f& pos ) override;

    bool
    onMousePress( const Event& event ) override;
    bool
    onMouseMove( const Event& event ) override;
    bool
    onMouseRelease( const Event& event ) override;

  private:
    void
    RedrawMyTexture() const override;

    float
    clamp01( float v ) const;
    void
    updateFromPos( float mouse_x );

  private:
    float                           factor_ = 0.0f; // [0,1]
    Theme                           theme_;
    ChangeCallback                  on_change_;
    bool                            dragging_ = false;
    std::unique_ptr<dr4::Rectangle> track_;
    std::unique_ptr<dr4::Rectangle> fill_;
    std::unique_ptr<dr4::Rectangle> handle_;
};

} // namespace hui
