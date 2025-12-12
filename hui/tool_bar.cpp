#include "tool_bar.hpp"
#include "dr4/math/color.hpp"
#include "dr4/window.hpp"
#include <algorithm>

namespace hui {

MenuPopup::MenuPopup( WindowManager*               wm,
                      const dr4::Vec2f&            pos,
                      const std::string&           menu_name,
                      const std::vector<MenuItem>& items,
                      const Theme&                 theme )
    : Widget( wm, pos, { 1.0f, 1.0f } ), items_( items ), menu_name_( menu_name ), theme_( theme )
{
    background_.reset( wm->getWindow()->CreateRectangle() );
    background_->SetFillColor( theme_.background );
    background_->SetBorderColor( theme_.border );
    background_->SetBorderThickness( theme_.border_thickness );

    createButtons();
}

void
MenuPopup::updateSize()
{
    float max_w = 0.0f;
    float max_h = 0.0f;
    for ( const auto& btn : buttons_->getButtons() )
    {
        max_w = std::max( max_w, btn->getSize().x );
        max_h = std::max( max_h, btn->getSize().y );
    }

    for ( auto& btn : buttons_->getButtons() )
    {
        btn->setSize( { max_w, max_h } );
        btn->setLabelAlignment( Button::LabelAlign::Left, theme_.padding_x );
    }

    buttons_->rebuildLayout();

    dr4::Vec2f list_size = buttons_->getSize();
    setSize( list_size );
    background_->SetSize( list_size );
}

void
MenuPopup::createButtons()
{
    buttons_ = std::make_unique<VerticalButtonsList>( wm_, theme_.padding_y );
    buttons_->setParent( this );

    auto btn_theme = Button::Theme{ theme_.background,
                                    theme_.hover_color,
                                    theme_.hover_color,
                                    theme_.text_color,
                                    theme_.font_size };

    for ( const auto& item : items_ )
    {
        auto btn = std::make_unique<Button>( wm_, item.label, btn_theme );
        btn->fitToLabel( theme_.padding_x * 2.0f, theme_.padding_y * 2.0f );
        btn->setOnClick( item.on_click );
        buttons_->addButton( std::move( btn ) );
    }

    updateSize();
}

void
MenuPopup::RedrawMyTexture() const
{
    texture_->Clear( { 0, 0, 0, 0 } );
    texture_->Draw( *background_ );

    if ( buttons_ )
    {
        buttons_->Redraw();
    }
}

bool
MenuPopup::containsPoint( const dr4::Vec2f& pt ) const
{
    dr4::Vec2f abs = getAbsPos();
    dr4::Vec2f sz  = getSize();
    return ( pt.x >= abs.x && pt.x <= abs.x + sz.x && pt.y >= abs.y && pt.y <= abs.y + sz.y );
}

bool
MenuPopup::onMousePress( const Event& event )
{
    if ( event.info.mouseButton.button != dr4::MouseButtonType::LEFT )
    {
        return false;
    }

    dr4::Vec2f mp( event.info.mouseButton.pos.x, event.info.mouseButton.pos.y );

    if ( !containsPoint( mp ) )
    {
        wm_->popModal();
        return true;
    }

    if ( buttons_ && event.apply( buttons_.get() ) )
    {
        return true;
    }

    return false;
}

bool
MenuPopup::onMouseMove( const Event& event )
{
    return buttons_ && event.apply( buttons_.get() );
}

bool
MenuPopup::onMouseRelease( const Event& event )
{
    return buttons_ && event.apply( buttons_.get() );
}

void
MenuPopup::setItemLabel( size_t idx, const std::string& label )
{
    if ( idx >= items_.size() || !buttons_ )
    {
        return;
    }

    items_[idx].label = label;
    auto& btns        = buttons_->getButtons();
    if ( idx < btns.size() )
    {
        btns[idx]->setLabelText( label );
        btns[idx]->fitToLabel( theme_.padding_x * 2.0f, theme_.padding_y * 2.0f );
        btns[idx]->setLabelAlignment( Button::LabelAlign::Left, theme_.padding_x );
    }

    updateSize();
}

const std::string&
MenuPopup::menuName() const
{
    return menu_name_;
}

ToolBar::ToolBar( WindowManager* wm, float height )
    : hui::ToolBar( wm, Theme::Default().changedHeight( height ) )
{
}

ToolBar::ToolBar( WindowManager* wm, const Theme& theme )
    : Widget( wm, 0.0f, 0.0f, wm->getWindow()->GetSize().x, theme.height ),
      wm_( wm ),
      theme_( theme )
{
    background_.reset( wm->getWindow()->CreateRectangle() );
    background_->SetPos( { 0, 0 } );
    background_->SetSize( { getSize().x, theme_.height } );
    background_->SetFillColor( theme_.background_color );

    buttons_ =
        std::make_unique<HorizontalButtonsList>( wm_,
                                                 dr4::Vec2f{ theme_.padding_x, theme_.padding_y },
                                                 dr4::Vec2f{ 0.0f, theme_.height },
                                                 theme_.padding_x );
    buttons_->setParent( this );
    buttons_->setLabelPadding( theme_.label_padding_x );
}

size_t
ToolBar::addMenu( const std::string& name, std::vector<MenuItem> items )
{
    MenuDef md;
    md.name  = name;
    md.items = std::move( items );

    auto btn_theme = Button::Theme{ theme_.background_color,
                                    theme_.hover_color,
                                    theme_.hover_color,
                                    theme_.font_color,
                                    theme_.font_size };

    auto btn = std::make_unique<Button>(
        wm_,
        dr4::Vec2f{ 0.0f, 0.0f },
        dr4::Vec2f{ 0.0f, std::max( 1.0f, theme_.height - 2.0f * theme_.padding_y ) },
        name,
        btn_theme );
    btn->setSize( { btn->getSize().x, std::max( 1.0f, theme_.height - 2.0f * theme_.padding_y ) } );

    size_t idx_capture = menu_defs_.size();
    btn->setOnClick( [this, idx_capture]() {
        if ( idx_capture >= menu_defs_.size() )
        {
            return;
        }
        const auto& md        = menu_defs_[idx_capture];
        dr4::Vec2f  popup_pos = getAbsPos() + buttons_->getRelPos() + md.button->getRelPos() +
                               dr4::Vec2f{ 0.0f, theme_.height };
        auto popup = std::make_unique<MenuPopup>( wm_, popup_pos, md.name, md.items );
        wm_->pushModal( std::move( popup ) );
    } );

    md.button = btn.get();

    buttons_->addButton( std::move( btn ) );
    menu_defs_.push_back( std::move( md ) );

    rebuildLayout();

    return menu_defs_.size() - 1;
}

void
ToolBar::setMenuItemLabel( const std::string& menu_name, size_t item_idx, const std::string& label )
{
    if ( menu_defs_.empty() )
    {
        return;
    }

    auto it = std::find_if( menu_defs_.begin(), menu_defs_.end(), [&]( const MenuDef& md ) {
        return md.name == menu_name;
    } );

    if ( it == menu_defs_.end() )
    {
        return;
    }

    if ( item_idx >= it->items.size() )
    {
        return;
    }

    it->items[item_idx].label = label;
    rebuildLayout();

    if ( auto* popup = dynamic_cast<MenuPopup*>( wm_->getTopModal() ) )
    {
        if ( popup->menuName() == menu_name )
        {
            popup->setItemLabel( item_idx, label );
        }
    }
}

void
ToolBar::rebuildLayout()
{
    if ( buttons_ )
    {
        buttons_->rebuildLayout();
    }
}

void
ToolBar::RedrawMyTexture() const
{
    texture_->Clear( { 0, 0, 0, 0 } );
    texture_->Draw( *background_ );

    if ( buttons_ )
    {
        buttons_->Redraw();
    }
}

bool
ToolBar::onMouseMove( const Event& event )
{
    if ( !buttons_ )
    {
        return false;
    }

    dr4::Vec2f mp( event.info.mouseMove.pos.x, event.info.mouseMove.pos.y );
    dr4::Vec2f local_pos = mp - getAbsPos();

    if ( local_pos.y < 0 || local_pos.y > theme_.height )
    {
        return false;
    }

    return event.apply( buttons_.get() );
}

bool
ToolBar::onMousePress( const Event& event )
{
    if ( event.info.mouseButton.button != dr4::MouseButtonType::LEFT )
    {
        return false;
    }

    if ( !buttons_ )
    {
        return false;
    }

    dr4::Vec2f mp( event.info.mouseButton.pos.x, event.info.mouseButton.pos.y );
    dr4::Vec2f local_pos = mp - getAbsPos();

    if ( local_pos.y < 0 || local_pos.y > theme_.height )
    {
        return false;
    }

    return event.apply( buttons_.get() );
}

bool
ToolBar::onMouseRelease( const Event& event )
{
    if ( event.info.mouseButton.button != dr4::MouseButtonType::LEFT )
    {
        return false;
    }

    if ( !buttons_ )
    {
        return false;
    }

    return event.apply( buttons_.get() );
}

} // namespace hui
