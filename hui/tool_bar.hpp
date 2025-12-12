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
        dr4::Color background       = { 40, 40, 40, 255 };
        dr4::Color border           = { 80, 80, 80, 255 };
        float      border_thickness = -1.0f;
        dr4::Color text_color       = { 220, 220, 220, 255 };
        float      font_size        = 16.0f;
        dr4::Color hover_color      = { 60, 60, 60, 255 };
        float      padding_x        = 8.0f;
        float      padding_y        = 4.0f;

        static Theme
        Default()
        {
            return Theme();
        }
    };

    MenuPopup( WindowManager*               wm,
               const dr4::Vec2f&            pos,
               const std::string&           menu_name,
               const std::vector<MenuItem>& items,
               const Theme&                 theme = Theme::Default() );

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
    updateSize();
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
        dr4::Color background_color = { 45, 45, 45, 255 };
        dr4::Color hover_color      = { 60, 60, 60, 255 };
        dr4::Color font_color       = { 220, 220, 220, 255 };
        float      font_size        = 18.0f;
        float      label_padding_x  = 6.0f;
        float      padding_x        = 2.0f;
        float      padding_y        = 0.0f;
        float      height           = 35.0f;

        static Theme
        Default()
        {
            return Theme();
        }

        Theme
        changedHeight( float height_ )
        {
            height = height_;
            return *this;
        }
    };

    ToolBar( WindowManager* wm, float height );
    ToolBar( WindowManager* wm, const Theme& theme = Theme::Default() );

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
    WindowManager*                         wm_ = nullptr;
    std::unique_ptr<dr4::Rectangle>        background_;
    std::vector<MenuDef>                   menu_defs_;
    std::unique_ptr<HorizontalButtonsList> buttons_;
    Theme                                  theme_;
};

} // namespace hui
