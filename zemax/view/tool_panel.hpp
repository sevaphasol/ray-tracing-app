#pragma once

#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/container_widget.hpp"
#include "custom-hui-impl/plugin_manager.hpp"
#include "custom-hui-impl/widget.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include "pp/tool.hpp"
#include "zemax/config.hpp"
#include <memory>
#include <string>
#include <vector>

namespace zemax {
namespace view {

class ToolPanel : public hui::ContainerWidget {
    using Tools = std::vector<std::unique_ptr<pp::Tool>>*;

  public:
    ToolPanel( cum::Manager* pm, dr4::Window* win, float x, float y, const dr4::Font* font )
        : hui::ContainerWidget( pm, win, x, y, 2 * 10 + 50, 2 * 10 + 50 )
    {
        setDraggable( true );

        rect_.reset( win->CreateRectangle() );
    }

    void
    addTools( Tools tools )
    {
        for ( auto& tool_ptr : *tools )
        {
            tools_.push_back( tool_ptr.get() );
        }

        setSize( { getSize().x, static_cast<float>( 2 * 10 + 50 * tools_.size() ) } );

        for ( size_t i = 0; i < tools->size(); ++i )
        {
            buttons_.push_back(
                std::make_unique<hui::Button>( pm_,
                                               window_,
                                               dr4::Vec2f( 10, 10 + float( i ) * 50 ),
                                               dr4::Vec2f( 50, 50 ),
                                               zemax::Config::ControlPanel::Button::DefaultColor,
                                               zemax::Config::ControlPanel::Button::HoveredColor,
                                               zemax::Config::ControlPanel::Button::PressedColor,
                                               std::string( ( *tools )[i]->Icon() ),
                                               zemax::Config::ControlPanel::Button::FontColor,
                                               zemax::Config::ControlPanel::Button::FontSize ) );
            buttons_.back()->setParent( this );
        }

        float height = 2 * 10 + 50 * static_cast<float>( tools_.size() );
        setSize( { 2 * 10 + 50, height } );
        rect_->SetSize( { 2 * 10 + 50, height } );
        rect_->SetBorderColor( { 118, 185, 0 } );
        rect_->SetFillColor( { 8, 8, 8 } );
        rect_->SetBorderThickness( 2.0f );
    }

    bool
    onMousePress( const hui::Event& event ) override final
    {
        if ( !onMe( event.info.mouseButton.pos ) )
        {
            // std::cerr << "ToolPanel: !onMe" << std::endl;
            return false;
        }

        if ( hui::ContainerWidget::onMousePress( event ) )
        {
            for ( size_t i = 0; i < buttons_.size(); ++i )
            {
                if ( buttons_[i]->isPressed() )
                {
                    if ( active_tool_idx_.has_value() )
                    {
                        tools_[active_tool_idx_.value()]->OnEnd();
                    }

                    active_tool_idx_ = i;
                    tools_[i]->OnStart();
                    return true;
                }
            }
        }

        active_tool_idx_ = std::nullopt;

        return true;
    }

    std::optional<size_t>
    getActiveToolIdx() const
    {
        return active_tool_idx_;
    }

    void
    setActiveTool( std::optional<size_t> idx )
    {
        active_tool_idx_ = idx;
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override final
    {
        bool my_event = false;

        for ( auto& btn : buttons_ )
        {
            // std::cerr << "Giving to Button " << typeid( event ).name() << std::endl;

            if ( event.apply( btn.get() ) )
            {
                // std::cerr << "Button took" << std::endl;
                my_event = true;
            }

            // std::cerr << "Button didn't took" << std::endl;
        }

        return my_event;
    }

    void
    RedrawMyTexture() const override final
    {
        texture_->Draw( *rect_ );
        for ( const auto& btn : buttons_ )
        {
            btn->Redraw();
        }
    }

  private:
    std::optional<size_t> active_tool_idx_;

    std::unique_ptr<dr4::Rectangle> rect_;

    std::vector<std::unique_ptr<hui::Button>> buttons_;

    std::vector<pp::Tool*> tools_;
};

} // namespace view
} // namespace zemax
