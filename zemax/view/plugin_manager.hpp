#pragma once

#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/scrollable_list_widget.hpp"
#include "custom-hui-impl/file_dialog_box.hpp"
#include "custom-hui-impl/dialog_box.hpp"
#include "zemax/config.hpp"
#include "zemax/view/snapshot_annotator.hpp"
#include <functional>
#include <memory>
#include <string>

namespace zemax {
namespace view {

class PluginItemRow : public hui::ContainerWidget {
  public:
    PluginItemRow( hui::WindowManager*   wm,
                   const dr4::Vec2f&     size,
                   const std::string&    label,
                   bool                  active,
                   std::function<void()> on_select,
                   std::function<void()> on_delete )
        : ContainerWidget( wm, { 0, 0 }, size ),
          select_btn_( wm,
                       dr4::Vec2f{ 0.0f, 0.0f },
                       dr4::Vec2f{ size.x - btn_w_ - gap_, size.y },
                       label,
                       hui::Button::Theme{ active ? Config::ControlPanel::Button::PressedColor
                                                  : Config::ControlPanel::Button::DefaultColor,
                                           Config::ControlPanel::Button::HoveredColor,
                                           Config::ControlPanel::Button::PressedColor,
                                           Config::ControlPanel::Button::FontColor,
                                           Config::ControlPanel::Button::FontSize } ),
          del_btn_( wm,
                    dr4::Vec2f{ size.x - btn_w_, 0.0f },
                    dr4::Vec2f{ btn_w_ - gap_, size.y },
                    "X",
                    hui::Button::Theme{ Config::ControlPanel::Button::DefaultColor,
                                        Config::ControlPanel::Button::HoveredColor,
                                        Config::ControlPanel::Button::PressedColor,
                                        Config::ControlPanel::Button::FontColor,
                                        Config::ControlPanel::Button::FontSize } )
    {
        select_btn_.setParent( this );
        del_btn_.setParent( this );

        select_btn_.setOnClick( std::move( on_select ) );
        del_btn_.setOnClick( std::move( on_delete ) );
    }

    void
    setActive( bool active )
    {
        select_btn_.setBackgroundColor( active ? Config::ControlPanel::Button::PressedColor
                                               : Config::ControlPanel::Button::DefaultColor );
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        bool handled = false;
        handled |= event.apply( &select_btn_ );
        handled |= event.apply( &del_btn_ );
        return handled;
    }

    void
    RedrawMyTexture() const override
    {
        select_btn_.Redraw();
        del_btn_.Redraw();
    }

  private:
    static constexpr float btn_w_ = 50.0f;
    static constexpr float gap_   = 4.0f;
    hui::Button            select_btn_;
    hui::Button            del_btn_;
};

class PluginListModal : public hui::DialogBox {
  public:
    PluginListModal( hui::WindowManager*   wm,
                     float                 x,
                     float                 y,
                     float                 w,
                     float                 h,
                     SnapshotAnnotator*    annotator,
                     std::function<void()> on_close )
        : DialogBox(
              wm,
              x,
              y,
              w,
              h,
              [on_close]() {
                  if ( on_close )
                      on_close();
              },
              "Plugins" ),
          annotator_( annotator ),
          list_( wm, { 8.0f, TopBarHeight + 8.0f }, { w - 16.0f, h - TopBarHeight - 16.0f }, 12.0f )
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
            auto  row    = std::make_unique<PluginItemRow>(
                wm_,
                dr4::Vec2f{ list_.getSize().x - 12.0f, 32.0f },
                entry.first,
                plugin == active,
                [this, plugin]() {
                    annotator_->setActivePlugin( plugin );
                    rebuild();
                },
                [this, plugin]() {
                    annotator_->removePlugin( plugin );
                    rebuild();
                } );
            list_.addItem( std::move( row ) );
        }
    }

  private:
    SnapshotAnnotator*        annotator_;
    hui::ScrollableListWidget list_;
};

class PluginPopup : public hui::DialogBox {
  public:
    PluginPopup( hui::WindowManager* wm, float x, float y, SnapshotAnnotator* annotator )
        : DialogBox(
              wm,
              x,
              y,
              240.0f,
              120.0f,
              [wm]() { wm->popModal(); },
              "Plugins" ),
          annotator_( annotator ),
          new_btn_( wm,
                    dr4::Vec2f( 10.0f, TopBarHeight + 12.0f ),
                    dr4::Vec2f( 100.0f, 32.0f ),
                    "New",
                    hui::Button::Theme{ Config::ControlPanel::Button::DefaultColor,
                                        Config::ControlPanel::Button::HoveredColor,
                                        Config::ControlPanel::Button::PressedColor,
                                        Config::ControlPanel::Button::FontColor,
                                        Config::ControlPanel::Button::FontSize } ),
          show_btn_( wm,
                     dr4::Vec2f( 130.0f, TopBarHeight + 12.0f ),
                     dr4::Vec2f( 100.0f, 32.0f ),
                     "Show",
                     hui::Button::Theme{ Config::ControlPanel::Button::DefaultColor,
                                         Config::ControlPanel::Button::HoveredColor,
                                         Config::ControlPanel::Button::PressedColor,
                                         Config::ControlPanel::Button::FontColor,
                                         Config::ControlPanel::Button::FontSize } )
    {
        new_btn_.setParent( this );
        show_btn_.setParent( this );

        new_btn_.setOnClick( [this, wm]() {
            wm->pushModal( std::make_unique<hui::FileDialogBox>(
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
            wm->pushModal( std::make_unique<PluginListModal>( wm,
                                                              getRelPos().x + getSize().x + 10.0f,
                                                              getRelPos().y,
                                                              320.0f,
                                                              280.0f,
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
        return hui::DialogBox::propagateEventToChildren( event );
    }

    void
    RedrawMyTexture() const override
    {
        hui::DialogBox::RedrawMyTexture();
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
