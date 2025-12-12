#include "rta/view/utils/rgb_picker.hpp"
#include "dr4/math/color.hpp"
#include "dr4/math/vec2.hpp"
#include "hui/widget.hpp"
#include "hui/window_manager.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <optional>
#include <string>

rta::view::RGBPicker::RGBPicker( hui::WindowManager* wm,
                                 const dr4::Vec2f&   pos,
                                 const dr4::Color&   initial_color,
                                 ColorChangeCallback on_change,
                                 const Theme&        theme )
    : hui::Widget( wm, pos, calcSize( theme ) ),
      theme_( theme ),
      current_color_( initial_color ),
      on_color_change_( std::move( on_change ) )
{
    background_rect_.reset( wm_->getWindow()->CreateRectangle() );
    bar_rect_.reset( wm_->getWindow()->CreateRectangle() );
    segment_rect_.reset( wm_->getWindow()->CreateRectangle() );
    marker_rect_.reset( wm_->getWindow()->CreateRectangle() );
    preview_background_.reset( wm_->getWindow()->CreateRectangle() );
    preview_rect_.reset( wm_->getWindow()->CreateRectangle() );

    background_rect_->SetSize( getSize() );
    background_rect_->SetFillColor( theme_.background_fill );
    background_rect_->SetBorderThickness( 1.0f );
    background_rect_->SetBorderColor( theme_.background_border );

    bar_rect_->SetFillColor( theme_.bar_fill );
    bar_rect_->SetBorderThickness( 1.0f );
    bar_rect_->SetBorderColor( theme_.bar_border );

    marker_rect_->SetFillColor( theme_.marker_fill );
    marker_rect_->SetBorderThickness( 0.0f );

    const float preview_x = barStartX() + theme_.bar_width + theme_.padding;
    const float preview_y = theme_.padding;

    preview_background_->SetPos( { preview_x, preview_y } );
    preview_background_->SetSize( { theme_.preview_size, theme_.preview_size } );
    preview_background_->SetFillColor( theme_.preview_fill );
    preview_background_->SetBorderThickness( 1.0f );
    preview_background_->SetBorderColor( theme_.preview_border );

    preview_rect_->SetPos( { preview_x, preview_y } );
    preview_rect_->SetSize( { theme_.preview_size, theme_.preview_size } );
    preview_rect_->SetFillColor( current_color_ );
    preview_rect_->SetBorderThickness( 1.0f );
    preview_rect_->SetBorderColor( theme_.preview_outline );

    const dr4::Font* font = wm_->getWindow()->GetDefaultFont();
    for ( auto channel : Channels )
    {
        const size_t idx = channelIndex( channel );

        labels_[idx].reset( wm_->getWindow()->CreateText() );
        labels_[idx]->SetFont( font );
        labels_[idx]->SetText( std::string( 1, ChannelLabels[idx] ) );
        labels_[idx]->SetFontSize( theme_.label_font_size );
        labels_[idx]->SetColor( theme_.label_color );

        const auto  pos    = channelPos( channel );
        const auto  bounds = labels_[idx]->GetBounds();
        const float x      = theme_.padding;
        const float y      = pos.y + theme_.bar_height * 0.5f - bounds.y * 0.5f;
        labels_[idx]->SetPos( { x, y } );
    }
}

void
rta::view::RGBPicker::setColor( const dr4::Color& color )
{
    current_color_ = color;
    preview_rect_->SetFillColor( current_color_ );
}

dr4::Color
rta::view::RGBPicker::getColor() const
{
    return current_color_;
}

void
rta::view::RGBPicker::setOnColorChange( ColorChangeCallback cb )
{
    on_color_change_ = std::move( cb );
}

bool
rta::view::RGBPicker::onMousePress( const hui::Event& event )
{
    if ( event.info.mouseButton.button != dr4::MouseButtonType::LEFT )
    {
        return false;
    }

    const dr4::Vec2f local   = event.info.mouseButton.pos - getAbsPos();
    const auto       channel = channelFromLocalPos( local );

    if ( !channel )
    {
        return false;
    }

    active_channel_ = channel;
    updateChannelValue( channel.value(), local.x );

    return true;
}

bool
rta::view::RGBPicker::onMouseMove( const hui::Event& event )
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
rta::view::RGBPicker::onMouseRelease( const hui::Event& event )
{
    if ( event.info.mouseButton.button != dr4::MouseButtonType::LEFT )
    {
        return false;
    }

    const bool was_active = active_channel_.has_value();
    active_channel_.reset();

    return was_active;
}

dr4::Vec2f
rta::view::RGBPicker::calcSize( const Theme& theme )
{
    const float bars_height = static_cast<float>( ChannelsCount ) * theme.bar_height +
                              static_cast<float>( ChannelsCount - 1 ) * theme.bar_spacing;
    const float width =
        theme.padding * 4.0f + theme.label_width + theme.bar_width + theme.preview_size;
    const float height = theme.padding * 2.0f + std::max( bars_height, theme.preview_size );
    return { width, height };
}

dr4::Vec2f
rta::view::RGBPicker::channelPos( Channel idx ) const
{
    const float x = barStartX();
    const float y =
        theme_.padding + static_cast<size_t>( idx ) * ( theme_.bar_height + theme_.bar_spacing );
    return { x, y };
}

std::optional<rta::view::RGBPicker::Channel>
rta::view::RGBPicker::channelFromLocalPos( const dr4::Vec2f& local ) const
{
    const float start_x = barStartX();
    if ( local.x < start_x || local.x > start_x + theme_.bar_width )
    {
        return std::nullopt;
    }

    for ( auto channel : Channels )
    {
        const auto pos = channelPos( channel );
        if ( local.y >= pos.y && local.y <= pos.y + theme_.bar_height )
        {
            return channel;
        }
    }

    return std::nullopt;
}

