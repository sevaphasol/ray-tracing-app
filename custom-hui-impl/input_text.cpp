#include <cassert>
#include <cctype>
#include <cmath>
#include <optional>

#include "dr4/texture.hpp"
#include "input_text.hpp"
#include "widget.hpp"

hui::InputText::InputText( hui::WindowManager* wm, float x, float y, float w, float h )
    : Widget( wm, x, y, w, h ),
      rect_( wm->getWindow()->CreateRectangle() ),
      text_( wm->getWindow()->CreateText() ),
      cursor_( wm->getWindow()->CreateRectangle() ),
      selection_rect_( wm->getWindow()->CreateRectangle() )
{
    rect_->SetBorderThickness( RectBorderThickness );
    rect_->SetBorderColor( { 118, 185, 0, 255 } );
    rect_->SetFillColor( { 0, 0, 0, 0 } );

    text_->SetFont( wm->getWindow()->GetDefaultFont() );
    text_->SetColor( { 255, 255, 255 } );
    text_->SetFontSize( 12 );

    cursor_->SetFillColor( { 255, 255, 255 } );

    selection_rect_->SetFillColor( { 50, 100, 200, 100 } );

    updateRect();
}

bool
hui::InputText::onMousePress( const hui::Event& evt )
{
    if ( evt.info.mouseButton.button != dr4::MouseButtonType::LEFT )
    {
        return false;
    }

    if ( !is_hovered_ )
    {
        return false;
    }

    onSelect();

    if ( textContains( evt.info.mouseButton.pos ) )
    {
        handleTextClick( evt.info.mouseButton );
        return true;
    }

    return true;
}

bool
hui::InputText::onMouseRelease( const hui::Event& evt )
{
    if ( evt.info.mouseButton.button != dr4::MouseButtonType::LEFT )
    {
        return false;
    }

    if ( is_selecting_ )
    {
        is_selecting_     = false;
        selection_active_ = ( selection_pos_ != cursor_pos_ );
        return true;
    }

    return false;
}

bool
hui::InputText::onMouseMove( const hui::Event& evt )
{
    if ( is_selecting_ )
    {
        insertCursor( evt.info.mouseMove.pos );
        selection_active_ = ( selection_pos_ != cursor_pos_ );
        updateRect();

        return true;
    }

    return hui::Widget::onMouseMove( evt );
}

bool
hui::InputText::onKeyPress( const hui::Event& evt )
{
    if ( wm_->getFocused() != this )
    {
        return false;
    }

    switch ( evt.info.key.sym )
    {
        case dr4::KEYCODE_BACKSPACE:
            {
                ( ( evt.info.key.mods & dr4::KEYMOD_CTRL ) != 0 ) ? ctrlBackspace() : backspace();
                return true;
            }
        case dr4::KEYCODE_DELETE:
            {
                ( ( evt.info.key.mods & dr4::KEYMOD_CTRL ) != 0 ) ? ctrlDel() : del();
                return true;
            }
        case dr4::KEYCODE_HOME:
            {
                ( ( evt.info.key.mods & dr4::KEYMOD_SHIFT ) != 0 ) ? shiftHome() : home();
                return true;
            }
        case dr4::KEYCODE_END:
            {
                ( ( evt.info.key.mods & dr4::KEYMOD_SHIFT ) != 0 ) ? shiftEnd() : end();
                return true;
            }

        case dr4::KEYCODE_LEFT:
        case dr4::KEYCODE_RIGHT:
            {
                arrows( evt.info.key );
                return true;
            }

        default:
            break;
    }

    if ( ( evt.info.key.mods & dr4::KEYMOD_CTRL ) != 0 )
    {
        switch ( evt.info.key.sym )
        {
            case dr4::KEYCODE_X:
                ctrlX();
                break;
            case dr4::KEYCODE_C:
                ctrlC();
                break;
            case dr4::KEYCODE_V:
                ctrlV();
                break;
            case dr4::KEYCODE_A:
                ctrlA();
                break;
            case dr4::KEYCODE_W:
                ctrlW();
                break;
            case dr4::KEYCODE_S:
                ctrlS();
                break;
            case dr4::KEYCODE_Z:
                ( ( evt.info.key.mods & dr4::KEYMOD_SHIFT ) != 0 ) ? ctrlShiftZ() : ctrlZ();
            default:
                return false;
        }

        return true;
    }

    return false;
}

