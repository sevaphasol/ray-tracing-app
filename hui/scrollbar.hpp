#pragma once

#include "container_widget.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/window.hpp"
#include "widget.hpp"
#include <memory>

namespace hui {

class ScrollBar;

class Thumb : public hui::Widget {
  public:
    Thumb( hui::WindowManager* wm,
           ScrollBar*          owner,
           const dr4::Vec2f&   pos,
           const dr4::Vec2f&   size );

    bool
    onIdle( const Event& event ) override;
    bool
    onMousePress( const Event& event ) override;
    bool
    onMouseMove( const Event& event ) override;
    bool
    onMouseRelease( const Event& event ) override;

    void
    RedrawMyTexture() const override;

  private:
    void
    updateVisuals();

  private:
    ScrollBar* owner_;

    std::unique_ptr<dr4::Rectangle> rect_;
};

class Arrow : public hui::Widget {
  public:
    Arrow( hui::WindowManager* wm,
           ScrollBar*          owner,
           const dr4::Vec2f&   pos,
           const dr4::Vec2f&   size,
           bool                is_up );

    bool
    onIdle( const Event& event ) override;
    bool
    onMousePress( const Event& event ) override;
    bool
    onMouseMove( const Event& event ) override;
    bool
    onMouseRelease( const Event& event ) override;

    void
    setUpTriangle();

    void
    RedrawMyTexture() const override;

  private:
    void
    updateTriangleColor( const dr4::Color& color );
    void
    updateTrianglePosition();
    void
    updateVisuals();

  private:
    ScrollBar* owner_;

    bool is_up_;

    std::unique_ptr<dr4::Rectangle> rect_;
    // dr4::Vertex    triangle_[3];
};

class ScrollBar : public hui::ContainerWidget {
  public:
    struct Theme
    {
        struct ColorSet
        {
            dr4::Color default_color;
            dr4::Color hover_color;
            dr4::Color pressed_color;
        };

        ColorSet   thumb;
        ColorSet   arrow_field;
        ColorSet   arrow_triangle;
        dr4::Color border_fill;
        dr4::Color border_line;

        Theme()
            : thumb{ { 48 + 32, 48 + 32, 48 + 32, 170 },
                     { 64 + 32, 64 + 32, 64 + 32, 230 },
                     { 32 + 32, 32 + 32, 32 + 32, 175 } },
              arrow_field{ { 96 + 32, 96 + 32, 96 + 32, 255 },
                           { 64 + 32, 64 + 32, 64 + 32, 255 },
                           { 32 + 32, 32 + 32, 32 + 32, 255 } },
              arrow_triangle{ { 96, 96, 96, 255 },
                              { 64, 64, 64, 255 },
                              { 32, 32, 32, 255 } },
              border_fill( { 48, 48, 48, 223 } ),
              border_line( { 32, 32, 32, 255 } )
        {
        }
    };

    static const inline Theme DefaultTheme{};

    ScrollBar( hui::WindowManager* wm,
               const dr4::Vec2f&   pos,
               dr4::Vec2f          size = { DefaultWidth, DefaultHeight },
               const Theme&        theme = DefaultTheme );
    ScrollBar( hui::WindowManager* wm,
               float               x,
               float               y,
               float               w = DefaultWidth,
               float               h = DefaultHeight,
               const Theme&        theme = DefaultTheme );

    void
    bringToFront( Widget* child ) override;

    bool
    propagateEventToChildren( const Event& event ) override final;

    dr4::Vec2f
    clampThumbPosition( const dr4::Vec2f& vector );

    void
    onThumbMove( float vertical_delta );
    void
    onArrowClick( bool is_up );

    void
    RedrawMyTexture() const override;

    double
    getScrollFactor() const;
    void
    setScrollFactor( double factor );

    bool
    isScrolled();

    const Theme&
    theme() const
    {
        return theme_;
    }

  private:
    void
    updateThumbPosition();

  private:
    static constexpr float DefaultWidth      = 15.0f;
    static constexpr float DefaultHeight     = 110.0f;
    static constexpr float ArrowHeightFactor = 0.1f;
    static constexpr float ThumbHeightFactor = 0.3f;

    bool   is_scrolled_   = false;
    double scroll_factor_ = 0.0;
    Theme  theme_;

    std::unique_ptr<dr4::Rectangle> border_;

    Thumb thumb_;
    Arrow up_arrow_;
    Arrow down_arrow_;
};

} // namespace hui
