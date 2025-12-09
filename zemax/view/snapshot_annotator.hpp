#pragma once

#include "cum/ifc/pp.hpp"
#include "cum/manager.hpp"
#include "custom-hui-impl/event.hpp"
#include "custom-hui-impl/widget.hpp"
#include "dr4/event.hpp"
#include "dr4/keycodes.hpp"
#include "dr4/texture.hpp"
#include "pp/canvas.hpp"
#include "pp/shape.hpp"
#include "pp/tool.hpp"
#include "zemax/view/tool_panel.hpp"
#include <cassert>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>
#include <string>

namespace zemax {
namespace view {

class SnapshotAnnotator : public hui::Widget, public pp::Canvas {
  public:
    SnapshotAnnotator( hui::WindowManager* wm, float x, float y, float w, float h )
        : hui::Widget( wm, x, y, w, h ),
          pp::Canvas(),
          active_( false ),
          tool_panel_( wm, 10, 100, 200 )
    {
        setDraggable( false );
        refreshPlugins();
        tool_panel_.setParent( this );

        border_.reset( wm->getWindow()->CreateRectangle() );

        border_->SetBorderColor( { 0, 118, 185, 255 } );
        border_->SetBorderThickness( -2 );
        border_->SetFillColor( { 0, 0, 0, 128 } );
        border_->SetSize( { w, h } );

        theme_ = { { 0, 0, 0, 0 },
                   { 0, 118, 185, 255 },
                   { 118, 0, 0, 255 },
                   { 255, 255, 255, 255 },
                   18,
                   { 128, 128, 128, 255 } };
    }

    SnapshotAnnotator( hui::WindowManager* wm, const dr4::Vec2f& pos, const dr4::Vec2f& size )
        : SnapshotAnnotator( wm, pos.x, pos.y, size.x, size.y )
    {
    }

    void
    attachTo( hui::Widget* parent )
    {
        attachTo( parent, { 0.0f, 0.0f }, parent->getSize() );
    }

    void
    attachTo( hui::Widget* parent, const dr4::Vec2f& rel_pos, const dr4::Vec2f& size )
    {
        setParent( parent );
        setRelPos( rel_pos );
        setSize( size );
    }

    void
    refreshPlugins()
    {
        plugin_tools_.clear();
        active_tools_  = nullptr;
        bool active_still_present = false;

        for ( auto* plg : wm_->getPluginManager()->GetAllOfType<cum::PPToolPlugin>() )
        {
            PluginTools bucket;
            bucket.plugin = plg;
            bucket.tools  = plg->CreateTools( this );
            plugin_tools_.push_back( std::move( bucket ) );
            if ( plg == active_plugin_ )
            {
                active_still_present = true;
            }
        }

        if ( !plugin_tools_.empty() )
        {
            if ( !active_plugin_ || !active_still_present )
            {
                active_plugin_ = plugin_tools_.back().plugin;
            }
        } else
        {
            active_plugin_ = nullptr;
        }

        rebuildToolPanel();
    }

    void
    setActivePlugin( cum::PPToolPlugin* plugin )
    {
        active_plugin_ = plugin;
        rebuildToolPanel();
    }

    cum::PPToolPlugin*
    getActivePlugin() const
    {
        return active_plugin_;
    }

    std::vector<std::pair<std::string, cum::PPToolPlugin*>>
    listPlugins() const
    {
        std::vector<std::pair<std::string, cum::PPToolPlugin*>> res;
        res.reserve( plugin_tools_.size() );
        for ( const auto& p : plugin_tools_ )
        {
            res.emplace_back( p.plugin->GetName(), p.plugin );
        }
        return res;
    }

    void
    show()
    {
        active_ = true;
    }

    void
    hide()
    {
        active_ = false;
        tool_panel_.setActiveTool( std::nullopt );
        shapes_.clear();
        selected_shape_ = nullptr;
    }

    bool
    isVisible() const
    {
        return active_;
    }

    void
    setSize( const dr4::Vec2f& size ) override
    {
        hui::Widget::setSize( size );
        if ( border_ )
        {
            border_->SetSize( size );
        }
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
        return wm_->getWindow();
    }

