#pragma once

#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/scrollable_list_widget.hpp"
#include "zemax/view/file_dialog.hpp"
#include "zemax/view/obj_info_box.hpp"
#include "zemax/view/snapshot_annotator.hpp"
#include "zemax/config.hpp"
#include <functional>

namespace zemax {
namespace view {

class PluginListModal : public ObjInfoBox {
  public:
    PluginListModal( hui::WindowManager* wm,
                     float               x,
                     float               y,
                     float               w,
                     float               h,
                     SnapshotAnnotator*  annotator,
                     std::function<void()> on_close )
        : ObjInfoBox( wm, x, y, w, h, [on_close]() { if ( on_close ) on_close(); }, "Plugins" ),
          annotator_( annotator ),
          list_( wm,
                 { 8.0f, TopBarHeight + 8.0f },
                 { w - 16.0f, h - TopBarHeight - 16.0f },
                 12.0f )
    {
        list_.setParent( this );
        rebuild();
    }

    void
    rebuild()
    {
        list_.clearItems();
        auto plugins = annotator_->listPlugins();
        auto active  = annotator_->getActivePlugin();

        for ( auto& entry : plugins )
        {
            auto* plugin = entry.second;
            auto  color_default = ( plugin == active )
                                      ? Config::ControlPanel::Button::PressedColor
                                      : Config::ControlPanel::Button::DefaultColor;
            auto color_hover    = ( plugin == active )
                                      ? Config::ControlPanel::Button::PressedColor
                                      : Config::ControlPanel::Button::HoveredColor;
            auto color_pressed  = Config::ControlPanel::Button::PressedColor;

            auto btn = std::make_unique<hui::Button>( wm_,
                                                      dr4::Vec2f( 4.0f, 0.0f ),
                                                      dr4::Vec2f( list_.getSize().x - 8.0f,
                                                                  32.0f ),
                                                      color_default,
                                                      color_hover,
                                                      color_pressed,
                                                      entry.first,
                                                      Config::ControlPanel::Button::FontColor,
                                                      Config::ControlPanel::Button::FontSize );
            btn->setOnClick( [this, plugin]() {
                annotator_->setActivePlugin( plugin );
                rebuild();
            } );
            list_.addItem( std::move( btn ) );
        }
    }

  private:
    SnapshotAnnotator*      annotator_;
    hui::ScrollableListWidget list_;
};

class PluginPopup : public ObjInfoBox {
  public:
    PluginPopup( hui::WindowManager* wm,
                 float               x,
                 float               y,
                 SnapshotAnnotator*  annotator )
        : ObjInfoBox( wm,
                      x,
                      y,
                      220.0f,
                      100.0f,
                      [wm]() { wm->popModal(); },
                      "Plugins" ),
          annotator_( annotator ),
          new_btn_( wm,
                    dr4::Vec2f( 10.0f, TopBarHeight + 10.0f ),
                    dr4::Vec2f( 90.0f, 32.0f ),
                    Config::ControlPanel::Button::DefaultColor,
                    Config::ControlPanel::Button::HoveredColor,
                    Config::ControlPanel::Button::PressedColor,
                    "New",
                    Config::ControlPanel::Button::FontColor,
                    Config::ControlPanel::Button::FontSize ),
          show_btn_( wm,
                     dr4::Vec2f( 120.0f, TopBarHeight + 10.0f ),
                     dr4::Vec2f( 90.0f, 32.0f ),
                     Config::ControlPanel::Button::DefaultColor,
                     Config::ControlPanel::Button::HoveredColor,
                     Config::ControlPanel::Button::PressedColor,
                     "Show",
                     Config::ControlPanel::Button::FontColor,
                     Config::ControlPanel::Button::FontSize )
    {
        new_btn_.setParent( this );
        show_btn_.setParent( this );

        new_btn_.setOnClick( [this, wm]() {
            wm->pushModal( std::make_unique<view::FileDialog>(
                wm,
                getRelPos().x + getSize().x + 10.0f,
                getRelPos().y,
                420.0f,
                170.0f,
                "Load Plugin",
                "plugins/libplugin_pp.so",
                [this, wm]( const std::string& path ) {
                    wm->getPluginManager()->LoadFromFile( path );
                    annotator_->refreshPlugins();
                    auto plugins = annotator_->listPlugins();
                    if ( !plugins.empty() )
                    {
                        annotator_->setActivePlugin( plugins.back().second );
                    }
                    wm->popModal();
                },
                [wm]() { wm->popModal(); } ) );
        } );

        show_btn_.setOnClick( [this, wm]() {
            annotator_->refreshPlugins();
            wm->pushModal( std::make_unique<PluginListModal>(
                wm,
                getRelPos().x + getSize().x + 10.0f,
                getRelPos().y,
                260.0f,
                260.0f,
                annotator_,
                [wm]() { wm->popModal(); } ) );
        } );
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        if ( event.apply( &new_btn_ ) )
            return true;
        if ( event.apply( &show_btn_ ) )
            return true;
        return ObjInfoBox::propagateEventToChildren( event );
    }

    void
    RedrawMyTexture() const override
    {
        ObjInfoBox::RedrawMyTexture();
        new_btn_.Redraw();
        show_btn_.Redraw();
    }

  private:
    SnapshotAnnotator* annotator_;
    hui::Button        new_btn_;
    hui::Button        show_btn_;
};

} // namespace view
} // namespace zemax
