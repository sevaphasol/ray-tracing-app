#pragma once

#include "cum/ifc/pp.hpp"
#include "custom-hui/event.hpp"
#include "custom-hui/widget.hpp"
#include "dr4/texture.hpp"
#include "pp/canvas.hpp"
#include "pp/shape.hpp"
#include "pp/tool.hpp"
#include "rta/view/annotator/tool_panel.hpp"
#include "rta/view/utils/rgb_picker.hpp"
#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rta {
namespace view {

class SnapshotAnnotator : public hui::Widget, public pp::Canvas {
  private:
    struct PluginTools
    {
        cum::PPToolPlugin*                     plugin = nullptr;
        std::vector<std::unique_ptr<pp::Tool>> tools;
        std::vector<bool>                      enabled;
    };

    std::vector<PluginTools> plugin_tools_;
    std::vector<pp::Tool*>   visible_tools_;
    cum::PPToolPlugin*       active_plugin_ = nullptr;

    bool active_ = false;

    std::unique_ptr<dr4::Rectangle> border_;

    pp::Tool* single_tool_;

    view::ToolPanel tool_panel_;

    std::unordered_map<pp::Shape*, std::unique_ptr<pp::Shape>> shapes_;

    pp::ControlsTheme theme_;

    RGBPicker color_picker_;
    bool      color_picker_visible_;

    pp::Shape* selected_shape_ = nullptr;

  public:
    explicit SnapshotAnnotator( hui::WindowManager* wm, float x, float y, float w, float h );

    explicit SnapshotAnnotator( hui::WindowManager* wm,
                                const dr4::Vec2f&   pos,
                                const dr4::Vec2f&   size );

    struct ToolInfo
    {
        size_t      index;
        std::string name;
        std::string icon;
        bool        enabled;
    };

    void
    attachTo( hui::Widget* parent );
    void
    attachTo( hui::Widget* parent, const dr4::Vec2f& rel_pos, const dr4::Vec2f& size );
    void
    refreshPlugins();
    void
    setActivePlugin( cum::PPToolPlugin* plugin );
    cum::PPToolPlugin*
    getActivePlugin() const;
    void
    removePlugin( cum::PPToolPlugin* plugin );
    std::vector<std::pair<std::string, cum::PPToolPlugin*>>
    listPlugins() const;
    void
    show();
    void
    hide();
    bool
    isVisible() const;
    bool
    hasActivePlugin() const;
    std::vector<ToolInfo>
    listActivePluginTools() const;
    void
    toggleToolEnabled( size_t idx );
    bool
    isColorPickerVisible() const;
    void
    setColorPickerVisible( bool visible );
    void
    setSize( const dr4::Vec2f& size ) override;
    pp::ControlsTheme
    GetControlsTheme() const override final;
    virtual void
    ShapeChanged( pp::Shape* shape ) override final;
    virtual void
    SetSelectedShape( pp::Shape* shape ) override final;
    virtual pp::Shape*
    GetSelectedShape() const override final;
    void
    AddShape( pp::Shape* shape ) override final;
    void
    DelShape( pp::Shape* shape ) override final;

    virtual dr4::Window*
    GetWindow() override final;
    bool
    onIdle( const hui::Event& event ) override final;
    bool
    onKeyPress( const hui::Event& event ) override final;
    bool
    onMousePress( const hui::Event& event ) override final;
    bool
    onTextEnter( const hui::Event& event ) override final;
    bool
    onMouseRelease( const hui::Event& event ) override final;
    bool
    onMouseMove( const hui::Event& event ) override final;

  private:
    void
    updateThemeColor( const dr4::Color& color );
    bool
    hasTools() const;
    void
    rebuildToolPanel();
    void
    RedrawMyTexture() const override final;
};

} // namespace view
} // namespace rta
