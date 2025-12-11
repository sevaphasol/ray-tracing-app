#pragma once

#include "dr4/math/vec2.hpp"
#include "widget.hpp"
#include <memory>
#include <string>

namespace hui {

class LabelWidget : public Widget {
  public:
    // pos and size are relative to parent/content (ScrollableListWidget will set rel pos)
    LabelWidget( hui::WindowManager* wm,
                 const dr4::Vec2f&   pos,
                 const dr4::Vec2f&   size,
                 const std::string&  text,
                 unsigned int        font_size = 13 );

    void
    setText( const std::string& text );
    void
    RedrawMyTexture() const override;

  private:
    std::unique_ptr<dr4::Text> text_obj_;
    std::string                text_;
    unsigned int               font_size_;
};
} // namespace hui