bool
hui::InputText::onIdle( const hui::Event& evt )
{
    if ( wm_->getFocused() != this )
    {
        cursor_visible_ = false;
        return false;
    }

    double now = wm_->getWindow()->GetTime();
    if ( now - cursor_last_blink_time_ >= cursor_blink_period_ )
    {
        cursor_visible_         = !cursor_visible_;
        cursor_last_blink_time_ = now;
    }

    return false;
}

bool
hui::InputText::onTextEnter( const hui::Event& evt )
{
    if ( wm_->getFocused() != this )
    {
        return false;
    }

    if ( evt.info.text.unicode != nullptr )
    {
        auto ch = *evt.info.text.unicode;

        if ( ( std::iscntrl( ch ) != 0 ) && ch != '\t' )
        {
            return false;
        }

        put( *evt.info.text.unicode );
        return true;
    }

    return false;
}

void
hui::InputText::setColor( dr4::Color color )
{
    text_->SetColor( color );
}

void
hui::InputText::onSelect()
{
    wm_->setFocused( this );
}

void
hui::InputText::onDeselect()
{
    wm_->setFocused( nullptr );
    selection_active_ = false;
    is_selecting_     = false;
}

void
hui::InputText::RedrawMyTexture() const
{
    texture_->Draw( *rect_ );

    if ( selection_active_ )
    {
        texture_->Draw( *selection_rect_ );
    }

    texture_->Draw( *text_ );

    if ( cursor_visible_ )
    {
        texture_->Draw( *cursor_ );
    }
}

void
hui::InputText::updateRect()
{
    dr4::Vec2f baseline = text_->GetPos();

    const dr4::Font* font      = text_->GetFont();
    float            font_size = text_->GetFontSize();

    float asc_up  = std::fabs( 1.3 * font->GetAscent( font_size ) );
    float desc_dn = std::fabs( 1.3 * font->GetDescent( font_size ) );

    float line_height = asc_up + desc_dn;

    dr4::Vec2f bounds     = text_->GetBounds();
    float      text_width = bounds.x;

    // rect_->SetPos( { baseline.x - RectMarginX, baseline.y - RectMarginY } );
    // rect_->SetSize( { text_width + 2.0f * RectMarginX, line_height + 2.0f * RectMarginY } );
    rect_->SetPos( baseline );
    rect_->SetSize( size_ );

    const float cursor_width = 2.0f;
    cursor_->SetSize( { cursor_width, line_height } );
    cursor_->SetPos( { getCursorX(), baseline.y } );

    if ( hasSelection() )
    {
        auto [begin, end] = getSelectionRange();

        tmp_string_ = text_->GetText();

        float start_w = 0.0f;
        if ( begin == 0 )
        {
            start_w = 0.0f;
        } else
        {
            text_->SetText( std::string( tmp_string_.data(), begin ) );
            start_w = text_->GetBounds().x;
        }

        float end_w = 0.0f;
        text_->SetText( std::string( tmp_string_.data(), end ) );
        end_w = text_->GetBounds().x;

        text_->SetText( tmp_string_ );

        float sel_x = baseline.x + start_w;
        float sel_y = baseline.y;
        float sel_w = end_w - start_w;
        float sel_h = line_height;

        selection_rect_->SetPos( { sel_x, sel_y } );
        selection_rect_->SetSize( { sel_w, sel_h } );
    }
}

void
hui::InputText::refreshCursor()
{
    cursor_last_blink_time_ = wm_->getWindow()->GetTime();
    cursor_visible_         = true;
}

void
hui::InputText::put( char c )
{
    ctrlS();

    if ( hasSelection() )
    {
        auto [begin, end] = getSelectionRange();
        string_.erase( begin, end - begin );
        cursor_pos_ = begin;
        clearSelection();
    }

    string_.insert( string_.begin() + cursor_pos_, c );
    ++cursor_pos_;
    text_->SetText( string_ );
    updateRect();
}

void
hui::InputText::home()
{
    cursor_pos_ = 0;
    updateRect();
    refreshCursor();
}

void
hui::InputText::end()
{
    cursor_pos_ = string_.size();
    updateRect();
    refreshCursor();
}

void
hui::InputText::shiftHome()
{
    if ( cursor_pos_ != 0 )
    {
        selection_active_ = true;
        selection_pos_    = cursor_pos_;
        cursor_pos_       = 0;
    }

    updateRect();
    refreshCursor();
}

