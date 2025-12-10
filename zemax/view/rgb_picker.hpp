#pragma once

#include "custom-hui-impl/widget.hpp"
#include "custom-hui-impl/window_manager.hpp"
#include "dr4/math/color.hpp"
#include "dr4/math/vec2.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace zemax {
namespace view {

class RGBPicker : public hui::Widget {
  public:
    using ColorChangeCallback = std::function<void( const dr4::Color& )>;

    RGBPicker( hui::WindowManager* wm,
               const dr4::Vec2f&   pos,
               const dr4::Color&   initial_color,
               ColorChangeCallback on_change = nullptr )
        : hui::Widget( wm, pos, defaultSize() ),
          current_color_( initial_color ),
          on_color_change_( std::move( on_change ) )
    {
        setupPrimitives();
    }

    void
    setColor( const dr4::Color& color )
    {
        current_color_ = color;
        Redraw();
    }

    dr4::Color
    currentColor() const
    {
        return current_color_;
    }

    void
    setOnColorChange( ColorChangeCallback cb )
    {
        on_color_change_ = std::move( cb );
    }

    bool
    onMousePress( const hui::Event& event ) override
    {
        if ( event.info.mouseButton.button != dr4::MouseButtonType::LEFT )
        {
            return false;
        }

        const dr4::Vec2f local = event.info.mouseButton.pos - getAbsPos();
        const int        idx   = channelFromLocalPos( local );

        if ( idx < 0 )
        {
            return false;
        }

        active_channel_ = static_cast<size_t>( idx );
        updateChannelValue( active_channel_.value(), local.x );
        return true;
    }

    bool
    onMouseMove( const hui::Event& event ) override
    {
        Widget::onMouseMove( event );

        if ( !active_channel_.has_value() )
        {
            return is_hovered_;
        }

        const dr4::Vec2f local = event.info.mouseMove.pos - getAbsPos();
        updateChannelValue( active_channel_.value(), local.x );
        return true;
    }

    bool
    onMouseRelease( const hui::Event& event ) override
    {
        if ( event.info.mouseButton.button != dr4::MouseButtonType::LEFT )
        {
            return false;
        }

        const bool was_active = active_channel_.has_value();
        active_channel_.reset();
        return was_active;
    }

  private:
    static dr4::Vec2f
    defaultSize()
    {
        const float bars_height = 4.0f * bar_height_ + 3.0f * bar_spacing_;
        const float width       = padding_ * 3.0f + label_width_ + bar_width_ + preview_size_;
        const float height      = padding_ * 2.0f + std::max( bars_height, preview_size_ );
        return { width, height };
    }

    void
    setupPrimitives()
    {
        font_ = wm_->getWindow()->GetDefaultFont();

        background_rect_.reset( wm_->getWindow()->CreateRectangle() );
        bar_rect_.reset( wm_->getWindow()->CreateRectangle() );
        segment_rect_.reset( wm_->getWindow()->CreateRectangle() );
        marker_rect_.reset( wm_->getWindow()->CreateRectangle() );
        preview_background_.reset( wm_->getWindow()->CreateRectangle() );
        preview_rect_.reset( wm_->getWindow()->CreateRectangle() );

        background_rect_->SetSize( getSize() );
        background_rect_->SetFillColor( { 20, 20, 20, 200 } );
        background_rect_->SetBorderThickness( 1.0f );
        background_rect_->SetBorderColor( { 55, 55, 55, 255 } );

        bar_rect_->SetFillColor( { 35, 35, 35, 255 } );
        bar_rect_->SetBorderThickness( 1.0f );
        bar_rect_->SetBorderColor( { 70, 70, 70, 255 } );

        marker_rect_->SetFillColor( { 255, 255, 255, 220 } );
        marker_rect_->SetBorderThickness( 0.0f );

        preview_background_->SetFillColor( { 30, 30, 30, 255 } );
        preview_background_->SetBorderThickness( 1.0f );
        preview_background_->SetBorderColor( { 60, 60, 60, 255 } );

        preview_rect_->SetBorderThickness( 1.0f );
        preview_rect_->SetBorderColor( { 255, 255, 255, 220 } );

        const char* labels = "RGBA";
        for ( size_t i = 0; i < labels_.size(); ++i )
        {
            labels_[i].reset( wm_->getWindow()->CreateText() );
            labels_[i]->SetFont( font_ );
            labels_[i]->SetText( std::string( 1, labels[i] ) );
            labels_[i]->SetFontSize( 12.0f );
            labels_[i]->SetColor( { 230, 230, 230, 255 } );

            const auto  pos    = channelPos( i );
            const auto  bounds = labels_[i]->GetBounds();
            const float x      = padding_;
            const float y      = pos.y + bar_height_ * 0.5f - bounds.y * 0.5f;
            labels_[i]->SetPos( { x, y } );
        }
    }

    dr4::Vec2f
    channelPos( size_t idx ) const
    {
        const float x = barStartX();
        const float y = padding_ + idx * ( bar_height_ + bar_spacing_ );
        return { x, y };
    }

    int
    channelFromLocalPos( const dr4::Vec2f& local ) const
    {
        const float start_x = barStartX();
        if ( local.x < start_x || local.x > start_x + bar_width_ )
        {
            return -1;
        }

        for ( size_t i = 0; i < labels_.size(); ++i )
        {
            const auto pos = channelPos( i );
            if ( local.y >= pos.y && local.y <= pos.y + bar_height_ )
            {
                return static_cast<int>( i );
            }
        }

        return -1;
    }

