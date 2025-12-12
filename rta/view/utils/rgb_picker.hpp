#pragma once

#include "dr4/math/color.hpp"
#include "dr4/math/vec2.hpp"
#include "hui/widget.hpp"
#include "hui/window_manager.hpp"
#include <array>
#include <cmath>
#include <functional>
#include <memory>
#include <optional>

namespace rta {
namespace view {

class RGBPicker : public hui::Widget {
  public:
    using ColorChangeCallback = std::function<void( const dr4::Color& )>;

    struct Theme
    {
        dr4::Color background_fill   = { 20, 20, 20, 200 };
        dr4::Color background_border = { 55, 55, 55, 255 };
        dr4::Color bar_fill          = { 35, 35, 35, 255 };
        dr4::Color bar_border        = { 70, 70, 70, 255 };
        dr4::Color marker_fill       = { 255, 255, 255, 220 };
        dr4::Color preview_fill      = { 30, 30, 30, 255 };
        dr4::Color preview_border    = { 60, 60, 60, 255 };
        dr4::Color preview_outline   = { 255, 255, 255, 220 };
        dr4::Color label_color       = { 230, 230, 230, 255 };
        float      padding           = 10.0f;
        float      bar_width         = 256.0f;
        float      bar_height        = 12.0f;
        float      bar_spacing       = 10.0f;
        float      label_width       = 16.0f;
        float      preview_size      = 36.0f;
        float      marker_width      = 3.0f;
        float      label_font_size   = 15.0f;

        static Theme
        Default()
        {
            return Theme();
        }
    };

    enum class Channel : size_t { R = 0, G = 1, B = 2, A = 3 };

    static constexpr size_t                             ChannelsCount = 4;
    static constexpr std::array<char, ChannelsCount>    ChannelLabels{ 'R', 'G', 'B', 'A' };
    static constexpr std::array<Channel, ChannelsCount> Channels = { Channel::R,
                                                                     Channel::G,
                                                                     Channel::B,
                                                                     Channel::A };

    RGBPicker( hui::WindowManager* wm,
               const dr4::Vec2f&   pos,
               const dr4::Color&   initial_color,
               ColorChangeCallback on_change = nullptr,
               const Theme&        theme     = Theme::Default() );

    void
    setColor( const dr4::Color& color );

    dr4::Color
    getColor() const;

    void
    setOnColorChange( ColorChangeCallback cb );

    bool
    onMousePress( const hui::Event& event ) override;
    bool
    onMouseMove( const hui::Event& event ) override;
    bool
    onMouseRelease( const hui::Event& event ) override;

  private:
    static dr4::Vec2f
    calcSize( const Theme& theme );
    dr4::Vec2f
    channelPos( Channel idx ) const;
    std::optional<Channel>
    channelFromLocalPos( const dr4::Vec2f& local ) const;
    static size_t
    channelIndex( Channel channel );
    static uint8_t&
    channelComponent( dr4::Color& color, Channel channel );
    static uint8_t
    channelComponent( const dr4::Color& color, Channel channel );
    void
    updateChannelValue( Channel channel, float local_x );
    uint8_t
    channelValue( Channel channel ) const;
    void
    setChannelValue( Channel channel, uint8_t value );
    static void
    setChannelValue( dr4::Color& color, Channel channel, uint8_t value );
    dr4::Color
    colorForChannel( Channel channel, float t ) const;
    float
    barStartX() const;
    uint8_t
    byteFromLocalX( float local_x ) const;
    static uint8_t
    normalizedToByte( float t );
    void
    drawBarRect( Channel channel, dr4::Vec2f pos ) const;
    void
    drawSegments( Channel channel, dr4::Vec2f pos, float bar_start_x, float bar_end_x ) const;
    void
    drawMarker( Channel channel, dr4::Vec2f pos, float bar_start_x, float bar_end_x ) const;
    void
    drawLabel( Channel channel ) const;
    void
    drawChannel( Channel channel ) const;
    void
    RedrawMyTexture() const override;

  private:
    Theme                                                 theme_;
    dr4::Color                                            current_color_;
    ColorChangeCallback                                   on_color_change_;
    std::array<std::unique_ptr<dr4::Text>, ChannelsCount> labels_;

    std::unique_ptr<dr4::Rectangle> background_rect_;
    std::unique_ptr<dr4::Rectangle> bar_rect_;
    std::unique_ptr<dr4::Rectangle> segment_rect_;
    std::unique_ptr<dr4::Rectangle> marker_rect_;
    std::unique_ptr<dr4::Rectangle> preview_background_;
    std::unique_ptr<dr4::Rectangle> preview_rect_;

    std::optional<Channel> active_channel_;
};

} // namespace view
} // namespace rta
