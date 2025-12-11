#pragma once

#include "custom-hui-impl/file_dialog_box.hpp"
#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/dialog_box.hpp"
#include "custom-hui-impl/scrollable_list_widget.hpp"
#include "custom-hui-impl/message_box.hpp"
#include "zemax/view/snapshot_annotator.hpp"
#include <algorithm>
#include <functional>
#include <memory>
#include <string>

namespace zemax {
namespace view {

class PluginManagerDialog : public hui::DialogBox {
  public:
    PluginManagerDialog( hui::WindowManager* wm,
                         float               x,
                         float               y,
                         float               w,
                         float               h,
                         SnapshotAnnotator*  annotator )
        : DialogBox( wm, x, y, w, h, [wm]() { wm->popModal(); }, "Plugins" ),
          annotator_( annotator ),
          list_( wm,
                 { 8.0f, TopBarHeight + 8.0f },
                 { w - 16.0f, h - TopBarHeight - 16.0f - buttons_row_h_ },
                 10.0f ),
          choose_btn_( wm,
                       { 14.0f, h - buttons_row_h_ + 6.0f },
                       { ( w - 14.0f * 3.0f ) * 0.5f, buttons_row_h_ - 12.0f },
                       "Choose",
                       hui::Button::DefaultTheme ),
          delete_btn_( wm,
                       { 14.0f + ( w - 14.0f * 3.0f ) * 0.5f + 14.0f,
                         h - buttons_row_h_ + 6.0f },
                       { ( w - 14.0f * 3.0f ) * 0.5f, buttons_row_h_ - 12.0f },
                       "Delete",
                       hui::Button::DefaultTheme )
    {
        list_.setParent( this );
        choose_btn_.setParent( this );
        delete_btn_.setParent( this );

        choose_btn_.setOnClick( [this]() { chooseSelected(); } );
        delete_btn_.setOnClick( [this]() { deleteSelected(); } );

        rebuild();
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        if ( event.apply( &list_ ) )
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
        list_.Redraw();
        choose_btn_.Redraw();
        delete_btn_.Redraw();
    }

  private:
    static hui::Button::Theme
    rowTheme( bool active, bool selected )
    {
        const dr4::Color active_bg{ 80, 130, 20, 255 };
        const dr4::Color selected_bg{ 50, 80, 140, 255 };
        const dr4::Color idle_bg{ 30, 30, 30, 255 };
        const dr4::Color hover_bg{ 50, 70, 30, 255 };
        const dr4::Color press_bg{ 100, 150, 0, 255 };

        dr4::Color bg = idle_bg;
        if ( active )
            bg = active_bg;
        else if ( selected )
            bg = selected_bg;

        return hui::Button::Theme{ bg, hover_bg, press_bg, { 230, 230, 230, 255 }, 15 };
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

        list_.clearItems();

        auto add_btn = std::make_unique<hui::Button>(
            wm_,
            dr4::Vec2f{ 0.0f, 0.0f },
            dr4::Vec2f{ list_.getSize().x - 12.0f, 28.0f },
            "+",
            hui::Button::Theme{ { 35, 35, 35, 255 },
                                { 60, 60, 60, 255 },
                                { 80, 120, 20, 255 },
                                { 220, 220, 220, 255 },
                                16 } );
        add_btn->setOnClick( [this]() { openFileDialog(); } );
        list_.addItem( std::move( add_btn ) );

        for ( auto& entry : plugins )
        {
            auto* plugin = entry.second;
            auto  btn    = std::make_unique<hui::Button>(
                wm_,
                dr4::Vec2f{ 0.0f, 0.0f },
                dr4::Vec2f{ list_.getSize().x - 12.0f, 28.0f },
                entry.first,
                rowTheme( plugin == active, plugin == selected_ ) );
            btn->setOnClick( [this, plugin]() {
                selected_ = plugin;
                rebuild();
            } );
            list_.addItem( std::move( btn ) );
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
    SnapshotAnnotator*        annotator_;
    hui::ScrollableListWidget list_;
    hui::Button               choose_btn_;
    hui::Button               delete_btn_;
    cum::PPToolPlugin*        selected_       = nullptr;
    static constexpr float    buttons_row_h_  = 44.0f;
};

class ToolSelectorDialog : public hui::DialogBox {
  public:
    ToolSelectorDialog( hui::WindowManager* wm,
                        float               x,
                        float               y,
                        float               w,
                        float               h,
                        SnapshotAnnotator*  annotator )
        : DialogBox( wm, x, y, w, h, [wm]() { wm->popModal(); }, "Tools" ),
          annotator_( annotator ),
          list_( wm,
                 { 8.0f, TopBarHeight + 8.0f },
                 { w - 16.0f, h - TopBarHeight - 16.0f - 44.0f },
                 10.0f ),
          close_( wm,
                  { w - 90.0f, h - 34.0f },
                  { 70.0f, 24.0f },
                  "Close",
                  hui::Button::DefaultTheme )
    {
        list_.setParent( this );
        close_.setParent( this );
        close_.setOnClick( [this]() { wm_->popModal(); } );
        rebuild();
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        if ( event.apply( &list_ ) )
            return true;
        if ( event.apply( &close_ ) )
            return true;
        return DialogBox::propagateEventToChildren( event );
    }

    void
    RedrawMyTexture() const override
    {
        DialogBox::RedrawMyTexture();
        list_.Redraw();
        close_.Redraw();
    }

  private:
    hui::Button::Theme
    rowTheme( bool enabled ) const
    {
        return enabled ? hui::Button::Theme{ { 50, 100, 20, 255 },
                                             { 70, 130, 30, 255 },
                                             { 90, 150, 40, 255 },
                                             { 230, 230, 230, 255 },
                                             15 }
                       : hui::Button::DefaultTheme;
    }

    void
    rebuild()
    {
        list_.clearItems();
        if ( !annotator_ )
            return;
        auto tools = annotator_->listActivePluginTools();
        for ( auto& t : tools )
        {
            auto label = t.icon.empty() ? t.name : ( std::string( t.icon ) + " " + t.name );
            auto btn   = std::make_unique<hui::Button>(
                wm_,
                dr4::Vec2f{ 0.0f, 0.0f },
                dr4::Vec2f{ list_.getSize().x - 12.0f, 28.0f },
                label,
                rowTheme( t.enabled ) );
            btn->setOnClick( [this, idx = t.index]() {
                annotator_->toggleToolEnabled( idx );
                rebuild();
            } );
            list_.addItem( std::move( btn ) );
        }
    }

  private:
    SnapshotAnnotator*        annotator_;
    hui::ScrollableListWidget list_;
    hui::Button               close_;
};

} // namespace view
} // namespace zemax
