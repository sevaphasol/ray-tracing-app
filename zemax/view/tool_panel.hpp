#pragma once

#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/window_manager.hpp"
#include "pp/tool.hpp"
#include "zemax/config.hpp"
#include <optional>
#include <vector>

namespace zemax {
namespace view {

class ToolPanel : public hui::ContainerWidget {
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
    std::vector<std::unique_ptr<hui::Button>> buttons_;

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        for ( auto& btn : buttons_ )
        {
            if ( event.apply( btn.get() ) )
            {
                return true;
            }
        }
        return false;
    }

    void
    RedrawMyTexture() const override
    {
        for ( auto& btn : buttons_ )
        {
            btn->Redraw();
        }
    }
};

ToolPanel::ToolPanel( hui::WindowManager* wm, float x, float y, float visible_height )
    : hui::ContainerWidget( wm, { x, y }, { 50.0f + 2.0f * 10.0f, visible_height } ),
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

    buttons_.clear();

    if ( tools == nullptr )
    {
        setSize( { button_size_ + 2 * padding_, padding_ * 2 } );
        return;
    }

    for ( size_t i = 0; i < tools->size(); ++i )
    {
        auto btn = std::make_unique<hui::Button>( wm_,
                                                  dr4::Vec2f( padding_, padding_ + i * ( button_size_ + padding_ ) ),
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

        btn->setParent( this );
        buttons_.push_back( std::move( btn ) );
    }

    float height = padding_ + tools_.size() * ( button_size_ + padding_ );
    setSize( { button_size_ + 2 * padding_, height } );
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
