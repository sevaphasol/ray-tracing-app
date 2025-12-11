#pragma once

#include "dr4/math/color.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include "widget.hpp"
#include <functional>
#include <memory>

namespace hui {

class Button : public hui::Widget {
  public:
    struct Theme
    {
        dr4::Color default_color;
        dr4::Color hovered_color;
        dr4::Color pressed_color;
        dr4::Color font_color;
        size_t     font_size;
    };

    static const inline Theme DefaultTheme = { { 30, 30, 30 },
                                               { 50, 70, 30 },
                                               { 100, 150, 0 },
                                               { 255, 255, 255 },
                                               15 };

    using HoldPressCb = std::function<void()>;
    using ClickCb     = std::function<void()>;

  private:
    std::unique_ptr<dr4::Rectangle> background_;
    std::unique_ptr<dr4::Text>      label_;

    ClickCb     on_click_;
    HoldPressCb on_hold_press_;

    Theme theme_;

  public:
    explicit Button( hui::WindowManager* wm,
                     const dr4::Vec2f&   pos,
                     const dr4::Vec2f&   size,
                     const std::string&  title,
                     const Theme&        theme = DefaultTheme );

    const Theme&
    getTheme() const;
    void
    setTheme( const Theme& theme );
    void
    setRelPos( const dr4::Vec2f& pos ) override;
    void
    setSize( const dr4::Vec2f& size ) override;

    void
    setLabelText( const std::string& text );
    void
    setFont( const dr4::Font* font );
    void
    setBackgroundColor( const dr4::Color& color );

    bool
    isPressed() const;

    void
    setOnHoldPress( HoldPressCb callback );
    void
    setOnClick( ClickCb callback );

    bool
    onMousePress( const Event& event ) override;
    bool
    onMouseMove( const Event& event ) override;
    bool
    onMouseRelease( const Event& event ) override;
    bool
    onIdle( const Event& event ) override;

  private:
    void
    RedrawMyTexture() const override;
};

} // namespace hui
