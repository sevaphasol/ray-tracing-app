#pragma once

#include "container_widget.hpp"
#include "widget.hpp"

#include <cmath>
#include <memory>
#include <optional>
#include <stdexcept>

namespace hui {

class Cursor : public hui::Widget {
  public:
    Cursor( cum::PluginManager* pm, float w, float h )
        : hui::Widget( pm, 0.0f, 0.0f, w, h ), color_( { 255, 255, 255, 255 } )
    {
        rect_.reset( pm->getWindow()->CreateRectangle() );

        rect_->SetFillColor( color_ );
        rect_->SetSize( { w, h } );
    }

    void
    move( float x )
    {
        setRelPos( { getRelPos().x + x, getRelPos().y } );
    }

    bool
    onIdle( const Event& event ) override
    {
        // TODO blinking logic
        return false;
    }

    void
    setColor( const dr4::Color& color )
    {
        rect_->SetFillColor( color );
    }

    void
    RedrawMyTexture() const override final
    {
        texture_->Draw( *rect_ );
    }

  private:
    dr4::Color                      color_;
    std::unique_ptr<dr4::Rectangle> rect_;
};

class TextField : public hui::ContainerWidget {
  public:
    TextField( cum::PluginManager* pm,
               const std::string&  label,
               const dr4::Font*    font,
               float               x,
               float               y,
               float               w,
               float               h )
        : ContainerWidget( pm, x, y, w, h ), cursor_( pm, 1.0f, h * 0.8 )
    {
        cursor_.setParent( this );

        border_.reset( pm->getWindow()->CreateRectangle() );
        label_.reset( pm->getWindow()->CreateText() );
        text_.reset( pm->getWindow()->CreateText() );

        label_->SetFontSize( 12 );
        label_->SetColor( { 255, 255, 255, 255 } );
        label_->SetFont( font );
        label_->SetText( label );

        text_->SetFontSize( 12 );
        text_->SetColor( { 255, 255, 255, 255 } );
        text_->SetFont( font );
        text_->SetPos( { 1.8f * label_->GetText().length() * 6.5f, h * 0.1f } );

        border_->SetFillColor( { 0, 0, 0, 0 } );
        border_->SetBorderColor( { 118, 185, 0, 255 } );
        border_->SetBorderThickness( -1.0f );
        border_->SetSize( { w - 1.05f * label_->GetText().length() * 12.0f, h } );
        border_->SetPos( { 1.75f * label_->GetText().length() * 6.5f, 0 } );

        cursor_.setRelPos( 1.9f * label_->GetText().length() * 7.0f, h * 0.1f );
    }

    TextField( cum::PluginManager* pm,
               const std::string&  label,
               const dr4::Font*    font,
               const dr4::Vec2f&   pos,
               const dr4::Vec2f&   size )
        : TextField( pm, label, font, pos.x, pos.y, size.x, size.y )
    {
    }

    bool
    propagateEventToChildren( const Event& event ) override
    {
        return event.apply( &cursor_ );
    }

    bool
    onMousePress( const hui::Event& event ) override
    {
        is_focused_ = Widget::onMousePress( event );

        return is_focused_;
    }

    bool
    onIdle( const Event& event ) override
    {
        ++govnokod_;

        if ( govnokod_ % 15 != 0 )
        {
            return false;
        }

        if ( govnokod_ % 10 < 5 )
        {
            cursor_.setColor( { 0, 0, 0, 0 } );
        } else
        {
            cursor_.setColor( { 255, 255, 255, 255 } );
        }

        return false;
    }

    bool
    onMouseMove( const Event& event ) override
    {
        // Widget::onMouseMove( event );

        dr4::Vec2f mouse_pos( event.info.mouseMove.pos.x, event.info.mouseMove.pos.y );
        // is_hovered_ = pointInside( mouse_pos );

        dr4::Vec2f rel_pos        = getRelPos();
        dr4::Vec2f parent_abs_pos = getParentAbsPos();

        dr4::Vec2f abs_pos = parent_abs_pos + rel_pos;
        dr4::Vec2f sz      = getSize();

        is_hovered_ = ( ( mouse_pos.x >= abs_pos.x && mouse_pos.x <= abs_pos.x + sz.x ) &&
                        ( mouse_pos.y >= abs_pos.y && mouse_pos.y <= abs_pos.y + sz.y ) );

        if ( !is_hovered_ )
        {
            is_pressed_ = false;
        }

        if ( is_dragging_ )
        {
            setRelPos( mouse_pos - drag_offset_ - getParentAbsPos() );
            return true;
        }

        return is_hovered_;
    }

    bool
    onTextEnter( const Event& event ) override
    {
        if ( !is_focused_ )
        {
            return false;
        }

        const char* str   = event.info.text.unicode;
        auto        old_w = text_->GetBounds().x;

        if ( *str == '\b' )
        {
            if ( str_.size() > 0 )
            {
                // fprintf( stderr, "debug in %s:%d:%s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__
                // );
                str_.pop_back();
            }
        } else
        {
            if ( old_w >= 0.9 * border_->GetSize().x )
            {
                return true;
            }

            // fprintf( stderr, "debug in %s:%d:%s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__ );

            str_ += *str;
        }

        text_->SetText( str_ );

        // fprintf( stderr, "%s\n", str_.data() );

        auto  new_w = text_->GetBounds().x;
        float ch_w  = new_w - old_w;

        cursor_.move( ch_w );

        return true;
    }

    std::optional<double>
    strToDouble()
    {
        try
        {
            return std::stod( str_ );
        } catch ( const std::invalid_argument& e )
        {
            return std::nullopt;
        } catch ( const std::out_of_range& e )
        {
            return std::nullopt;
        }
    }

    std::optional<uint32_t>
    strToUint32()
    {
        try
        {
            return std::stoul( str_ );
        } catch ( const std::invalid_argument& e )
        {
            return std::nullopt;
        } catch ( const std::out_of_range& e )
        {
            return std::nullopt;
        }
    }

  private:
    void
    RedrawMyTexture() const override final
    {
        texture_->Draw( *label_ );
        texture_->Draw( *text_ );
        texture_->Draw( *border_ );

        if ( is_focused_ )
        {
            cursor_.Redraw();
            // window.draw( cursor_, transform );
        }
    }

  private:
    bool is_focused_ = false;

    int govnokod_ = 0;

    std::unique_ptr<dr4::Text> label_;

    Cursor                          cursor_;
    std::unique_ptr<dr4::Rectangle> border_;

    std::string                str_;
    std::unique_ptr<dr4::Text> text_;
};

} // namespace hui