size_t
rta::view::RGBPicker::channelIndex( Channel channel )
{
    return static_cast<size_t>( channel );
}

uint8_t&
rta::view::RGBPicker::channelComponent( dr4::Color& color, Channel channel )
{
    switch ( channel )
    {
        case Channel::R:
            return color.r;
        case Channel::G:
            return color.g;
        case Channel::B:
            return color.b;
        case Channel::A:
            return color.a;
    }

    return color.r;
}

uint8_t
rta::view::RGBPicker::channelComponent( const dr4::Color& color, Channel channel )
{
    switch ( channel )
    {
        case Channel::R:
            return color.r;
        case Channel::G:
            return color.g;
        case Channel::B:
            return color.b;
        case Channel::A:
            return color.a;
    }

    return color.r;
}

void
rta::view::RGBPicker::updateChannelValue( Channel channel, float local_x )
{
    setChannelValue( channel, byteFromLocalX( local_x ) );

    if ( on_color_change_ )
    {
        on_color_change_( current_color_ );
    }

    preview_rect_->SetFillColor( current_color_ );
}

uint8_t
rta::view::RGBPicker::channelValue( Channel channel ) const
{
    return channelComponent( current_color_, channel );
}

void
rta::view::RGBPicker::setChannelValue( Channel channel, uint8_t value )
{
    channelComponent( current_color_, channel ) = value;
}

void
rta::view::RGBPicker::setChannelValue( dr4::Color& color, Channel channel, uint8_t value )
{
    channelComponent( color, channel ) = value;
}

dr4::Color
rta::view::RGBPicker::colorForChannel( Channel channel, float t ) const
{
    dr4::Color color = current_color_;
    setChannelValue( color, channel, normalizedToByte( t ) );
    return color;
}

float
rta::view::RGBPicker::barStartX() const
{
    return theme_.padding + theme_.label_width + theme_.padding;
}

uint8_t
rta::view::RGBPicker::byteFromLocalX( float local_x ) const
{
    const float start_x = barStartX();
    const float clamped = std::clamp( local_x, start_x, start_x + theme_.bar_width );
    const float t       = ( clamped - start_x ) / theme_.bar_width;

    return normalizedToByte( t );
}

uint8_t
rta::view::RGBPicker::normalizedToByte( float t )
{
    return static_cast<uint8_t>(
        std::clamp<int>( static_cast<int>( std::round( t * 255.0f ) ), 0, 255 ) );
}

void
rta::view::RGBPicker::drawBarRect( Channel channel, dr4::Vec2f pos ) const
{
    bar_rect_->SetPos( pos );
    bar_rect_->SetSize( { theme_.bar_width, theme_.bar_height } );
    texture_->Draw( *bar_rect_ );
}

void
rta::view::RGBPicker::drawSegments( Channel    channel,
                                    dr4::Vec2f pos,
                                    float      bar_start_x,
                                    float      bar_end_x ) const
{
    const float  segment_w = std::clamp( std::round( theme_.bar_width / 256.0f ), 1.0f, 2.0f );
    const size_t segments =
        std::max( 1LU, static_cast<size_t>( std::ceil( theme_.bar_width / segment_w ) ) );

    for ( size_t i = 0; i < segments; ++i )
    {
        const float x          = bar_start_x + segment_w * static_cast<float>( i );
        const float segment_sz = std::min( segment_w, bar_end_x - x );
        if ( segment_sz <= 0.0f )
        {
            break;
        }

        const float t = segments > 1 ? static_cast<float>( i ) / ( segments - 1 ) : 0.0f;
        segment_rect_->SetPos( { x, pos.y } );
        segment_rect_->SetSize( { segment_sz, theme_.bar_height } );
        segment_rect_->SetFillColor( colorForChannel( channel, t ) );
        texture_->Draw( *segment_rect_ );
    }
}

void
rta::view::RGBPicker::drawMarker( Channel    channel,
                                  dr4::Vec2f pos,
                                  float      bar_start_x,
                                  float      bar_end_x ) const
{
    const float marker_center =
        bar_start_x + static_cast<float>( channelValue( channel ) ) / 255.0f * theme_.bar_width;
    const float marker_x = std::clamp( marker_center - theme_.marker_width * 0.5f,
                                       bar_start_x,
                                       bar_end_x - theme_.marker_width );

    marker_rect_->SetPos( { marker_x, pos.y - 2.0f } );
    marker_rect_->SetSize( { theme_.marker_width, theme_.bar_height + 4.0f } );
    texture_->Draw( *marker_rect_ );
}

void
rta::view::RGBPicker::drawLabel( Channel channel ) const
{
    texture_->Draw( *labels_[channelIndex( channel )] );
}

void
rta::view::RGBPicker::drawChannel( Channel channel ) const
{
    const dr4::Vec2f pos         = channelPos( channel );
    const float      bar_start_x = pos.x;
    const float      bar_end_x   = bar_start_x + theme_.bar_width;

    drawBarRect( channel, pos );
    drawSegments( channel, pos, bar_start_x, bar_end_x );
    drawMarker( channel, pos, bar_start_x, bar_end_x );
    drawLabel( channel );
}

void
rta::view::RGBPicker::RedrawMyTexture() const
{
    texture_->Draw( *background_rect_ );

    for ( auto channel : Channels )
    {
        drawChannel( channel );
    }

    texture_->Draw( *preview_background_ );
    texture_->Draw( *preview_rect_ );
}
