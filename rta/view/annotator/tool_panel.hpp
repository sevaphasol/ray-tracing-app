#pragma once

#include "hui/button.hpp"
#include "hui/window_manager.hpp"
#include "pp/tool.hpp"
#include <optional>
#include <vector>

namespace rta {
namespace view {

class ToolPanel : public hui::ContainerWidget {
  public:
    ToolPanel( hui::WindowManager* wm, float x, float y, float visible_height );

    void
    addTools( const std::vector<pp::Tool*>& tools );

    std::optional<size_t>
    getActiveToolIdx() const;
    void
    setActiveTool( std::optional<size_t> idx );

  private:
    std::optional<size_t>                     active_tool_idx_;
    std::vector<pp::Tool*>                    tools_;
    const dr4::Font*                          font_;
    float                                     button_size_;
    float                                     padding_;
    std::vector<std::unique_ptr<hui::Button>> buttons_;
    std::unique_ptr<dr4::Font>                icon_font_;
    bool                                      icon_font_loaded_ = false;

    bool
    propagateEventToChildren( const hui::Event& event ) override;
    void
    RedrawMyTexture() const override;
};

} // namespace view
} // namespace rta
