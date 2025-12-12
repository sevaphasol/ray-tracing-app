#pragma once

#include "hui/button.hpp"
#include "hui/dialog_box.hpp"
#include "hui/file_dialog_box.hpp"
#include "hui/message_box.hpp"
#include "hui/scrollable_buttons_list_widget.hpp"
#include "rta/view/snapshot_annotator.hpp"
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
                         const Theme&        theme = Theme::Default() )
        : PluginManagerDialog(
              wm, theme.pos.x, theme.pos.y, theme.size.x, theme.size.y, annotator, theme )
    {
    }

    PluginManagerDialog( hui::WindowManager* wm,
                         float               x,
                         float               y,
                         float               w,
                         float               h,
                         SnapshotAnnotator*  annotator,
                         const Theme&        theme = Theme::Default() )
        : DialogBox(
              wm,
              x,
              y,
              w,
              h,
              [wm]() { wm->popModal(); },
              "Plugins" ),
          annotator_( annotator ),
          theme_( theme ),
          choose_btn_( wm,
                       dr4::Vec2f{ 0.0f, 0.0f },
                       dr4::Vec2f{ 0.0f, 0.0f },
                       "Choose",
                       theme.action_button_theme ),
          delete_btn_( wm,
                       dr4::Vec2f{ 0.0f, 0.0f },
                       dr4::Vec2f{ 0.0f, 0.0f },
                       "Delete",
                       theme.action_button_theme )
    {
        theme_.pos  = { x, y };
        theme_.size = { w, h };

        buildList();
        choose_btn_.setParent( this );
        delete_btn_.setParent( this );

        choose_btn_.setOnClick( [this]() { chooseSelected(); } );
        delete_btn_.setOnClick( [this]() { deleteSelected(); } );

        layoutActionButtons();
        rebuild();
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        if ( list_ && event.apply( list_.get() ) )
            return true;
        if ( event.apply( &choose_btn_ ) )
            return true;
        if ( event.apply( &delete_btn_ ) )
            return true;
        return hui::DialogBox::propagateEventToChildren( event );
    }

    void
    RedrawMyTexture() const override
    {
        hui::DialogBox::RedrawMyTexture();
        if ( list_ )
        {
            list_->Redraw();
        }
        choose_btn_.Redraw();
        delete_btn_.Redraw();
    }

  private:
    void
    buildList()
    {
        const float inner_w = theme_.size.x - 2.0f * theme_.margin;
        const float inner_h =
            theme_.size.y - TopBarHeight - theme_.actions_row_height - 2.0f * theme_.margin;

        list_ = std::make_unique<hui::ScrollableButtonsListWidget>(
            wm_,
            dr4::Vec2f{ theme_.margin, TopBarHeight + theme_.margin },
            dr4::Vec2f{ inner_w - theme_.scrollbar_width, inner_h },
            theme_.scrollbar_width,
            theme_.list_items_spacing );
        list_->setParent( this );
    }

    void
    layoutActionButtons()
    {
        const float available_w =
            theme_.size.x - 2.0f * theme_.action_buttons_gap;
        const float btn_w = available_w * 0.5f - 0.5f * theme_.action_buttons_gap;
        const float y = theme_.size.y - theme_.actions_row_height +
                        0.5f * ( theme_.actions_row_height - theme_.action_button_height );

        choose_btn_.setSize( { btn_w, theme_.action_button_height } );
        delete_btn_.setSize( { btn_w, theme_.action_button_height } );

        choose_btn_.setRelPos( { theme_.action_buttons_gap, y } );
        delete_btn_.setRelPos( { theme_.action_buttons_gap + btn_w + theme_.action_buttons_gap, y } );
    }

    hui::Button::Theme
    rowTheme( bool active, bool selected ) const
    {
        const auto& colors = theme_.row_theme;
        dr4::Color  bg     = colors.idle_bg;

        if ( active )
        {
            bg = colors.active_bg;
        } else if ( selected )
        {
            bg = colors.selected_bg;
        }

        return hui::Button::Theme{ bg, colors.hover_bg, colors.press_bg, colors.font_color, colors.font_size };
    }

    float
    itemWidth() const
    {
        return std::max( 1.0f,
                         theme_.size.x - 2.0f * theme_.margin - theme_.scrollbar_width -
                             2.0f * theme_.list_item_padding );
    }

    void
    openFileDialog()
    {
        wm_->pushModal( std::make_unique<hui::FileDialogBox>(
            wm_,
            getRelPos().x + getSize().x + 10.0f,
            getRelPos().y,
            420.0f,
            170.0f,
            "Load Plugin",
            "plugins/libplugin_pp.so",
            [this]( const std::string& path ) {
                wm_->getPluginManager()->LoadFromFile( path );
                annotator_->refreshPlugins();
                auto plugins = annotator_->listPlugins();
                if ( !plugins.empty() )
                {
                    selected_ = plugins.back().second;
                }
                wm_->popModal();
                rebuild();
            },
            [this]() { wm_->popModal(); } ) );
    }

    void
    rebuild()
    {
        if ( !list_ )
        {
            return;
        }

        annotator_->refreshPlugins();
        auto plugins = annotator_->listPlugins();
        auto active  = annotator_->getActivePlugin();

        if ( !selected_ )
        {
            selected_ = active;
        }

        if ( selected_ && std::none_of( plugins.begin(), plugins.end(), [this]( auto& p ) {
                 return p.second == selected_;
             } ) )
        {
            selected_ = active;
        }

        list_->clearButtons();

        const dr4::Vec2f btn_size{ itemWidth(), theme_.list_item_height };

        auto add_btn = std::make_unique<hui::Button>(
            wm_, dr4::Vec2f{ 0.0f, 0.0f }, btn_size, "+", theme_.add_button_theme );
        add_btn->setOnClick( [this]() { openFileDialog(); } );
        list_->addButton( std::move( add_btn ) );

        for ( auto& entry : plugins )
        {
            auto* plugin = entry.second;
            auto  btn    = std::make_unique<hui::Button>(
                wm_,
                dr4::Vec2f{ 0.0f, 0.0f },
                btn_size,
                entry.first,
                rowTheme( plugin == active, plugin == selected_ ) );
            btn->setOnClick( [this, plugin]() {
                selected_ = plugin;
                rebuild();
            } );
            list_->addButton( std::move( btn ) );
        }

        if ( auto* buttons_list = list_->getButtonsList() )
        {
            buttons_list->rebuildLayout();
        }
    }

    void
    chooseSelected()
    {
        if ( selected_ )
        {
            annotator_->setActivePlugin( selected_ );
            rebuild();
        }
    }

    void
    deleteSelected()
    {
        if ( !selected_ )
        {
            return;
        }
        annotator_->removePlugin( selected_ );
        annotator_->refreshPlugins();
        auto plugins = annotator_->listPlugins();
        selected_    = plugins.empty() ? nullptr : plugins.front().second;
        rebuild();
    }

  private:
    SnapshotAnnotator*                             annotator_;
    Theme                                          theme_;
    std::unique_ptr<hui::ScrollableButtonsListWidget> list_;
    hui::Button                                    choose_btn_;
    hui::Button                                    delete_btn_;
    cum::PPToolPlugin*                             selected_ = nullptr;
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
                        const Theme&        theme = Theme::Default() )
        : ToolSelectorDialog(
              wm, theme.pos.x, theme.pos.y, theme.size.x, theme.size.y, annotator, theme )
    {
    }

    ToolSelectorDialog( hui::WindowManager* wm,
                        float               x,
                        float               y,
                        float               w,
                        float               h,
                        SnapshotAnnotator*  annotator,
                        const Theme&        theme = Theme::Default() )
        : DialogBox(
              wm,
              x,
              y,
              w,
              h,
              [wm]() { wm->popModal(); },
              "Tools" ),
          annotator_( annotator ),
          theme_( theme ),
          close_( wm, dr4::Vec2f{ 0.0f, 0.0f }, theme.close_button_size, "Close", theme.close_button_theme )
    {
        theme_.pos  = { x, y };
        theme_.size = { w, h };

        buildList();

        close_.setParent( this );
        close_.setOnClick( [this]() { wm_->popModal(); } );

        layoutCloseButton();
        rebuild();
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        if ( list_ && event.apply( list_.get() ) )
            return true;
        if ( event.apply( &close_ ) )
            return true;
        return DialogBox::propagateEventToChildren( event );
    }

    void
    RedrawMyTexture() const override
    {
        DialogBox::RedrawMyTexture();
        if ( list_ )
        {
            list_->Redraw();
        }
        close_.Redraw();
    }

  private:
    void
    buildList()
    {
        const float inner_w = theme_.size.x - 2.0f * theme_.margin;
        const float inner_h =
            theme_.size.y - TopBarHeight - theme_.actions_row_height - 2.0f * theme_.margin;

        list_ = std::make_unique<hui::ScrollableButtonsListWidget>(
            wm_,
            dr4::Vec2f{ theme_.margin, TopBarHeight + theme_.margin },
            dr4::Vec2f{ inner_w - theme_.scrollbar_width, inner_h },
            theme_.scrollbar_width,
            theme_.list_items_spacing );
        list_->setParent( this );
    }

    void
    layoutCloseButton()
    {
        const float y = theme_.size.y - theme_.actions_row_height +
                        0.5f * ( theme_.actions_row_height - theme_.close_button_size.y );
        const float x =
            theme_.size.x - theme_.close_button_padding - theme_.close_button_size.x;

        close_.setRelPos( { x, y } );
    }

    hui::Button::Theme
    rowTheme( bool enabled ) const
    {
        return enabled ? theme_.row_theme.enabled : theme_.row_theme.disabled;
    }

    float
    itemWidth() const
    {
        return std::max( 1.0f,
                         theme_.size.x - 2.0f * theme_.margin - theme_.scrollbar_width -
                             2.0f * theme_.list_item_padding );
    }

    void
    rebuild()
    {
        if ( !list_ || !annotator_ )
            return;

        list_->clearButtons();

        auto tools = annotator_->listActivePluginTools();
        const dr4::Vec2f btn_size{ itemWidth(), theme_.list_item_height };

        for ( auto& t : tools )
        {
            auto label = t.icon.empty() ? t.name : ( std::string( t.icon ) + " " + t.name );
            auto btn =
                std::make_unique<hui::Button>( wm_,
                                               dr4::Vec2f{ 0.0f, 0.0f },
                                               btn_size,
                                               label,
                                               rowTheme( t.enabled ) );
            btn->setOnClick( [this, idx = t.index]() {
                annotator_->toggleToolEnabled( idx );
                rebuild();
            } );
            list_->addButton( std::move( btn ) );
        }

        if ( auto* buttons_list = list_->getButtonsList() )
        {
            buttons_list->rebuildLayout();
        }
    }

  private:
    SnapshotAnnotator*                             annotator_;
    Theme                                          theme_;
    std::unique_ptr<hui::ScrollableButtonsListWidget> list_;
    hui::Button                                    close_;
};

} // namespace view
} // namespace rta
