#pragma once

#include "buttons_list.hpp"
#include "window_manager.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace hui {

struct MenuItem
{
    std::string           label;
    std::function<void()> on_click;
    std::vector<MenuItem> children;
};

class MenuPopup : public Widget {
  public:
    struct Theme
    {
        dr4::Color background;
        dr4::Color border;
        float      border_thickness;
        dr4::Color text_color;
        float      font_size;
        dr4::Color hover_color;
        float      padding_x;
        float      padding_y;

        Theme()
            : background( { 40, 40, 40, 255 } ),
              border( { 80, 80, 80, 255 } ),
              border_thickness( -1.0f ),
              text_color( { 220, 220, 220, 255 } ),
              font_size( 14.0f ),
              hover_color( { 60, 60, 60, 255 } ),
              padding_x( 8.0f ),
              padding_y( 4.0f )
        {
        }
    };

    MenuPopup( WindowManager*               wm,
               const dr4::Vec2f&            pos,
               const std::string&           menu_name,
               const std::vector<MenuItem>& items,
               const Theme&                 theme = Theme() );

    void
    RedrawMyTexture() const override;
    bool
    onMouseMove( const Event& event ) override;
    bool
    onMousePress( const Event& event ) override;
    bool
    onMouseRelease( const Event& event ) override;
    bool
    containsPoint( const dr4::Vec2f& pt ) const;
    void
    setItemLabel( size_t idx, const std::string& label );
    const std::string&
    menuName() const;

  private:
    void
    createButtons();

  private:
    std::vector<MenuItem>                items_;
    std::string                          menu_name_;
    std::unique_ptr<VerticalButtonsList> buttons_;
    std::unique_ptr<dr4::Rectangle>      background_;
    Theme                                theme_;
};

class ToolBar : public Widget {
  public:
    struct Theme
    {
        dr4::Color background_color;
        dr4::Color hover_color;
        dr4::Color font_color;
        float      font_size;
        float      padding;

        Theme()
            : background_color( { 45, 45, 45, 255 } ),
              hover_color( { 60, 60, 60, 255 } ),
              font_color( { 220, 220, 220, 255 } ),
              font_size( 15.0f ),
              padding( 6.0f )
        {
        }
    };

    ToolBar( WindowManager* wm, float height = 35.0f, const Theme& theme = Theme() );

    size_t
    addMenu( const std::string& name, std::vector<MenuItem> items );

    void
    setMenuItemLabel( const std::string& menu_name, size_t item_idx, const std::string& label );
    void
    RedrawMyTexture() const override;

    bool
    onMouseMove( const Event& event ) override;
    bool
    onMousePress( const Event& event ) override;
    bool
    onMouseRelease( const Event& event ) override;

  private:
    struct MenuDef
    {
        std::string           name;
        std::vector<MenuItem> items;
        Button*               button = nullptr;
    };

    void
    rebuildLayout();

  private:
    WindowManager*                         wm_     = nullptr;
    float                                  height_ = 28.0f;
    std::unique_ptr<dr4::Rectangle>        background_;
    std::vector<MenuDef>                   menu_defs_;
    std::unique_ptr<HorizontalButtonsList> buttons_;
    Theme                                  theme_;
};

} // namespace hui
