// #include "tool_bar.hpp"
// #include "dr4/math/vec2.hpp"
// #include "dr4/window.hpp"
//
// namespace hui {
//
// // === MenuButton ===
// MenuButton::MenuButton( WindowManager*     wm,
//                         const std::string& label,
//                         const dr4::Vec2f&  pos,
//                         const dr4::Vec2f&  size )
//     : Button( wm,
//               pos,
//               size,
//               { 50, 50, 50, 255 }, // default
//               { 70, 70, 70, 255 }, // hover
//               { 30, 30, 30, 255 }, // pressed
//               label,
//               { 220, 220, 220, 255 },
//               12 )
// {
// }
//
// bool
// MenuButton::onMousePress( const Event& event )
// {
//     if ( is_hovered_ && event.info.mouseButton.button == dr4::MouseButtonType::LEFT )
//     {
//         if ( on_click_ )
//             on_click_();
//         return true;
//     }
//     return false;
// }
//
// bool
// MenuButton::onMouseRelease( const Event& event )
// {
//     return false;
// }
//
// // === MenuPopup ===
// MenuPopup::MenuPopup( WindowManager* wm, const dr4::Vec2f& abs_pos, const std::vector<Item>&
// items )
//     : VectorContainerWidget( wm, abs_pos, { 160, static_cast<float>( items.size() * 25 ) } )
// {
//     background_.reset( wm->getWindow()->CreateRectangle() );
//     background_->SetSize( getSize() );
//     background_->SetFillColor( { 40, 40, 40, 255 } );
//     background_->SetBorderColor( { 80, 80, 80, 255 } );
//     background_->SetBorderThickness( -1.0f );
//
//     float y = 0;
//     for ( const auto& item : items )
//     {
//         auto btn = std::make_unique<Button>( wm,
//                                              dr4::Vec2f( 0, y ),
//                                              dr4::Vec2f( 160, 24 ),
//                                              dr4::Color( 50, 50, 50, 255 ),
//                                              dr4::Color( 70, 70, 70, 255 ),
//                                              dr4::Color( 30, 30, 30, 255 ),
//                                              item.label,
//                                              dr4::Color( 220, 220, 220, 255 ),
//                                              12 );
//         btn->setOnClick( [this, cb = item.on_click]() {
//             cb();
//             wm_->popModal();
//         } );
//         addChild( std::move( btn ) );
//         y += 25;
//     }
// }
//
// void
// MenuPopup::RedrawMyTexture() const
// {
//     texture_->Clear( { 0, 0, 0, 0 } );
//     texture_->Draw( *background_ );
//     RedrawChildren();
// }
//
// bool
// MenuPopup::containsPoint( const dr4::Vec2f& pt ) const
// {
//     dr4::Vec2f abs = getAbsPos();
//     dr4::Vec2f sz  = getSize();
//     return ( pt.x >= abs.x && pt.x <= abs.x + sz.x && pt.y >= abs.y && pt.y <= abs.y + sz.y );
// }
//
// bool
// MenuPopup::onMousePress( const Event& event )
// {
//     fprintf( stderr, "debug in %s:%d:%s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__ );
//     dr4::Vec2f mouse = { event.info.mouseButton.pos.x, event.info.mouseButton.pos.y };
//     if ( !containsPoint( mouse ) )
//     {
//         wm_->popModal();
//         return true;
//     }
//     return VectorContainerWidget::onMousePress( event );
// }
//
// void
// ToolBar::addMenu( const std::string& name, std::vector<MenuPopup::Item> items )
// {
// }
//
// void
// ToolBar::RedrawMyTexture() const
// {
//     texture_->Clear( { 0, 0, 0, 0 } );
//     texture_->Draw( *background_ );
//     RedrawChildren();
// }
//
// } // namespace hui

// tool_bar.cpp
#include "tool_bar.hpp"
#include "dr4/math/color.hpp"
#include "dr4/window.hpp"
#include <algorithm>

