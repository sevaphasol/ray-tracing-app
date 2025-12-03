// #pragma once
//
// #include "custom-hui-impl/button.hpp"
// #include "custom-hui-impl/container_widget.hpp"
// #include "custom-hui-impl/plugin_manager.hpp"
// #include "custom-hui-impl/widget.hpp"
// #include "dr4/texture.hpp"
// #include "pp/tool.hpp"
// #include <memory>
// #include <vector>
//
// namespace zemax {
// namespace view {
//
// class ToolPanel : public hui::ContainerWidget {
//     using Tools = std::vector<std::unique_ptr<pp::Tool>>*;
//
//   public:
//     ToolPanel( cum::PluginManager* pm,
//                float               x,
//                float               y,
//                float               w,
//                float               h,
//                Tools               tools,
//                dr4::Font*          font )
//         : hui::ContainerWidget( pm, x, y, w, h ), tools_( tools )
//     {
//         setDraggable( true );
//
//         buttons_.resize( tools->size() );
//
//         for ( size_t i = 0; i < tools_->size(); ++i )
//         {
//             buttons_[i]->setLabelFont( font, 12 );
//             buttons_[i]->setLabelText( std::string( ( *tools_ )[i]->Icon() ) );
//             buttons_[i]->setSize( { 50, 50 } );
//             buttons_[i]->setBackgroundColor( { 16, 16, 16 } );
//             buttons_[i]->setParent( this );
//             buttons_[i]->setRelPos( { 10, float( i ) } );
//         }
//
//         rect_->SetSize( { w, h } );
//         rect_->SetBorderColor( { 118, 185, 0 } );
//         rect_->SetFillColor( { 32, 32, 32 } );
//         rect_->SetBorderThickness( 2.0f );
//     }
//
//     bool
//     onMousePress( const hui::Event& event ) override final
//     {
//         hui::ContainerWidget::onMousePress( event );
//
//         for ( size_t i = 0; i < buttons_.size(); ++i )
//         {
//             if ( buttons_[i]->isPressed() )
//             {
//                 active_tool_idx_ = i;
//                 return true;
//             }
//         }
//
//         active_tool_idx_ = std::nullopt;
//
//         return false;
//     }
//
//     std::optional<size_t>
//     getActiveToolIdx() const
//     {
//         return active_tool_idx_;
//     }
//
//     bool
//     propagateEventToChildren( const hui::Event& event ) override final
//     {
//         for ( auto& btn : buttons_ )
//         {
//             if ( event.apply( btn.get() ) )
//             {
//                 return true;
//             }
//         }
//
//         return false;
//     }
//
//   private:
//     std::optional<size_t> active_tool_idx_;
//
//     std::unique_ptr<dr4::Rectangle> rect_;
//
//     std::vector<std::unique_ptr<hui::Button>> buttons_;
//
//     Tools tools_;
// };
//
// } // namespace view
// } // namespace zemax

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
    ToolPanel( cum::PluginManager* pm, float x, float y, Tools tools, const dr4::Font* font )
        : hui::ContainerWidget( pm, x, y, 2 * 10 + 50, 2 * 10 + 50 * tools->size() ),
          tools_( tools )
    {
        setDraggable( true );

        for ( size_t i = 0; i < tools_->size(); ++i )
        {
            buttons_.push_back(
                std::make_unique<hui::Button>( pm,
                                               dr4::Vec2f( 10, 10 + float( i ) * 50 ),
                                               dr4::Vec2f( 50, 50 ),
                                               zemax::Config::ControlPanel::Button::DefaultColor,
                                               zemax::Config::ControlPanel::Button::HoveredColor,
                                               zemax::Config::ControlPanel::Button::PressedColor,
                                               font,
                                               std::string( ( *tools_ )[i]->Icon() ),
                                               zemax::Config::ControlPanel::Button::FontColor,
                                               zemax::Config::ControlPanel::Button::FontSize ) );

            buttons_[i]->setParent( this );
            // buttons_[i]->setLabelFont( font, 12 );
            // buttons_[i]->setLabelText( std::string( ( *tools_ )[i]->Icon() ) );
            // buttons_[i]->setSize( { 50, 50 } );
            // buttons_[i]->setBackgroundColor( { 16, 16, 16 } );
            // buttons_[i]->setParent( this );
            // buttons_[i]->setRelPos( { 10, float( i ) } );
        }

        rect_.reset( pm->getWindow()->CreateRectangle() );

        rect_->SetSize( { 2 * 10 + 50, 2 * 10 + 50 * float( tools->size() ) } );
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
                    active_tool_idx_ = i;
                    ( *tools_ )[i]->OnStart();
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

    Tools tools_;
};

} // namespace view
} // namespace zemax
