#pragma once

#include "container_widget.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/window.hpp"
#include "widget.hpp"
#include <memory>

namespace hui {

namespace detail {

struct ScrollBar
{
    static inline const dr4::Vec2f Size = { 15.0f, 110.0f };

    struct ArrowField
    {
        static constexpr float SizeCoef = 0.1;

        static inline const dr4::Vec2f Size = { ScrollBar::Size.x, ScrollBar::Size.y* SizeCoef };

        struct Color
        {
            static const inline dr4::Color Default = { 96 + 32, 96 + 32, 96 + 32, 255 };
            static const inline dr4::Color Hover   = { 64 + 32, 64 + 32, 64 + 32, 255 };
            static const inline dr4::Color Pressed = { 32 + 32, 32 + 32, 32 + 32, 255 };
        };

        struct Triangle
        {
            struct Color
            {
                static const inline dr4::Color Default = { 96, 96, 96, 255 };
                static const inline dr4::Color Hover   = { 64, 64, 64, 255 };
                static const inline dr4::Color Pressed = { 32, 32, 32, 255 };
            };

            struct Up
            {
                static inline const dr4::Vec2f Pos = { 0.0f, 0.0f };

                static inline const dr4::Vec2f Triangle[] = {
                    dr4::Vec2f( ScrollBar::ArrowField::Size.x / 2,
                                ScrollBar::ArrowField::Size.y / 3 ),
                    dr4::Vec2f( ScrollBar::ArrowField::Size.x / 3,
                                2 * ScrollBar::ArrowField::Size.y / 3 ),
                    dr4::Vec2f( 2 * ScrollBar::ArrowField::Size.x / 3,
                                2 * ScrollBar::ArrowField::Size.y / 3 ) };
            };

            struct Down
            {
                static inline const dr4::Vec2f Pos = { 0.0f, ScrollBar::Size.y * ( 1 - SizeCoef ) };

                static inline const dr4::Vec2f Triangle[] = {
                    dr4::Vec2f( ScrollBar::ArrowField::Size.x / 2,
                                2 * ScrollBar::ArrowField::Size.y / 3 ),
                    dr4::Vec2f( ScrollBar::ArrowField::Size.x / 3,
                                ScrollBar::ArrowField::Size.y / 3 ),
                    dr4::Vec2f( 2 * ScrollBar::ArrowField::Size.x / 3,
                                ScrollBar::ArrowField::Size.y / 3 ) };
            };
        };
    };

    struct Thumb
    {
        static constexpr float SizeCoef = 0.3;

        struct Color
        {
            static const inline dr4::Color Default = { 48 + 32, 48 + 32, 48 + 32, 170 };
            static const inline dr4::Color Hover   = { 64 + 32, 64 + 32, 64 + 32, 230 };
            static const inline dr4::Color Pressed = { 32 + 32, 32 + 32, 32 + 32, 175 };
        };

        static inline const dr4::Vec2f Size = { ScrollBar::Size.x, ScrollBar::Size.y* SizeCoef };

        static inline const dr4::Vec2f StartPos = { 0.0f, ArrowField::Size.y };
    };
};

} // namespace detail

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
    ScrollBar( hui::WindowManager* wm,
               const dr4::Vec2f&   pos,
               dr4::Vec2f          size = detail::ScrollBar::Size );
    ScrollBar( hui::WindowManager* wm,
               float               x,
               float               y,
               float               w = detail::ScrollBar::Size.x,
               float               h = detail::ScrollBar::Size.y );

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

  private:
    void
    updateThumbPosition();

  private:
    bool   is_scrolled_   = false;
    double scroll_factor_ = 0.0;

    std::unique_ptr<dr4::Rectangle> border_;

    Thumb thumb_;
    Arrow up_arrow_;
    Arrow down_arrow_;
};

} // namespace hui