    void
    updateChannelValue( size_t idx, float local_x )
    {
        const float   start_x = barStartX();
        const float   clamped = std::clamp( local_x, start_x, start_x + bar_width_ );
        const float   t       = ( clamped - start_x ) / bar_width_;
        const uint8_t value   = static_cast<uint8_t>(
            std::clamp<int>( static_cast<int>( std::round( t * 255.0f ) ), 0, 255 ) );

        switch ( idx )
        {
            case 0:
                current_color_.r = value;
                break;
            case 1:
                current_color_.g = value;
                break;
            case 2:
                current_color_.b = value;
                break;
            case 3:
                current_color_.a = value;
                break;
            default:
                break;
        }

        notifyChange();
    }

    uint8_t
    channelValue( size_t idx ) const
    {
        switch ( idx )
        {
            case 0:
                return current_color_.r;
            case 1:
                return current_color_.g;
            case 2:
                return current_color_.b;
            case 3:
                return current_color_.a;
            default:
                return 0;
        }
    }

    dr4::Color
    colorForChannel( size_t idx, float t ) const
    {
        dr4::Color    res   = current_color_;
        const uint8_t value = static_cast<uint8_t>(
            std::clamp<int>( static_cast<int>( std::round( t * 255.0f ) ), 0, 255 ) );
        switch ( idx )
        {
            case 0:
                res.r = value;
                break;
            case 1:
                res.g = value;
                break;
            case 2:
                res.b = value;
                break;
            case 3:
                res.a = value;
                break;
            default:
                break;
        }
        return res;
    }

    float
    barStartX() const
    {
        return padding_ + label_width_ + padding_;
    }

    void
    notifyChange()
    {
        if ( on_color_change_ )
        {
            on_color_change_( current_color_ );
        }

        Redraw();
    }

    void
    drawChannel( size_t idx ) const
    {
        const dr4::Vec2f pos       = channelPos( idx );
        const float      start_x   = pos.x;
        const float      segment_w = std::clamp( std::round( bar_width_ / 256.0f ), 1.0f, 2.0f );
        const size_t     segments =
            std::max<size_t>( 1, static_cast<size_t>( std::ceil( bar_width_ / segment_w ) ) );

        bar_rect_->SetPos( pos );
        bar_rect_->SetSize( { bar_width_, bar_height_ } );
        texture_->Draw( *bar_rect_ );

        const float bar_end = start_x + bar_width_;
        for ( size_t i = 0; i < segments; ++i )
        {
            const float x          = start_x + segment_w * static_cast<float>( i );
            const float segment_sz = std::min( segment_w, bar_end - x );
            if ( segment_sz <= 0.0f )
            {
                break;
            }

            const float t = segments > 1 ? static_cast<float>( i ) / ( segments - 1 ) : 0.0f;
            segment_rect_->SetPos( { x, pos.y } );
            segment_rect_->SetSize( { segment_sz, bar_height_ } );
            segment_rect_->SetFillColor( colorForChannel( idx, t ) );
            texture_->Draw( *segment_rect_ );
        }

        const float marker_center =
            start_x + static_cast<float>( channelValue( idx ) ) / 255.0f * bar_width_;
        const float marker_x =
            std::clamp( marker_center - marker_width_ * 0.5f, start_x, bar_end - marker_width_ );

        marker_rect_->SetPos( { marker_x, pos.y - 2.0f } );
        marker_rect_->SetSize( { marker_width_, bar_height_ + 4.0f } );
        texture_->Draw( *marker_rect_ );

        texture_->Draw( *labels_[idx] );
    }

    void
    RedrawMyTexture() const override
    {
        background_rect_->SetSize( getSize() );
        texture_->Draw( *background_rect_ );

        for ( size_t i = 0; i < labels_.size(); ++i )
        {
            drawChannel( i );
        }

        const float preview_x = barStartX() + bar_width_ + padding_;
        const float preview_y = padding_;

        preview_background_->SetPos( { preview_x, preview_y } );
        preview_background_->SetSize( { preview_size_, preview_size_ } );
        texture_->Draw( *preview_background_ );

        preview_rect_->SetPos( { preview_x, preview_y } );
        preview_rect_->SetSize( { preview_size_, preview_size_ } );
        preview_rect_->SetFillColor( current_color_ );
        texture_->Draw( *preview_rect_ );
    }

  private:
    static constexpr float padding_      = 6.0f;
    static constexpr float bar_width_    = 256.0f;
    static constexpr float bar_height_   = 12.0f;
    static constexpr float bar_spacing_  = 10.0f;
    static constexpr float label_width_  = 16.0f;
    static constexpr float preview_size_ = 36.0f;
    static constexpr float marker_width_ = 3.0f;

    dr4::Color                                current_color_;
    ColorChangeCallback                       on_color_change_;
    std::array<std::unique_ptr<dr4::Text>, 4> labels_;
    const dr4::Font*                          font_ = nullptr;

    std::unique_ptr<dr4::Rectangle> background_rect_;
    std::unique_ptr<dr4::Rectangle> bar_rect_;
    std::unique_ptr<dr4::Rectangle> segment_rect_;
    std::unique_ptr<dr4::Rectangle> marker_rect_;
    std::unique_ptr<dr4::Rectangle> preview_background_;
    std::unique_ptr<dr4::Rectangle> preview_rect_;

    mutable std::optional<size_t> active_channel_;
};

} // namespace view
} // namespace zemax