void
hui::InputText::shiftEnd()
{
    if ( cursor_pos_ != string_.size() )
    {
        selection_active_ = true;
        selection_pos_    = cursor_pos_;
        cursor_pos_       = string_.size();
    }

    updateRect();
    refreshCursor();
}

void
hui::InputText::backspace()
{
    if ( string_.empty() )
    {
        return;
    }

    if ( hasSelection() )
    {
        auto [begin, end] = getSelectionRange();
        string_.erase( begin, end - begin );
        cursor_pos_ = begin;
        clearSelection();
        text_->SetText( string_ );
        updateRect();
        return;
    }

    if ( cursor_pos_ == 0 )
    {
        return;
    }

    string_.erase( string_.begin() + cursor_pos_ - 1 );
    --cursor_pos_;
    text_->SetText( string_ );
    updateRect();
}

void
hui::InputText::del()
{
    if ( string_.empty() )
    {
        return;
    }

    if ( hasSelection() )
    {
        auto [begin, end] = getSelectionRange();
        string_.erase( begin, end - begin );
        cursor_pos_ = begin;
        clearSelection();
        text_->SetText( string_ );
        updateRect();
        return;
    }

    if ( cursor_pos_ == string_.size() )
    {
        return;
    }

    string_.erase( string_.begin() + cursor_pos_ );
    text_->SetText( string_ );
    updateRect();
}

void
hui::InputText::decrementCursor()
{
    if ( cursor_pos_ != 0 )
    {
        --cursor_pos_;
    }

    updateRect();
}

void
hui::InputText::incrementCursor()
{
    if ( cursor_pos_ != string_.size() )
    {
        ++cursor_pos_;
    }

    updateRect();
}

float
hui::InputText::getCursorX() const
{
    dr4::Vec2f baseline = text_->GetPos();

    if ( string_.empty() || cursor_pos_ == 0 )
    {
        return baseline.x;
    }

    if ( cursor_pos_ >= string_.size() )
    {
        return baseline.x + text_->GetBounds().x;
    }

    tmp_string_ = string_;
    text_->SetText( std::string( string_.data(), cursor_pos_ ) );
    float prefix_w = text_->GetBounds().x;
    text_->SetText( tmp_string_ );

    return baseline.x + prefix_w;
}

void
hui::InputText::insertCursor( dr4::Vec2f pos )
{
    dr4::Vec2f text_pos = text_->GetPos();

    float local_x = pos.x - text_pos.x;

    if ( local_x <= 0.0f || string_.empty() )
    {
        cursor_pos_ = 0;
        return;
    }

    tmp_string_ = text_->GetText();

    size_t best_index = string_.size();
    float  prev_w     = 0.0f;
    float  cur_w      = 0.0f;

    for ( size_t i = 1; i <= string_.size(); ++i )
    {
        text_->SetText( std::string( string_.data(), i ) );
        cur_w = text_->GetBounds().x;

        if ( cur_w >= local_x )
        {
            float mid = ( prev_w + cur_w ) * 0.5f;
            if ( local_x < mid )
            {
                best_index = i - 1;
            } else
            {
                best_index = i;
            }
            break;
        }

        prev_w = cur_w;
    }

    text_->SetText( tmp_string_ );

    cursor_pos_ = best_index;

    updateRect();
}

std::pair<size_t, size_t>
hui::InputText::getSelectionRange() const
{
    if ( !selection_active_ )
    {
        return { 0, 0 };
    }

    return {
        std::min( cursor_pos_, selection_pos_ ),
        std::max( cursor_pos_, selection_pos_ ),
    };
}

bool
hui::InputText::hasSelection() const
{
    return selection_active_ && ( selection_pos_ != cursor_pos_ );
}

void
hui::InputText::clearSelection()
{
    selection_active_ = false;
    selection_pos_    = cursor_pos_;
}

std::string
hui::InputText::copySelected() const
{
    if ( !hasSelection() )
    {
        return "";
    }

    auto [begin, end] = getSelectionRange();

    return string_.substr( begin, end - begin );
}

void
hui::InputText::pasteFromClipboard()
{
    ctrlS();

    const std::string& clip = wm_->getWindow()->GetClipboard();

    if ( hasSelection() )
    {
        auto [begin, end] = getSelectionRange();
        string_.erase( begin, end - begin );
        cursor_pos_ = begin;
        clearSelection();
    }

    string_.insert( cursor_pos_, clip );
    cursor_pos_ += clip.size();
    text_->SetText( string_ );
    updateRect();
}

