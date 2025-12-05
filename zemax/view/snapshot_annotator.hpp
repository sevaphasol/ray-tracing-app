#pragma once

#include "cum/ifc/pp.hpp"
#include "cum/manager.hpp"
#include "custom-hui-impl/event.hpp"
#include "custom-hui-impl/widget.hpp"
#include "dr4/event.hpp"
#include "dr4/keycodes.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include "pp/canvas.hpp"
#include "pp/shape.hpp"
#include "pp/tool.hpp"
#include "zemax/view/tool_panel.hpp"
#include <cassert>
#include <memory>
#include <unordered_map>
#include <utility>

namespace zemax {
namespace view {

class SnapshotAnnotator : public hui::Widget, public pp::Canvas {
  public:
    SnapshotAnnotator( cum::Manager*    pm,
                       dr4::Window*     win,
                       float            x,
                       float            y,
                       float            w,
                       float            h,
                       const dr4::Font* font )
        : hui::Widget( pm, win, x, y, w, h ),
          pp::Canvas(),
          active_( false ),
          tool_panel_( pm, win, 10, 100, font )
    {
        for ( auto* plg : pm->GetAllOfType<cum::PPToolPlugin>() )
        {
            auto new_tools = plg->CreateTools( this );
            for ( auto& tool : new_tools )
            {
                tools_.push_back( std::move( tool ) );
            }
        }

        tool_panel_.addTools( &tools_ );

        tool_panel_.setParent( this );

        border_.reset( win->CreateRectangle() );

        border_->SetBorderColor( { 0, 118, 185, 255 } );
        border_->SetBorderThickness( -2 );
        border_->SetFillColor( { 0, 0, 0, 128 } );
        border_->SetSize( { w, h } );

        theme_ = { { 0, 0, 0, 0 },
                   { 0, 118, 185, 255 },
                   { 0, 118, 185, 255 },
                   { 255, 255, 255, 255 },
                   18,
                   { 128, 128, 128, 255 } };
    }

    SnapshotAnnotator( cum::Manager*     pm,
                       dr4::Window*      win,
                       const dr4::Vec2f& pos,
                       const dr4::Vec2f& size,
                       const dr4::Font*  font )
        : SnapshotAnnotator( pm, win, pos.x, pos.y, size.x, size.y, font )
    {
    }

    pp::ControlsTheme
    GetControlsTheme() const override final
    {
        return theme_;
    }

    virtual void
    ShapeChanged( pp::Shape* shape ) override final
    {
    }

    virtual void
    SetSelectedShape( pp::Shape* shape ) override final
    {
        selected_shape_ = shape;
    }

    virtual pp::Shape*
    GetSelectedShape() const override final
    {
        return selected_shape_;
    }

    void
    AddShape( pp::Shape* shape ) override final
    {
        shapes_.insert( std::make_pair( shape, shape ) );
    }

    void
    DelShape( pp::Shape* shape ) override final
    {
        shapes_.erase( shape );
    }

    virtual dr4::Window*
    GetWindow() override final
    {
        return window_;
    }

    bool
    onIdle( const hui::Event& event ) override final
    {
        event.apply( &tool_panel_ );

        return false;
    }

    bool
    onKeyPress( const hui::Event& event ) override final
    {
        for ( auto& tool : tools_ )
        {
            if ( tool->OnKeyDown( event.info.key ) )
            {
                return true;
            }
        }

        if ( selected_shape_ != nullptr )
        {
            if ( selected_shape_->OnKeyDown( event.info.key ) )
            {
                return true;
            }
        }

        if ( event.info.key.sym == dr4::KEYCODE_ESCAPE )
        {
            for ( auto& tool : tools_ )
            {
                if ( tool->IsCurrentlyDrawing() )
                {
                    tool->OnBreak();
                    return true;
                }
            }

            shapes_.clear();

            selected_shape_ = nullptr;

            active_ = !active_;
            return true;
        }

        // if ( active_ )
        // {
        //     switch ( event.info.key.sym )
        //     {
        //         case dr4::KEYCODE_R:
        //             single_tool_->OnStart();
        //             return true;
        //         case dr4::KEYCODE_DELETE:
        //             DelShape( selected_shape_ );
        //             selected_shape_ = nullptr;
        //         default:
        //             break;
        //     }
        // }

        return false;
    }

