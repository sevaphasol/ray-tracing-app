#include "rta/view/utils/plugin_manager.hpp"
#include "hui/button.hpp"
#include "hui/dialog_box.hpp"
#include "hui/file_dialog_box.hpp"
#include "hui/scrollable_buttons_list_widget.hpp"
#include "rta/view/annotator/snapshot_annotator.hpp"
#include <algorithm>
#include <memory>
#include <string>

rta::view::PluginManagerDialog::PluginManagerDialog( hui::WindowManager* wm,
                                                     SnapshotAnnotator*  annotator,
                                                     const Theme&        theme )
    : PluginManagerDialog( wm,
                           theme.pos.x,
                           theme.pos.y,
                           theme.size.x,
                           theme.size.y,
                           annotator,
                           theme )
{
}

rta::view::PluginManagerDialog::PluginManagerDialog( hui::WindowManager* wm,
                                                     float               x,
                                                     float               y,
                                                     float               w,
                                                     float               h,
                                                     SnapshotAnnotator*  annotator,
                                                     const Theme&        theme )
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
rta::view::PluginManagerDialog::propagateEventToChildren( const hui::Event& event )
{
    if ( list_ && event.apply( list_.get() ) )
    {
        return true;
    }
    if ( event.apply( &choose_btn_ ) )
    {
        return true;
    }
    if ( event.apply( &delete_btn_ ) )
    {
        return true;
    }

    return hui::DialogBox::propagateEventToChildren( event );
}

void
rta::view::PluginManagerDialog::RedrawMyTexture() const
{
    hui::DialogBox::RedrawMyTexture();
    if ( list_ )
    {
        list_->Redraw();
    }
    choose_btn_.Redraw();
    delete_btn_.Redraw();
}

void
rta::view::PluginManagerDialog::buildList()
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
rta::view::PluginManagerDialog::layoutActionButtons()
{
    const float available_w = theme_.size.x - 2.0f * theme_.action_buttons_gap;
    const float btn_w       = available_w * 0.5f - 0.5f * theme_.action_buttons_gap;
    const float y           = theme_.size.y - theme_.actions_row_height +
                    0.5f * ( theme_.actions_row_height - theme_.action_button_height );

    choose_btn_.setSize( { btn_w, theme_.action_button_height } );
    delete_btn_.setSize( { btn_w, theme_.action_button_height } );

    choose_btn_.setRelPos( { theme_.action_buttons_gap, y } );
    delete_btn_.setRelPos( { theme_.action_buttons_gap + btn_w + theme_.action_buttons_gap, y } );
}

hui::Button::Theme
rta::view::PluginManagerDialog::rowTheme( bool active, bool selected ) const
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

    return hui::Button::Theme{ bg,
                               colors.hover_bg,
                               colors.press_bg,
                               colors.font_color,
                               colors.font_size };
}

float
rta::view::PluginManagerDialog::itemWidth() const
{
    return std::max( 1.0f,
                     theme_.size.x - 2.0f * theme_.margin - theme_.scrollbar_width -
                         2.0f * theme_.list_item_padding );
}

void
rta::view::PluginManagerDialog::openFileDialog()
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
rta::view::PluginManagerDialog::rebuild()
{
    if ( !list_ )
    {
        return;
    }

    annotator_->refreshPlugins();
    auto  plugins = annotator_->listPlugins();
    auto* active  = annotator_->getActivePlugin();

    if ( selected_ == nullptr )
    {
        selected_ = active;
    }

    if ( ( selected_ != nullptr ) &&
         std::none_of( plugins.begin(), plugins.end(), [this]( auto& p ) {
             return p.second == selected_;
         } ) )
    {
        selected_ = active;
    }

    list_->clearButtons();

    const dr4::Vec2f btn_size{ itemWidth(), theme_.list_item_height };

    auto add_btn = std::make_unique<hui::Button>( wm_,
                                                  dr4::Vec2f{ 0.0f, 0.0f },
                                                  btn_size,
                                                  "+",
                                                  theme_.add_button_theme );
    add_btn->setOnClick( [this]() { openFileDialog(); } );
    list_->addButton( std::move( add_btn ) );

    for ( auto& entry : plugins )
    {
        auto* plugin = entry.second;
        auto  btn =
            std::make_unique<hui::Button>( wm_,
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
rta::view::PluginManagerDialog::chooseSelected()
{
    if ( selected_ != nullptr )
    {
        annotator_->setActivePlugin( selected_ );
        rebuild();
    }
}

void
rta::view::PluginManagerDialog::deleteSelected()
{
    if ( selected_ == nullptr )
    {
        return;
    }
    annotator_->removePlugin( selected_ );
    annotator_->refreshPlugins();
    auto plugins = annotator_->listPlugins();
    selected_    = plugins.empty() ? nullptr : plugins.front().second;
    rebuild();
}

rta::view::ToolSelectorDialog::ToolSelectorDialog( hui::WindowManager*           wm,
                                                   rta::view::SnapshotAnnotator* annotator,
                                                   const Theme&                  theme )
    : ToolSelectorDialog( wm,
                          theme.pos.x,
                          theme.pos.y,
                          theme.size.x,
                          theme.size.y,
                          annotator,
                          theme )
{
}

rta::view::ToolSelectorDialog::ToolSelectorDialog( hui::WindowManager*           wm,
                                                   float                         x,
                                                   float                         y,
                                                   float                         w,
                                                   float                         h,
                                                   rta::view::SnapshotAnnotator* annotator,
                                                   const Theme&                  theme )
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
      close_( wm,
              dr4::Vec2f{ 0.0f, 0.0f },
              theme.close_button_size,
              "Close",
              theme.close_button_theme )
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
rta::view::ToolSelectorDialog::propagateEventToChildren( const hui::Event& event )
{
    if ( list_ && event.apply( list_.get() ) )
    {
        return true;
    }
    if ( event.apply( &close_ ) )
    {
        return true;
    }

    return DialogBox::propagateEventToChildren( event );
}

void
rta::view::ToolSelectorDialog::RedrawMyTexture() const
{
    DialogBox::RedrawMyTexture();
    if ( list_ )
    {
        list_->Redraw();
    }
    close_.Redraw();
}

void
rta::view::ToolSelectorDialog::buildList()
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
rta::view::ToolSelectorDialog::layoutCloseButton()
{
    const float y = theme_.size.y - theme_.actions_row_height +
                    0.5f * ( theme_.actions_row_height - theme_.close_button_size.y );
    const float x = theme_.size.x - theme_.close_button_padding - theme_.close_button_size.x;

    close_.setRelPos( { x, y } );
}

hui::Button::Theme
rta::view::ToolSelectorDialog::rowTheme( bool enabled ) const
{
    return enabled ? theme_.row_theme.enabled : theme_.row_theme.disabled;
}

float
rta::view::ToolSelectorDialog::itemWidth() const
{
    return std::max( 1.0f,
                     theme_.size.x - 2.0f * theme_.margin - theme_.scrollbar_width -
                         2.0f * theme_.list_item_padding );
}

void
rta::view::ToolSelectorDialog::rebuild()
{
    if ( !list_ || ( annotator_ == nullptr ) )
    {
        return;
    }

    list_->clearButtons();

    auto             tools = annotator_->listActivePluginTools();
    const dr4::Vec2f btn_size{ itemWidth(), theme_.list_item_height };

    for ( auto& t : tools )
    {
        auto label = t.icon.empty() ? t.name : ( std::string( t.icon ) + " " + t.name );
        auto btn   = std::make_unique<hui::Button>( wm_,
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
