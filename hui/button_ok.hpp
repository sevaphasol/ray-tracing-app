#pragma once

#include "button.hpp"
#include "dr4/math/color.hpp"
#include "dr4/math/vec2.hpp"
#include <string>

namespace hui {

class ButtonOk : public hui::Button {
  public:
    ButtonOk( hui::WindowManager* wm, const dr4::Vec2f& pos, const dr4::Vec2f& size )
        : Button( wm,
                  pos,
                  size,
                  "OK",
                  hui::Button::Theme{ Colors::Default, Colors::Hover, Colors::Pressed, Colors::Font, 12 } )
    {
    }

  private:
    struct Colors
    {
        static inline const dr4::Color Default = { 32, 128, 32, 255 };
        static inline const dr4::Color Hover   = { 48, 148, 48, 255 };
        static inline const dr4::Color Pressed = { 24, 96, 24, 255 };
        static inline const dr4::Color Font    = { 255, 255, 255, 255 };
    };
};

} // namespace hui
