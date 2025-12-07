#pragma once

#include <memory>
#include <stack>
#include <string_view>

#include "dr4/event.hpp"
#include "dr4/math/color.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"

#include "window_manager.hpp"

namespace hui {

class InputText : public Widget {
  public:
    explicit InputText( hui::WindowManager* wm, float x, float y, float w, float h );
    explicit InputText( hui::WindowManager* wm, const dr4::Vec2f& pos, const dr4::Vec2f& size )
        : InputText( wm, pos.x, pos.y, size.x, size.y )
    {
    }

    bool
    onIdle( const hui::Event& evt ) override;
    bool
    onTextEnter( const hui::Event& evt ) override;
    bool
    onKeyPress( const hui::Event& evt ) override;
    bool
    onMousePress( const hui::Event& evt ) override;
    bool
    onMouseRelease( const hui::Event& evt ) override;
    bool
    onMouseMove( const hui::Event& evt ) override;

    void
    setColor( dr4::Color color );

    void
    setString( const std::string& str );
    std::optional<std::string_view>
    getString();
    std::optional<double>
    getDouble();
    std::optional<uint32_t>
    getUint32();

    void
    RedrawMyTexture() const override;

    void
    SetIsDrawing( bool state );

  private:
    void
    onSelect();
    void
    onDeselect();

    void
    refreshCursor();
    bool
    textContains( dr4::Vec2f rel ) const;

    void
    put( char c );

    void
    arrows( dr4::Event::KeyEvent evt );

    void
    home();
    void
    end();
    void
    shiftHome();
    void
    shiftEnd();

    void
    backspace();
    void
    ctrlBackspace();
    void
    del();
    void
    ctrlDel();

    void
    ctrlX();
    void
    ctrlC();
    void
    ctrlV();
    void
    ctrlA();
    void
    ctrlW();
    void
    ctrlS();
    void
    ctrlZ();
    void
    ctrlShiftZ();

    void
    decrementCursor();
    void
    incrementCursor();
    void
    moveCursorWordLeft();
    void
    moveCursorWordRight();

    void
    updateRect();

    void
    insertCursor( dr4::Vec2f pos );
    float
    getCursorX() const;

    std::pair<size_t, size_t>
    getSelectionRange() const;

    std::string
    copySelected() const;
    void
    pasteFromClipboard();
    void
    eraseSelected();

    size_t
    getLeftWordPos();
    size_t
    getRightWordPos();

    bool
    hasSelection() const;
    void
    clearSelection();

    void
    selectWordAtCursor();
    void
    selectString();

    void
    handleTextClick( const dr4::Event::MouseButton& evt );

    bool
    OnKeyDownReadOnly( const dr4::Event::KeyEvent& evt );

  private:
    static constexpr float RectBorderThickness = -2.0f;
    static constexpr float RectMarginX         = 10.0f;
    static constexpr float RectMarginY         = 10.0f;

    const std::string empty_str_;

    std::string         string_;
    mutable std::string tmp_string_;

    std::unique_ptr<dr4::Text>      text_;
    std::unique_ptr<dr4::Rectangle> rect_;

    std::unique_ptr<dr4::Rectangle> cursor_;

    double cursor_blink_period_    = 0.5;
    double cursor_last_blink_time_ = 0.0;
    bool   cursor_visible_         = false;
    size_t cursor_pos_             = 0;

    size_t                          selection_pos_    = 0;
    bool                            selection_active_ = false;
    bool                            is_selecting_     = false;
    std::unique_ptr<dr4::Rectangle> selection_rect_;

    bool is_drawing_ = false;

    float ascent_;
    float descent_;

    std::stack<std::tuple<std::string, size_t>> undo_stack_;
    std::stack<std::tuple<std::string, size_t>> redo_stack_;

    double     last_click_time_ = 0.0;
    dr4::Vec2f last_click_pos_  = { 0, 0 };
    size_t     click_count_     = 0;

    static constexpr double DoubleClickTime    = 0.45;
    static constexpr float  ClickMoveThreshold = 6.0f;
};

} // namespace hui
