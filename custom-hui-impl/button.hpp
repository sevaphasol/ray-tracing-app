// #pragma once
//
// #include "dr4/math/color.hpp"
// #include "dr4/math/vec2.hpp"
// #include "dr4/texture.hpp"
// #include "widget.hpp"
// #include <memory>
//
// namespace hui {
//
// class Button : public hui::Widget {
//   public:
//     explicit Button( cum::Manager*      pm,
//                      dr4::Window*       win,
//                      const dr4::Vec2f&  pos,
//                      const dr4::Vec2f&  size,
//                      const dr4::Color&  default_color,
//                      const dr4::Color&  hovered_color,
//                      const dr4::Color&  pressed_color,
//                      const std::string& title,
//                      const dr4::Color&  font_color,
//                      size_t             font_size );
//
//     void
//     setRelPos( const dr4::Vec2f& pos ) override;
//     void
//     setSize( const dr4::Vec2f& size ) override;
//
//     void
//     setLabelText( const std::string& text );
//     void
//     setLabelFont( const dr4::Font* font, size_t size );
//     void
//     setBackgroundColor( const dr4::Color& color );
//     bool
//     isPressed() const;
//     bool
//     isPressedJustNow() const;
//     bool
//     onMousePress( const Event& event ) override;
//
//     bool
//     onIdle( const Event& event ) override;
//
//   private:
//     // bool
//     // onMousePress( const Event& event ) override;
//     // bool
//     // onMouseRelease( const Event& event ) override;
//     // bool
//     // onMouseMove( const Event& event ) override;
//     void
//     RedrawMyTexture() const override;
//     void
//     updateVisuals();
//
//     bool is_pressed_just_now_ = false;
//
//     std::unique_ptr<dr4::Rectangle> background_;
//     const dr4::Font*                font_;
//     std::unique_ptr<dr4::Text>      label_;
//     dr4::Color                      default_color_;
//     dr4::Color                      hovered_color_;
//     dr4::Color                      pressed_color_;
// };
//
// } // namespace hui

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
    using ClickCallback = std::function<void()>;

    explicit Button( hui::WindowManager* wm,
                     const dr4::Vec2f&   pos,
                     const dr4::Vec2f&   size,
                     const dr4::Color&   default_color,
                     const dr4::Color&   hovered_color,
                     const dr4::Color&   pressed_color,
                     const std::string&  title,
                     const dr4::Color&   font_color,
                     size_t              font_size );

    void
    setRelPos( const dr4::Vec2f& pos ) override;
    void
    setSize( const dr4::Vec2f& size ) override;

    void
    setLabelText( const std::string& text );
    void
    setLabelFont( const dr4::Font* font, size_t size );
    void
    setBackgroundColor( const dr4::Color& color );

    bool
    isPressed() const;
    bool
    isPressedJustNow() const;

    void
    setOnClick( ClickCallback callback );

    bool
    onMousePress( const Event& event ) override;
    bool
    onMouseRelease( const Event& event ) override;
    bool
    onIdle( const Event& event ) override;

  private:
    void
    RedrawMyTexture() const override;
    void
    updateVisuals();

    bool is_pressed_just_now_ = false;

    std::unique_ptr<dr4::Rectangle> background_;
    const dr4::Font*                font_;
    std::unique_ptr<dr4::Text>      label_;

    dr4::Color default_color_;
    dr4::Color hovered_color_;
    dr4::Color pressed_color_;

    ClickCallback on_click_;
};

} // namespace hui
