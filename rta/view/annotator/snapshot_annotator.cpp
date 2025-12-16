#include "rta/view/annotator/snapshot_annotator.hpp"
#include "cum/ifc/pp.hpp"
#include "cum/manager.hpp"
#include "custom-hui/event.hpp"
#include "custom-hui/widget.hpp"
#include "dr4/event.hpp"
#include "dr4/texture.hpp"
#include "pp/canvas.hpp"
#include "pp/shape.hpp"
#include "pp/tool.hpp"
#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rta {
namespace view {

rta::view::SnapshotAnnotator::SnapshotAnnotator( hui::WindowManager* wm,
                                                 float               x,
                                                 float               y,
                                                 float               w,
                                                 float               h )
    : hui::Widget( wm, x, y, w, h ),
      pp::Canvas(),
      active_( false ),
      tool_panel_( wm, 10, 100, 200 ),
      theme_( { { 0, 0, 0, 0 },
                { 0, 118, 185, 255 },
                { 118, 0, 0, 255 },
                { 255, 255, 255, 255 },
                18,
                { 128, 128, 128, 255 } } ),
      color_picker_( wm,
                     { 80.0f, 20.0f },
                     theme_.shapeBorderColor,
                     [this]( const dr4::Color& color ) { updateThemeColor( color ); } ),
      color_picker_visible_( false )
{
    setDraggable( false );
    refreshPlugins();
    tool_panel_.setParent( this );
    color_picker_.setParent( this );

    border_.reset( wm->getWindow()->CreateRectangle() );

    border_->SetBorderColor( { 0, 118, 185, 255 } );
    border_->SetBorderThickness( -2 );
    border_->SetFillColor( { 0, 0, 0, 128 } );
    border_->SetSize( { w, h } );
}

rta::view::SnapshotAnnotator::SnapshotAnnotator( hui::WindowManager* wm,
                                                 const dr4::Vec2f&   pos,
                                                 const dr4::Vec2f&   size )
    : SnapshotAnnotator( wm, pos.x, pos.y, size.x, size.y )
{
}

void
rta::view::SnapshotAnnotator::attachTo( hui::Widget* parent )
{
    attachTo( parent, { 0.0f, 0.0f }, parent->getSize() );
}

void
rta::view::SnapshotAnnotator::attachTo( hui::Widget*      parent,
                                        const dr4::Vec2f& rel_pos,
                                        const dr4::Vec2f& size )
{
    setParent( parent );
    setRelPos( rel_pos );
    setSize( size );
}

void
rta::view::SnapshotAnnotator::refreshPlugins()
{
    plugin_tools_.clear();
    bool active_still_present = false;

    for ( auto* plg : wm_->getPluginManager()->GetAllOfType<cum::PPToolPlugin>() )
    {
        PluginTools bucket;
        bucket.plugin = plg;
        bucket.tools  = plg->CreateTools( this );
        bucket.enabled.assign( bucket.tools.size(), true );
        plugin_tools_.push_back( std::move( bucket ) );
        if ( plg == active_plugin_ )
        {
            active_still_present = true;
        }
    }

    if ( !plugin_tools_.empty() )
    {
        if ( ( active_plugin_ == nullptr ) || !active_still_present )
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
rta::view::SnapshotAnnotator::setActivePlugin( cum::PPToolPlugin* plugin )
{
    active_plugin_ = plugin;
    rebuildToolPanel();
}

cum::PPToolPlugin*
rta::view::SnapshotAnnotator::getActivePlugin() const
{
    return active_plugin_;
}

void
rta::view::SnapshotAnnotator::removePlugin( cum::PPToolPlugin* plugin )
{
    //     std::cerr << plugin_tools_.size() << std::endl;
    //
    //     for ( auto it = plugin_tools_.begin(); it != plugin_tools_.end(); ++it )
    //     {
    //         if ( it->plugin == plugin )
    //         {
    //             plugin_tools_.erase( it );
    //         }
    //     }
    //
    //     plugin_tools_.erase(
    //         std::remove_if( plugin_tools_.begin(),
    //                         plugin_tools_.end(),
    //                         [plugin]( const PluginTools& p ) { return p.plugin == plugin; } ),
    //         plugin_tools_.end() );
    //
    //     std::cerr << plugin_tools_.size() << std::endl;
    //
    //     if ( active_plugin_ == plugin )
    //     {
    //         active_plugin_ = plugin_tools_.empty() ? nullptr : plugin_tools_.back().plugin;
    //     }
    //
    //     rebuildToolPanel();
}

std::vector<std::pair<std::string, cum::PPToolPlugin*>>
rta::view::SnapshotAnnotator::listPlugins() const
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
rta::view::SnapshotAnnotator::show()
{
    active_ = true;
}

void
rta::view::SnapshotAnnotator::hide()
{
    active_ = false;
    tool_panel_.setActiveTool( std::nullopt );
    shapes_.clear();
    selected_shape_ = nullptr;
}

bool
rta::view::SnapshotAnnotator::isVisible() const
{
    return active_;
}

bool
rta::view::SnapshotAnnotator::hasActivePlugin() const
{
    return active_plugin_ != nullptr;
}

std::vector<rta::view::SnapshotAnnotator::ToolInfo>
rta::view::SnapshotAnnotator::listActivePluginTools() const
{
    std::vector<ToolInfo> res;
    if ( active_plugin_ == nullptr )
    {
        return res;
    }
    for ( const auto& entry : plugin_tools_ )
    {
        if ( entry.plugin != active_plugin_ )
        {
            continue;
        }
        res.reserve( entry.tools.size() );
        for ( size_t i = 0; i < entry.tools.size(); ++i )
        {
            const auto& t = entry.tools[i];
            res.push_back(
                { i, std::string( t->Name() ), std::string( t->Icon() ), entry.enabled[i] } );
        }
        break;
    }
    return res;
}

void
rta::view::SnapshotAnnotator::toggleToolEnabled( size_t idx )
{
    for ( auto& entry : plugin_tools_ )
    {
        if ( entry.plugin != active_plugin_ )
        {
            continue;
        }
        if ( idx >= entry.enabled.size() )
        {
            return;
        }
        entry.enabled[idx] = !entry.enabled[idx];
        rebuildToolPanel();
        return;
    }
}

bool
rta::view::SnapshotAnnotator::isColorPickerVisible() const
{
    return color_picker_visible_;
}

void
rta::view::SnapshotAnnotator::setColorPickerVisible( bool visible )
{
    color_picker_visible_ = visible;
}

void
rta::view::SnapshotAnnotator::setSize( const dr4::Vec2f& size )
{
    hui::Widget::setSize( size );
    if ( border_ )
    {
        border_->SetSize( size );
    }
}

pp::ControlsTheme
rta::view::SnapshotAnnotator::GetControlsTheme() const
{
    return theme_;
}

void
rta::view::SnapshotAnnotator::ShapeChanged( pp::Shape* shape )
{
}

void
rta::view::SnapshotAnnotator::SetSelectedShape( pp::Shape* shape )
{
    selected_shape_ = shape;
}

pp::Shape*
rta::view::SnapshotAnnotator::GetSelectedShape() const
{
    return selected_shape_;
}

void
rta::view::SnapshotAnnotator::AddShape( pp::Shape* shape )
{
    shapes_.insert( std::make_pair( shape, shape ) );
}

void
rta::view::SnapshotAnnotator::DelShape( pp::Shape* shape )
{
    shapes_.erase( shape );
}

dr4::Window*
rta::view::SnapshotAnnotator::GetWindow()
{
    return wm_->getWindow();
}

bool
rta::view::SnapshotAnnotator::onIdle( const hui::Event& event )
{
    if ( !active_ )
    {
        return false;
    }
    if ( hasTools() )
    {
        event.apply( &tool_panel_ );
    }
    if ( color_picker_visible_ )
    {
        event.apply( &color_picker_ );
    }

    return false;
}

bool
rta::view::SnapshotAnnotator::onKeyPress( const hui::Event& event )
{
    if ( !active_ )
    {
        return false;
    }
    for ( auto& tool : visible_tools_ )
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
rta::view::SnapshotAnnotator::onMousePress( const hui::Event& event )
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
        return false;
    }

    // // std::cerr << "Giving to tool_panel" << std::endl;

    if ( hasTools() && event.apply( &tool_panel_ ) )
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

    if ( color_picker_visible_ && event.apply( &color_picker_ ) )
    {
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

    if ( !visible_tools_.empty() && tool_panel_.getActiveToolIdx().has_value() )
    {
        auto& active_tool = visible_tools_[tool_panel_.getActiveToolIdx().value()];
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

    if ( !visible_tools_.empty() && tool_panel_.getActiveToolIdx().has_value() )
    {
        auto& active_tool = visible_tools_[tool_panel_.getActiveToolIdx().value()];
        if ( active_tool->OnMouseDown( evt.info.mouseButton ) )
        {
            return true;
        }
    }

    return true;
}

bool
rta::view::SnapshotAnnotator::onTextEnter( const hui::Event& event )
{
    if ( visible_tools_.empty() )
    {
        return false;
    }

    for ( auto& tool : visible_tools_ )
    {
        if ( tool->OnText( event.info.text ) )
        {
            return true;
        }
    }

    return false;
}

bool
rta::view::SnapshotAnnotator::onMouseRelease( const hui::Event& event )
{
    if ( !active_ )
    {
        return false;
    }

    if ( hasTools() && event.apply( &tool_panel_ ) )
    {
        return true;
    }

    if ( color_picker_visible_ && event.apply( &color_picker_ ) )
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

    if ( visible_tools_.empty() )
    {
        return false;
    }

    for ( auto& tool : visible_tools_ )
    {
        if ( tool->OnMouseUp( evt ) )
        {
            return true;
        }
    }

    return false;
}

bool
rta::view::SnapshotAnnotator::onMouseMove( const hui::Event& event )
{
    if ( !active_ )
    {
        return false;
    }

    if ( hasTools() && event.apply( &tool_panel_ ) )
    {
        return true;
    }

    if ( color_picker_visible_ && event.apply( &color_picker_ ) )
    {
        return true;
    }

    dr4::Event::MouseMove evt = {
        { event.info.mouseMove.pos.x - getAbsPos().x, event.info.mouseMove.pos.y - getAbsPos().y },
        event.info.mouseMove.rel };

    if ( selected_shape_ != nullptr )
    {
        if ( selected_shape_->OnMouseMove( evt ) )
        {
            return true;
        }
    }

    if ( visible_tools_.empty() )
    {
        return false;
    }

    for ( auto& tool : visible_tools_ )
    {
        if ( tool->OnMouseMove( evt ) )
        {
            return true;
        }
    }

    return false;
}

void
rta::view::SnapshotAnnotator::updateThemeColor( const dr4::Color& color )
{
    theme_.shapeBorderColor = color;
    theme_.shapeFillColor   = color;
}

bool
rta::view::SnapshotAnnotator::hasTools() const
{
    return !visible_tools_.empty();
}

void
rta::view::SnapshotAnnotator::rebuildToolPanel()
{
    visible_tools_.clear();
    for ( auto& entry : plugin_tools_ )
    {
        if ( entry.plugin == active_plugin_ )
        {
            if ( entry.enabled.empty() )
            {
                entry.enabled.assign( entry.tools.size(), true );
            }
            for ( size_t i = 0; i < entry.tools.size(); ++i )
            {
                if ( i < entry.enabled.size() && entry.enabled[i] )
                {
                    visible_tools_.push_back( entry.tools[i].get() );
                }
            }
            break;
        }
    }

    tool_panel_.addTools( visible_tools_ );
}

void
rta::view::SnapshotAnnotator::RedrawMyTexture() const
{
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

    if ( hasTools() )
    {
        tool_panel_.Redraw();
    }
    if ( color_picker_visible_ )
    {
        color_picker_.Redraw();
    }
}

} // namespace view
} // namespace rta
