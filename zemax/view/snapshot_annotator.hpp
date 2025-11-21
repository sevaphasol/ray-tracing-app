#pragma once

#include "custom-hui-impl/widget.hpp"
#include "dr4/event.hpp"
#include "dr4/keycodes.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include "pp/canvas.hpp"
#include "pp/shape.hpp"
#include "pp/tool.hpp"
#include <cassert>
#include <future>
#include <memory>
#include <unordered_map>
#include <utility>

namespace zemax {
namespace view {

class SnapshotAnnotator : public hui::Widget, public pp::Canvas {
  public:
    SnapshotAnnotator( cum::PluginManager* pm, float x, float y, float w, float h )
        : hui::Widget( pm, x, y, w, h ), pp::Canvas(), active_( false )
    {
        tools_ = pm->getPluginPP()->CreateTools( this );

        assert( tools_.size() == 1 );

        single_tool_ = tools_[0].get();

        border_.reset( pm->getWindow()->CreateRectangle() );

        border_->SetBorderColor( { 0, 118, 185, 255 } );
        border_->SetBorderThickness( -2 );
        border_->SetFillColor( { 0, 0, 0, 128 } );
        border_->SetSize( { w, h } );

        theme_ = { { 0, 0, 0, 0 },
                   { 255, 0, 0, 255 },
                   { 255, 0, 0, 255 },
                   12,
                   { 128, 128, 128, 255 } };
    }

    SnapshotAnnotator( cum::PluginManager* pm, const dr4::Vec2f& pos, const dr4::Vec2f& size )
        : SnapshotAnnotator( pm, pos.x, pos.y, size.x, size.y )
    {
    }

    pp::ControlsTheme
    GetControlsTheme() const override final
    {
        return theme_;
    }

    pp::State*
    GetState() override final
    {
        return &state_;
    }

    size_t
    AddShape( pp::Shape* shape ) override final
    {
        shapes_.insert( std::make_pair( reinterpret_cast<size_t>( shape ), shape ) );

        return reinterpret_cast<size_t>( shape );
    }

    void
    DelShape( size_t id ) override final
    {
        shapes_.erase( id );
    }

    virtual dr4::Window*
    GetWindow() override final
    {
        return pm_->getWindow();
    }

    bool
    onKeyPress( const hui::Event& event ) override final
    {
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

            state_.selectedShape = nullptr;

            active_ = !active_;
            return true;
        }

        if ( active_ )
        {
            switch ( event.info.key.sym )
            {
                case dr4::KEYCODE_R:
                    single_tool_->OnStart();
                    return true;
                case dr4::KEYCODE_DELETE:
                    DelShape( size_t( state_.selectedShape ) );
                    state_.selectedShape = nullptr;
                default:
                    break;
            }
        }

        return false;
    }

    bool
    onMousePress( const hui::Event& event ) override final
    {
        if ( !active_ )
        {
            return false;
        }

        dr4::Event::MouseButton evt = { event.info.mouseButton.button,
                                        { event.info.mouseButton.pos.x - getAbsPos().x,
                                          event.info.mouseButton.pos.y - getAbsPos().y } };

        if ( !onMe( event.info.mouseButton.pos ) )
        {
            state_.selectedShape = nullptr;
            state_.selectedTool  = nullptr;
            return false;
        }

        if ( state_.selectedShape != nullptr )
        {
            if ( state_.selectedShape->OnMouseDown( evt ) )
            {
                return true;
            }
        }

        for ( auto& tool : tools_ )
        {
            std::cerr << tool->IsCurrentlyDrawing() << std::endl;

            if ( !tool->IsCurrentlyDrawing() )
            {
                continue;
            }

            std::cerr << "tool->OnMouseDown" << std::endl;

            if ( tool->OnMouseDown( evt ) )
            {
                return true;
            }
        }

        std::cerr << "Propagating to shapes" << std::endl;

        for ( auto& pair : shapes_ )
        {
            auto& shape = *pair.second;

            if ( shape.OnMouseDown( evt ) )
            {
                return true;
            }
        }

        std::cerr << "Shapes didn't took" << std::endl;

        for ( auto& tool : tools_ )
        {
            if ( tool->OnMouseDown( evt ) )
            {
                state_.selectedShape = nullptr;
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

        dr4::Event::MouseButton evt = { event.info.mouseButton.button,
                                        { event.info.mouseButton.pos.x - getAbsPos().x,
                                          event.info.mouseButton.pos.y - getAbsPos().y } };

        if ( state_.selectedShape != nullptr )
        {
            if ( state_.selectedShape->OnMouseUp( evt ) )
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

        dr4::Event::MouseMove evt = { { event.info.mouseMove.pos.x - getAbsPos().x,
                                        event.info.mouseMove.pos.y - getAbsPos().y },
                                      event.info.mouseMove.rel };

        if ( state_.selectedShape != nullptr )
        {
            if ( state_.selectedShape->OnMouseMove( evt ) )
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
        // std::cerr << active_ << std::endl;

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

        // std::cerr << "Drawing border.." << std::endl;
    }

  private:
    bool active_ = false;

    std::unique_ptr<dr4::Rectangle> border_;

    pp::Tool* single_tool_;

    std::vector<std::unique_ptr<pp::Tool>> tools_;

    std::unordered_map<size_t, std::unique_ptr<pp::Shape>> shapes_;

    pp::ControlsTheme theme_;

    pp::State state_;
};

} // namespace view
} // namespace zemax