    bool
    onMousePress( const hui::Event& event ) override final
    {
        if ( !active_ )
        {
            return false;
        }

        hui::MousePressEvent evt;
        evt.info.mouseButton.button = event.info.mouseButton.button;
        evt.info.mouseButton.pos    = { event.info.mouseButton.pos.x - getAbsPos().x,
                                        event.info.mouseButton.pos.y - getAbsPos().y };

        // dr4::Event::MouseButton evt = { event.info.mouseButton.button,
        //                                 { event.info.mouseButton.pos.x - getAbsPos().x,
        //                                   event.info.mouseButton.pos.y - getAbsPos().y } };

        if ( !onMe( event.info.mouseButton.pos ) )
        {
            selected_shape_ = nullptr;
            tool_panel_.setActiveTool( std::nullopt );
            return true;
        }

        // std::cerr << "Giving to tool_panel" << std::endl;

        if ( evt.apply( &tool_panel_ ) )
        {
            // if ( tool_panel_.getActiveToolIdx().has_value() )
            // {
            // std::cerr << "ActiveToolIdx = " << tool_panel_.getActiveToolIdx().value()
            //   << std::endl;
            // } else
            // {
            // std::cerr << "ActiveToolIdx = " << "None" << std::endl;
            // }

            return true;
        }

        // std::cerr << "ToolPanel didn't took" << std::endl;

        if ( selected_shape_ != nullptr )
        {
            if ( selected_shape_->OnMouseDown( evt.info.mouseButton ) )
            {
                return true;
            }
        }

        if ( tool_panel_.getActiveToolIdx().has_value() )
        {
            auto& active_tool = tools_[tool_panel_.getActiveToolIdx().value()];
            if ( active_tool->IsCurrentlyDrawing() )
            {
                if ( active_tool->OnMouseDown( evt.info.mouseButton ) )
                {
                    return true;
                }
            }
            // tools_[tool_panel_.getActiveToolIdx().value()]->OnMouseDown( evt.info.mouseButton );
        }

        //         for ( auto& tool : tools_ )
        //         {
        //             // std::cerr << tool->IsCurrentlyDrawing() << std::endl;
        //
        //             if ( !tool->IsCurrentlyDrawing() )
        //             {
        //                 continue;
        //             }
        //
        //             // std::cerr << "tool->OnMouseDown" << std::endl;
        //
        //             if ( tool->OnMouseDown( evt.info.mouseButton ) )
        //             {
        //                 return true;
        //             }
        //         }

        for ( auto& pair : shapes_ )
        {
            auto& shape = *pair.second;

            if ( shape.OnMouseDown( evt.info.mouseButton ) )
            {
                return true;
            }
        }

        // std::cerr << "Shapes didn't took" << std::endl;

        if ( tool_panel_.getActiveToolIdx().has_value() )
        {
            auto& active_tool = tools_[tool_panel_.getActiveToolIdx().value()];
            if ( active_tool->OnMouseDown( evt.info.mouseButton ) )
            {
                return true;
            }
        }

        return true;
    }

    bool
    onTextEnter( const hui::Event& event ) override final
    {
        for ( auto& tool : tools_ )
        {
            if ( tool->OnText( event.info.text ) )
            {
                return true;
            }
        }

        return false;
    }

    bool
    onMouseRelease( const hui::Event& event ) override final
    {
        if ( !active_ )
        {
            return false;
        }

        if ( event.apply( &tool_panel_ ) )
        {
            return true;
        }

        dr4::Event::MouseButton evt = { event.info.mouseButton.button,
                                        { event.info.mouseButton.pos.x - getAbsPos().x,
                                          event.info.mouseButton.pos.y - getAbsPos().y } };

        if ( selected_shape_ != nullptr )
        {
            if ( selected_shape_->OnMouseUp( evt ) )
            {
                return true;
            }
        }

        for ( auto& tool : tools_ )
        {
            if ( tool->OnMouseUp( evt ) )
            {
                return true;
            }
        }

        return false;
    }

    bool
    onMouseMove( const hui::Event& event ) override final
    {
        if ( !active_ )
        {
            return false;
        }

        if ( event.apply( &tool_panel_ ) )
        {
            return true;
        }

        dr4::Event::MouseMove evt = { { event.info.mouseMove.pos.x - getAbsPos().x,
                                        event.info.mouseMove.pos.y - getAbsPos().y },
                                      event.info.mouseMove.rel };

        if ( selected_shape_ != nullptr )
        {
            if ( selected_shape_->OnMouseMove( evt ) )
            {
                return true;
            }
        }

        for ( auto& tool : tools_ )
        {
            if ( tool->OnMouseMove( evt ) )
            {
                return true;
            }
        }

        return false;
    }

  private:
    void
    RedrawMyTexture() const override final
    {
        // // std::cerr << active_ << std::endl;

        if ( !active_ )
        {
            return;
        }

        border_->DrawOn( *texture_ );

        for ( const auto& pair : shapes_ )
        {
            auto& shape = *pair.second;

            shape.DrawOn( *texture_ );
        }

        tool_panel_.Redraw();

        // // std::cerr << "Drawing border.." << std::endl;
    }

  private:
    std::vector<std::unique_ptr<pp::Tool>> tools_;

    bool active_ = false;

    std::unique_ptr<dr4::Rectangle> border_;

    pp::Tool* single_tool_;

    view::ToolPanel tool_panel_;

    std::unordered_map<pp::Shape*, std::unique_ptr<pp::Shape>> shapes_;

    pp::ControlsTheme theme_;

    pp::Shape* selected_shape_;
};

} // namespace view
} // namespace zemax
