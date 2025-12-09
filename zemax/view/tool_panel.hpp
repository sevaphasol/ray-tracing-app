#pragma once

#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/scrollable_list_widget.hpp"
#include "custom-hui-impl/window_manager.hpp"
#include "pp/tool.hpp"
#include "zemax/config.hpp"
#include <optional>
#include <vector>

namespace zemax {
namespace view {

class ToolPanel : public hui::ScrollableListWidget {
    using Tools = std::vector<std::unique_ptr<pp::Tool>>*;

  public:
    ToolPanel( hui::WindowManager* wm, float x, float y, float visible_height );

    void
    addTools( Tools tools );

    std::optional<size_t>
    getActiveToolIdx() const;
    void
    setActiveTool( std::optional<size_t> idx );

  private:
    std::optional<size_t>  active_tool_idx_;
    std::vector<pp::Tool*> tools_;
    const dr4::Font*       font_;
    float                  button_size_;
    float                  padding_;
};

ToolPanel::ToolPanel( hui::WindowManager* wm, float x, float y, float visible_height )
    : hui::ScrollableListWidget(
          wm,
          { x, y },
          { 50.0f + 2.0f * 10.0f, visible_height }, // content_size: width = button + 2*padding
          10                                        // scrollbar width
          ),
      font_( wm->getWindow()->GetDefaultFont() ),
      button_size_( 50.0f ),
      padding_( 10.0f )
{
    setDraggable( true );
}

void
ToolPanel::addTools( Tools tools )
{
    tools_.clear();
    active_tool_idx_.reset();
    clearItems();

    if ( tools == nullptr )
    {
        return;
    }

    for ( size_t i = 0; i < tools->size(); ++i )
    {
        auto btn = std::make_unique<hui::Button>( wm_,
                                                  dr4::Vec2f( padding_, 0.0f ),
                                                  dr4::Vec2f( button_size_, button_size_ ),
                                                  zemax::Config::ControlPanel::Button::DefaultColor,
                                                  zemax::Config::ControlPanel::Button::HoveredColor,
                                                  zemax::Config::ControlPanel::Button::PressedColor,
                                                  std::string( ( *tools )[i]->Icon() ),
                                                  zemax::Config::ControlPanel::Button::FontColor,
                                                  zemax::Config::ControlPanel::Button::FontSize );

        tools_.push_back( ( *tools )[i].get() );

        size_t capture_index = i;
        btn->setOnClick( [this, capture_index]() {
            if ( active_tool_idx_.has_value() )
            {
                tools_[active_tool_idx_.value()]->OnEnd();
            }
            active_tool_idx_ = capture_index;
            tools_[capture_index]->OnStart();
        } );

        addItem( std::move( btn ) );
    }
}

std::optional<size_t>
ToolPanel::getActiveToolIdx() const
{
    return active_tool_idx_;
}

void
ToolPanel::setActiveTool( std::optional<size_t> idx )
{
    active_tool_idx_ = idx;
}

} // namespace view
} // namespace zemax