void
hui::InputText::eraseSelected()
{
    auto [begin, end] = getSelectionRange();
    string_.erase( begin, end - begin );
    cursor_pos_ = begin;
    clearSelection();
    text_->SetText( string_ );
    updateRect();
}

size_t
hui::InputText::getLeftWordPos()
{
    size_t pos = cursor_pos_;

    if ( pos == 0 )
    {
        return pos;
    }

    while ( pos > 0 && ( std::isspace( string_[pos - 1] ) != 0 ) )
    {
        --pos;
    }

    while ( pos > 0 && ( std::isspace( string_[pos - 1] ) == 0 ) )
    {
        --pos;
    }

    return pos;
}

size_t
hui::InputText::getRightWordPos()
{
    size_t pos = cursor_pos_;

    size_t n = string_.size();
    if ( pos >= n )
    {
        return pos;
    }

    while ( pos < n && ( std::isspace( string_[pos] ) != 0 ) )
    {
        ++pos;
    }

    while ( pos < n && ( std::isspace( string_[pos] ) == 0 ) )
    {
        ++pos;
    }

    return pos;
}

void
hui::InputText::moveCursorWordLeft()
{
    cursor_pos_ = getLeftWordPos();
    updateRect();
}

void
hui::InputText::moveCursorWordRight()
{
    cursor_pos_ = getRightWordPos();
    updateRect();
}

void
hui::InputText::ctrlBackspace()
{
    ctrlS();

    size_t start_pos = getLeftWordPos();
    if ( start_pos < cursor_pos_ )
    {
        size_t length = cursor_pos_ - start_pos;
        string_.erase( start_pos, length );
        cursor_pos_ = start_pos;
        clearSelection();
        text_->SetText( string_ );
        updateRect();
    }
}

void
hui::InputText::ctrlDel()
{
    ctrlS();

    size_t end_pos = getRightWordPos();
    if ( end_pos > cursor_pos_ )
    {
        size_t length = end_pos - cursor_pos_;
        string_.erase( cursor_pos_, length );
        clearSelection();
        text_->SetText( string_ );
        updateRect();
    }
}

void
hui::InputText::ctrlX()
{
    ctrlS();

    wm_->getWindow()->SetClipboard( copySelected() );
    eraseSelected();
    refreshCursor();
}

void
hui::InputText::ctrlC()
{
    wm_->getWindow()->SetClipboard( copySelected() );
    refreshCursor();
}

void
hui::InputText::ctrlV()
{
    pasteFromClipboard();
    refreshCursor();
}

void
hui::InputText::ctrlA()
{
    selection_active_ = true;
    selection_pos_    = 0;
    cursor_pos_       = string_.size();
    refreshCursor();
    updateRect();
}

void
hui::InputText::ctrlW()
{
    ctrlBackspace();
    refreshCursor();
    updateRect();
}

void
hui::InputText::arrows( dr4::Event::KeyEvent evt )
{
    size_t old_pos = cursor_pos_;

    bool shift = ( evt.mods & dr4::KEYMOD_SHIFT ) != 0;

    if ( shift )
    {
        if ( !selection_active_ )
        {
            selection_pos_    = old_pos;
            selection_active_ = true;
        }
    } else
    {
        clearSelection();
    }

    if ( ( evt.mods & dr4::KEYMOD_CTRL ) != 0 )
    {
        ( evt.sym == dr4::KEYCODE_LEFT ) ? moveCursorWordLeft() : moveCursorWordRight();
    } else
    {
        ( evt.sym == dr4::KEYCODE_LEFT ) ? decrementCursor() : incrementCursor();
    }

    if ( selection_active_ && selection_pos_ == cursor_pos_ )
    {
        clearSelection();
    }

    refreshCursor();
}

void
hui::InputText::ctrlS()
{
    undo_stack_.push( { string_, cursor_pos_ } );
    while ( !redo_stack_.empty() )
    {
        redo_stack_.pop();
    }
}

void
hui::InputText::ctrlZ()
{
    if ( undo_stack_.empty() )
    {
        return;
    }

    redo_stack_.push( { string_, cursor_pos_ } );
    auto [prev_str, prev_cursor] = undo_stack_.top();
    undo_stack_.pop();

    string_     = prev_str;
    cursor_pos_ = prev_cursor;
    clearSelection();
    text_->SetText( string_ );
    updateRect();
    refreshCursor();
}

