#pragma once

#include "button.hpp"
#include "dialog_box.hpp"
#include <functional>
#include <memory>
#include <string>

namespace hui {

class MessageBox : public hui::DialogBox {
  public:
    MessageBox( hui::WindowManager*   wm,
                float                 x,
                float                 y,
                float                 w,
                float                 h,
                const std::string&    title,
                const std::string&    message,
                std::function<void()> on_close = nullptr );

    bool
    propagateEventToChildren( const hui::Event& event ) override;

    void
    RedrawMyTexture() const override;

  private:
    std::unique_ptr<dr4::Text> text_;
    hui::Button                ok_;
    std::function<void()>      on_close_;
};

} // namespace hui