    bool
    onIdle( const hui::Event& event ) override final
    {
        if ( !active_ )
        {
            return false;
        }
        event.apply( &tool_panel_ );

        return false;
    }

    bool
    onKeyPress( const hui::Event& event ) override final
    {
        if ( !active_ )
        {
            return false;
        }
        if ( active_tools_ )
        {
            for ( auto& tool : *active_tools_ )
            {
                if ( tool->OnKeyDown( event.info.key ) )
                {
                    return true;
                }
            }
        }

        if ( selected_shape_ != nullptr )
        {
            if ( selected_shape_->OnKeyDown( event.info.key ) )
            {
                return true;
            }
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

        // // std::cerr << "Giving to tool_panel" << std::endl;

        if ( event.apply( &tool_panel_ ) )
        {
            // if ( tool_panel_.getActiveToolIdx().has_value() )
            // {
            // // std::cerr << "ActiveToolIdx = " << tool_panel_.getActiveToolIdx().value()
            //   << std::endl;
            // } else
            // {
            // // std::cerr << "ActiveToolIdx = " << "None" << std::endl;
            // }

            return true;
        }

        // // std::cerr << "ToolPanel didn't took" << std::endl;

        if ( selected_shape_ != nullptr )
        {
            if ( selected_shape_->OnMouseDown( evt.info.mouseButton ) )
            {
                return true;
            }
        }

        if ( active_tools_ && tool_panel_.getActiveToolIdx().has_value() )
        {
            auto& active_tool = ( *active_tools_ )[tool_panel_.getActiveToolIdx().value()];
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
        //             // // std::cerr << tool->IsCurrentlyDrawing() << std::endl;
        //
        //             if ( !tool->IsCurrentlyDrawing() )
        //             {
        //                 continue;
        //             }
        //
        //             // // std::cerr << "tool->OnMouseDown" << std::endl;
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

        // // std::cerr << "Shapes didn't took" << std::endl;

        if ( active_tools_ && tool_panel_.getActiveToolIdx().has_value() )
        {
            auto& active_tool = ( *active_tools_ )[tool_panel_.getActiveToolIdx().value()];
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
        if ( !active_tools_ )
        {
            return false;
        }

        for ( auto& tool : *active_tools_ )
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

        if ( !active_tools_ )
        {
            return false;
        }

        for ( auto& tool : *active_tools_ )
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

        if ( !active_tools_ )
        {
            return false;
        }

        for ( auto& tool : *active_tools_ )
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
    rebuildToolPanel()
    {
        active_tools_ = nullptr;
        for ( auto& entry : plugin_tools_ )
        {
            if ( entry.plugin == active_plugin_ )
            {
                active_tools_ = &entry.tools;
                break;
            }
        }

        tool_panel_.addTools( active_tools_ );
    }

    void
    RedrawMyTexture() const override final
    {
        // // // std::cerr << active_ << std::endl;

        if ( !active_ )
        {
            return;
        }

        texture_->Clear( { 0, 0, 0, 0 } );
        border_->DrawOn( *texture_ );

        for ( const auto& pair : shapes_ )
        {
            auto& shape = *pair.second;

            shape.DrawOn( *texture_ );
        }

        tool_panel_.Redraw();

        // // // std::cerr << "Drawing border.." << std::endl;
    }

  private:
    struct PluginTools
    {
        cum::PPToolPlugin*                     plugin = nullptr;
        std::vector<std::unique_ptr<pp::Tool>> tools;
    };

    std::vector<PluginTools>                 plugin_tools_;
    std::vector<std::unique_ptr<pp::Tool>>*  active_tools_  = nullptr;
    cum::PPToolPlugin*                       active_plugin_ = nullptr;

    bool active_ = false;

    std::unique_ptr<dr4::Rectangle> border_;

    pp::Tool* single_tool_;

    view::ToolPanel tool_panel_;

    std::unordered_map<pp::Shape*, std::unique_ptr<pp::Shape>> shapes_;

    pp::ControlsTheme theme_;

    pp::Shape* selected_shape_ = nullptr;
};

} // namespace view
} // namespace zemax