void
hui::InputText::ctrlShiftZ()
{
    if ( redo_stack_.empty() )
    {
        return;
    }

    undo_stack_.push( { string_, cursor_pos_ } );
    auto [next_str, next_cursor] = redo_stack_.top();
    redo_stack_.pop();

    string_     = next_str;
    cursor_pos_ = next_cursor;
    clearSelection();
    text_->SetText( string_ );
    updateRect();
    refreshCursor();
}

void
hui::InputText::selectWordAtCursor()
{
    size_t n = string_.size();
    if ( n == 0 )
    {
        clearSelection();
        return;
    }

    size_t pos = cursor_pos_;
    if ( pos == n )
    {
        pos = n - 1;
    }

    size_t l = pos;
    size_t r = pos;

    int want_space = std::isspace( string_[pos] );

    while ( l > 0 && ( std::isspace( ( string_[l - 1] ) ) == want_space ) )
    {
        --l;
    }

    while ( r < n && ( std::isspace( ( string_[r] ) ) == want_space ) )
    {
        ++r;
    }

    selection_pos_ = l;
    cursor_pos_    = r;

    selection_active_ = ( selection_pos_ != cursor_pos_ );
    text_->SetText( string_ );
    updateRect();
}

void
hui::InputText::selectString()
{
    selection_pos_    = 0;
    cursor_pos_       = string_.size();
    selection_active_ = ( cursor_pos_ != selection_pos_ );
    text_->SetText( string_ );
    updateRect();
}

bool
hui::InputText::textContains( dr4::Vec2f rel ) const
{
    dr4::Vec2f rect_size = rect_->GetSize() - dr4::Vec2f( RectMarginX, RectMarginY );
    dr4::Vec2f rect_pos  = rect_->GetPos() + dr4::Vec2f( RectMarginX, RectMarginY );

    if ( rect_size.x < 0 )
    {
        rect_size.x = -rect_size.x;
        rect_pos.x -= rect_size.x;
    }

    if ( rect_size.y < 0 )
    {
        rect_size.y = -rect_size.y;
        rect_pos.y -= rect_size.y;
    }

    return dr4::Rect2f( rect_pos, rect_size ).Contains( rel );
}

void
hui::InputText::handleTextClick( const dr4::Event::MouseButton& evt )
{
    std::cerr << __func__ << std::endl;

    double now     = wm_->getWindow()->GetTime();
    float  dx      = evt.pos.x - last_click_pos_.x;
    float  dy      = evt.pos.y - last_click_pos_.y;
    float  dist_sq = dx * dx + dy * dy;

    if ( ( now - last_click_time_ ) <= DoubleClickTime &&
         dist_sq <= ( ClickMoveThreshold * ClickMoveThreshold ) )
    {
        ++click_count_;
    } else
    {
        click_count_ = 1;
    }

    last_click_time_ = now;
    last_click_pos_  = evt.pos;

    switch ( click_count_ )
    {
        case 1:
            std::cerr << "1 click count_" << std::endl;

            insertCursor( evt.pos );
            clearSelection();
            is_selecting_ = true;
            break;

        case 2:
            insertCursor( evt.pos );
            selectWordAtCursor();
            is_selecting_ = true;
            break;

        case 3:
            selectString();
            is_selecting_    = true;
            click_count_     = 0;
            last_click_time_ = 0.0;
            break;

        default:
            break;
    }

    refreshCursor();
    updateRect();
}

bool
hui::InputText::OnKeyDownReadOnly( const dr4::Event::KeyEvent& evt )
{
    if ( ( evt.mods & dr4::KEYMOD_CTRL ) != 0 )
    {
        if ( evt.sym == dr4::KEYCODE_C )
        {
            wm_->getWindow()->SetClipboard( copySelected() );
            return true;
        }
        if ( evt.sym == dr4::KEYCODE_A )
        {
            selection_active_ = true;
            selection_pos_    = 0;
            cursor_pos_       = string_.size();
            updateRect();
            return true;
        }
    }

    return false;
}

void
hui::InputText::setString( const std::string& str )
{
    string_ = str;
    text_->SetText( string_ );
    updateRect();
}

std::optional<std::string_view>
hui::InputText::getString()
{
    if ( string_.empty() )
    {
        return std::nullopt;
    }

    return string_;
}

std::optional<double>
hui::InputText::getDouble()
try
{
    return std::stod( string_ );
} catch ( ... )
{
    return std::nullopt;
}

std::optional<uint32_t>
hui::InputText::getUint32()
try
{
    return std::stoul( string_ );
} catch ( ... )
{
    return std::nullopt;
}