namespace hui {

// === MenuPopup ===
MenuPopup::MenuPopup( WindowManager* wm, const dr4::Vec2f& pos, const std::vector<MenuItem>& items )
    : Widget( wm,
              pos,
              { 160.0f, 24.0f * static_cast<float>( std::max<size_t>( 1, items.size() ) ) } ),
      items_( items )
{
    background_.reset( wm->getWindow()->CreateRectangle() );
    background_->SetSize( getSize() );
    background_->SetFillColor( { 40, 40, 40, 255 } );
    background_->SetBorderColor( { 80, 80, 80, 255 } );
    background_->SetBorderThickness( -1.0f );

    createTextElements();
}

void
MenuPopup::createTextElements()
{
    const dr4::Font* font = wm_->getWindow()->GetDefaultFont();
    text_elements_.clear();
    text_elements_.reserve( items_.size() );

    for ( size_t i = 0; i < items_.size(); ++i )
    {
        auto* text = wm_->getWindow()->CreateText();
        text->SetFont( font );
        text->SetFontSize( 12 );
        text->SetText( items_[i].label );
        text->SetColor( { 220, 220, 220, 255 } );
        text->SetPos( { 4.0f, static_cast<float>( i ) * ItemHeight + 4.0f } );
        text_elements_.push_back( std::unique_ptr<dr4::Text>( text ) );
    }
}

void
MenuPopup::RedrawMyTexture() const
{
    texture_->Clear( { 0, 0, 0, 0 } );
    texture_->Draw( *background_ );

    // Рисуем все элементы меню
    for ( size_t i = 0; i < text_elements_.size(); ++i )
    {
        texture_->Draw( *text_elements_[i] );
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

    // Если клик вне меню - закрываем его
    if ( !containsPoint( mp ) )
    {
        wm_->popModal();
        return true;
    }

    // Определяем, по какому элементу был клик
    dr4::Vec2f local_pos = mp - getAbsPos();
    int        idx       = static_cast<int>( local_pos.y / ItemHeight );

    if ( idx >= 0 && idx < static_cast<int>( items_.size() ) )
    {
        // Выполняем действие
        if ( items_[idx].on_click )
        {
            items_[idx].on_click();
        }
        // Закрываем меню
        // wm_->popModal();
        return true;
    }

    return false;
}

// === ToolBar ===
ToolBar::ToolBar( WindowManager* wm, float height )
    : Widget( wm, 0.0f, 0.0f, wm->getWindow()->GetSize().x, height ), wm_( wm ), height_( height )
{
    background_.reset( wm->getWindow()->CreateRectangle() );
    background_->SetPos( { 0, 0 } );
    background_->SetSize( { getSize().x, height_ } );
    background_->SetFillColor( { 45, 45, 45, 255 } );

    item_x_ = 10.0f;
}

size_t
ToolBar::addMenu( const std::string& name, std::vector<MenuItem> items )
{
    MenuDef md;
    md.name  = name;
    md.items = std::move( items );
    md.pos   = { item_x_, 0.0f };
    md.size  = { 10.0f + static_cast<float>( name.size() ) * 8.0f, height_ };

    menu_defs_.push_back( std::move( md ) );
    item_x_ += menu_defs_.back().size.x + 10.0f;
    text_elements_.clear();
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
    text_elements_.clear();
}

void
ToolBar::createTextElements()
{
    const dr4::Font* font = wm_->getWindow()->GetDefaultFont();
    text_elements_.clear();
    text_elements_.reserve( menu_defs_.size() );

    for ( size_t i = 0; i < menu_defs_.size(); ++i )
    {
        auto* text = wm_->getWindow()->CreateText();
        text->SetFont( font );
        text->SetFontSize( 12 );
        text->SetText( menu_defs_[i].name );
        text->SetColor( { 220, 220, 220, 255 } );
        text->SetPos( { menu_defs_[i].pos.x + 6.0f, 6.0f } );
        text_elements_.push_back( std::unique_ptr<dr4::Text>( text ) );
    }
}

void
ToolBar::RedrawMyTexture() const
{
    texture_->Clear( { 0, 0, 0, 0 } );
    texture_->Draw( *background_ );

    const_cast<ToolBar*>( this )->createTextElements();

    // Рисуем пункты меню
    for ( size_t i = 0; i < text_elements_.size(); ++i )
    {
        // Подсветка при наведении
        if ( hovered_menu_ == static_cast<int>( i ) )
        {
            auto* highlight = wm_->getWindow()->CreateRectangle();
            highlight->SetPos( menu_defs_[i].pos );
            highlight->SetSize( menu_defs_[i].size );
            highlight->SetFillColor( { 60, 60, 60, 255 } );
            texture_->Draw( *highlight );
        }
        texture_->Draw( *text_elements_[i] );
    }
}

bool
ToolBar::onMouseMove( const Event& event )
{
    dr4::Vec2f mp( event.info.mouseMove.pos.x, event.info.mouseMove.pos.y );
    dr4::Vec2f local_pos = mp - getAbsPos();

    // Проверяем наведение только на топбар (не на всё окно)
    if ( local_pos.y < 0 || local_pos.y > height_ )
    {
        if ( hovered_menu_ != -1 )
        {
            hovered_menu_ = -1;
            return true;
        }
        return false;
    }

    // Находим, над каким пунктом меню находится курсор
    int new_hover = -1;
    for ( size_t i = 0; i < menu_defs_.size(); ++i )
    {
        if ( local_pos.x >= menu_defs_[i].pos.x &&
             local_pos.x <= menu_defs_[i].pos.x + menu_defs_[i].size.x )
        {
            new_hover = static_cast<int>( i );
            break;
        }
    }

    if ( new_hover != hovered_menu_ )
    {
        hovered_menu_ = new_hover;
        return true;
    }

    return false;
}

bool
ToolBar::onMousePress( const Event& event )
{
    if ( event.info.mouseButton.button != dr4::MouseButtonType::LEFT )
    {
        return false;
    }

    dr4::Vec2f mp( event.info.mouseButton.pos.x, event.info.mouseButton.pos.y );
    dr4::Vec2f local_pos = mp - getAbsPos();

    // Если клик вне топбара - не обрабатываем
    if ( local_pos.y < 0 || local_pos.y > height_ )
    {
        return false;
    }

    // Проверяем, по какому пункту меню был клик
    for ( size_t i = 0; i < menu_defs_.size(); ++i )
    {
        if ( local_pos.x >= menu_defs_[i].pos.x &&
             local_pos.x <= menu_defs_[i].pos.x + menu_defs_[i].size.x )
        {
            pressed_index_ = static_cast<int>( i );
            return true;
        }
    }

    return false;
}

bool
ToolBar::onMouseRelease( const Event& event )
{
    if ( event.info.mouseButton.button != dr4::MouseButtonType::LEFT )
    {
        return false;
    }

    if ( pressed_index_ >= 0 )
    {
        int idx        = pressed_index_;
        pressed_index_ = -1;

        if ( idx >= 0 && idx < static_cast<int>( menu_defs_.size() ) )
        {
            // Создаем и показываем popup меню
            dr4::Vec2f popup_pos = { getAbsPos().x + menu_defs_[idx].pos.x,
                                     getAbsPos().y + height_ };

            std::cerr << "popup_pos = " << popup_pos.x << " " << popup_pos.y << std::endl;

            auto popup = std::make_unique<MenuPopup>( wm_, popup_pos, menu_defs_[idx].items );
            wm_->pushModal( std::move( popup ) );
            return true;
        }
    }

    return false;
}

} // namespace hui
