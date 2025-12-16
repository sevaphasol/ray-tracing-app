#pragma once

#include "custom-hui/button.hpp"
#include "custom-hui/dialog_box.hpp"
#include "custom-hui/file_dialog_box.hpp"
#include "custom-hui/message_box.hpp"
#include "custom-hui/scrollable_buttons_list_widget.hpp"
#include "rta/view/annotator/snapshot_annotator.hpp"
#include <algorithm>
#include <functional>
#include <memory>
#include <string>

namespace rta {
namespace view {

class PluginManagerDialog : public hui::DialogBox {
  public:
    struct Theme
    {
        dr4::Vec2f         pos                  = { 120.0f, 30.0f };
        dr4::Vec2f         size                 = { 340.0f, 320.0f };
        float              margin               = 8.0f;
        float              scrollbar_width      = 10.0f;
        float              list_item_height     = 28.0f;
        float              list_item_padding    = 6.0f;
        float              list_items_spacing   = 8.0f;
        float              actions_row_height   = 44.0f;
        float              action_buttons_gap   = 14.0f;
        float              action_button_height = 32.0f;
        hui::Button::Theme action_button_theme  = hui::Button::Theme::Default();
        hui::Button::Theme add_button_theme     = { { 35, 35, 35, 255 },
                                                    { 60, 60, 60, 255 },
                                                    { 80, 120, 20, 255 },
                                                    { 220, 220, 220, 255 },
                                                    16 };

        struct RowTheme
        {
            dr4::Color active_bg   = { 80, 130, 20, 255 };
            dr4::Color selected_bg = { 50, 80, 140, 255 };
            dr4::Color idle_bg     = { 30, 30, 30, 255 };
            dr4::Color hover_bg    = { 50, 70, 30, 255 };
            dr4::Color press_bg    = { 100, 150, 0, 255 };
            dr4::Color font_color  = { 230, 230, 230, 255 };
            float      font_size   = 15.0f;
        } row_theme;

        static Theme
        Default()
        {
            return Theme();
        }
    };

    PluginManagerDialog( hui::WindowManager* wm,
                         SnapshotAnnotator*  annotator,
                         const Theme&        theme = Theme::Default() );

    PluginManagerDialog( hui::WindowManager* wm,
                         float               x,
                         float               y,
                         float               w,
                         float               h,
                         SnapshotAnnotator*  annotator,
                         const Theme&        theme = Theme::Default() );

    bool
    propagateEventToChildren( const hui::Event& event ) override;

    void
    RedrawMyTexture() const override;

  private:
    void
    buildList();

    void
    layoutActionButtons();

    hui::Button::Theme
    rowTheme( bool active, bool selected ) const;

    float
    itemWidth() const;

    void
    openFileDialog();

    void
    rebuild();

    void
    chooseSelected();

    void
    deleteSelected();

  private:
    SnapshotAnnotator*                                annotator_;
    Theme                                             theme_;
    std::unique_ptr<hui::ScrollableButtonsListWidget> list_;
    hui::Button                                       choose_btn_;
    hui::Button                                       delete_btn_;
    cum::PPToolPlugin*                                selected_ = nullptr;
};

class ToolSelectorDialog : public hui::DialogBox {
  public:
    struct Theme
    {
        dr4::Vec2f         pos                  = { 120.0f, 30.0f };
        dr4::Vec2f         size                 = { 320.0f, 300.0f };
        float              margin               = 8.0f;
        float              scrollbar_width      = 10.0f;
        float              list_item_height     = 28.0f;
        float              list_item_padding    = 6.0f;
        float              list_items_spacing   = 8.0f;
        float              actions_row_height   = 44.0f;
        float              close_button_padding = 20.0f;
        dr4::Vec2f         close_button_size    = { 70.0f, 24.0f };
        hui::Button::Theme close_button_theme   = hui::Button::Theme::Default();

        struct RowTheme
        {
            hui::Button::Theme enabled  = { { 50, 100, 20, 255 },
                                            { 70, 130, 30, 255 },
                                            { 90, 150, 40, 255 },
                                            { 230, 230, 230, 255 },
                                            15 };
            hui::Button::Theme disabled = hui::Button::Theme::Default();
        } row_theme;

        static Theme
        Default()
        {
            return Theme();
        }
    };

    ToolSelectorDialog( hui::WindowManager* wm,
                        SnapshotAnnotator*  annotator,
                        const Theme&        theme = Theme::Default() );

    ToolSelectorDialog( hui::WindowManager* wm,
                        float               x,
                        float               y,
                        float               w,
                        float               h,
                        SnapshotAnnotator*  annotator,
                        const Theme&        theme = Theme::Default() );

    bool
    propagateEventToChildren( const hui::Event& event ) override;

    void
    RedrawMyTexture() const override;

  private:
    void
    buildList();
    void
    layoutCloseButton();
    hui::Button::Theme
    rowTheme( bool enabled ) const;
    float
    itemWidth() const;
    void
    rebuild();

  private:
    SnapshotAnnotator*                                annotator_;
    Theme                                             theme_;
    std::unique_ptr<hui::ScrollableButtonsListWidget> list_;
    hui::Button                                       close_;
};

} // namespace view
} // namespace rta
