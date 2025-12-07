// // // #pragma once
// // //
// // // #include "button.hpp"
// // // #include "container_widget.hpp"
// // // #include "dr4/math/vec2.hpp"
// // // #include "widget.hpp"
// // // #include "window_manager.hpp"
// // // #include <functional>
// // // #include <memory>
// // // #include <string>
// // // #include <vector>
// // //
// // // namespace hui {
// // //
// // // class MenuPopup;
// // //
// // // class MenuButton : public Button {
// // //   public:
// // //     using ClickCallback = std::function<void()>;
// // //
// // //     MenuButton( WindowManager*     wm,
// // //                 const std::string& label,
// // //                 const dr4::Vec2f&  pos,
// // //                 const dr4::Vec2f&  size );
// // //
// // //     void
// // //     setOnClick( ClickCallback cb )
// // //     {
// // //         on_click_ = std::move( cb );
// // //     }
// // //
// // //     bool
// // //     onMousePress( const Event& event ) override;
// // //     bool
// // //     onMouseRelease( const Event& event ) override;
// // //
// // //   private:
// // //     ClickCallback on_click_;
// // // };
// // //
// // // class MenuPopup : public VectorContainerWidget {
// // //   public:
// // //     struct Item
// // //     {
// // //         std::string           label;
// // //         std::function<void()> on_click;
// // //     };
// // //
// // //     MenuPopup( WindowManager* wm, const dr4::Vec2f& abs_pos, const std::vector<Item>& items
// );
// // //
// // //     void
// // //     RedrawMyTexture() const override;
// // //
// // //     bool
// // //     onMousePress( const Event& event ) override;
// // //     bool
// // //     containsPoint( const dr4::Vec2f& pt ) const;
// // //
// // //   private:
// // //     std::unique_ptr<dr4::Rectangle> background_;
// // // };
// // //
// // // class ToolBar : public VectorContainerWidget {
// // //   public:
// // //     ToolBar( WindowManager* wm, dr4::Vec2f pos, dr4::Vec2f size )
// // //         : hui::VectorContainerWidget( wm, pos, size ),
// // //           background_( wm->getWindow()->CreateRectangle() )
// // //     {
// // //         background_->SetPos( { 0, 0 } );
// // //         background_->SetSize( getSize() );
// // //         background_->SetFillColor( { 32, 32, 32 } );
// // //     }
// // //
// // //     void
// // //     addMenu( const std::string& name, std::vector<MenuPopup::Item> items )
// // //     {
// // //         float width    = 10.0f + name.size() * 8.0f;
// // //         auto  menu_btn = std::make_unique<MenuButton>( wm_,
// // //                                                       name,
// // //                                                       dr4::Vec2f{ item_x_, 0 },
// // //                                                       dr4::Vec2f{ width, item_height_ } );
// // //
// // //         menu_btn->setOnClick( [this, items = std::move( items ), btn_ptr =
// menu_btn.get()]() {
// // //             dr4::Vec2f abs_pos = btn_ptr->getAbsPos();
// // //             auto       popup =
// // //                 std::make_unique<MenuPopup>( wm_,
// // //                                              dr4::Vec2f{ abs_pos.x, abs_pos.y +
// item_height_
// // },
// // //                                              items );
// // //             wm_->pushModal( std::move( popup ) );
// // //         } );
// // //
// // //         item_x_ += width + item_spacing_;
// // //         addChild( std::move( menu_btn ) );
// // //     }
// // //
// // //     void
// // //     RedrawMyTexture() const override;
// // //
// // //   private:
// // //     std::unique_ptr<dr4::Rectangle> background_;
// // // };
// // //
// // // } // namespace hui
// //
// // #pragma once
// //
// // // tool_bar_new.hpp + tool_bar_new.cpp
// // // Complete standalone implementation of a ToolBar widget that does NOT depend
// // // on the previous toolbar code. It uses the project's Widget/WindowManager
// // // primitives (assumes they exist as in your repo) and implements full-window
// // // texture for the toolbar so popups aren't clipped. Popups support nested
// // // submenus opened on hover (OnMouseMove) and click activation via callbacks.
// //
// // #include "dr4/math/color.hpp"
// // #include "dr4/math/vec2.hpp"
// // #include "dr4/texture.hpp"
// // #include "dr4/window.hpp"
// // #include "widget.hpp"
// // #include "window_manager.hpp"
// // #include <algorithm>
// // #include <chrono>
// // #include <functional>
// // #include <memory>
// // #include <optional>
// // #include <string>
// // #include <vector>
// //
// // namespace hui {
// //
// // // ---------------------------------------------------------------------------
// // // Menu data structures (public)
// // // ---------------------------------------------------------------------------
// //
// // struct MenuItem
// // {
// //     std::string           label;
// //     std::function<void()> on_click;
// //     std::vector<MenuItem> children; // nested submenu
// // };
// //
// // // ---------------------------------------------------------------------------
// // // MenuPopup - draws a rectangular popup with items and supports nested
// // // submenu openings on hover. Designed to be attached as a child of the
// // // ToolbarFullWindow. Popup draws into toolbar's full-window texture so it
// // // never gets clipped.
// // // ---------------------------------------------------------------------------
// //
// // class MenuPopup : public Widget {
// //   public:
// //     MenuPopup( WindowManager* wm, const dr4::Vec2f& pos, const std::vector<MenuItem>& items )
// //         : Widget( wm,
// //                   pos,
// //                   { 160.0f, 24.0f * static_cast<float>( std::max<size_t>( 1, items.size() ) )
// }
// //                   ),
// //           items_( items )
// //     {
// //         background_.reset( wm->getWindow()->CreateRectangle() );
// //         background_->SetSize( getSize() );
// //         background_->SetFillColor( { 40, 40, 40, 255 } );
// //         background_->SetBorderColor( { 80, 80, 80, 255 } );
// //         background_->SetBorderThickness( -1.0f );
// //
// //         // Prepare text objects lazily in RedrawMyTexture to avoid font issues at construction
// //     }
// //
// //     void
// //     RedrawMyTexture() const override
// //     {
// //         // std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' << __func__ <<
// //         std::endl;
// //
// //         texture_->Clear( { 0, 0, 0, 0 } );
// //         texture_->Draw( *background_ );
// //
// //         // std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' << __func__ <<
// //         std::endl;
// //
// //         const dr4::Font* font = wm_->getWindow()->GetDefaultFont();
// //         float            y    = 0.0f;
// //
// //         // std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' << __func__ <<
// //         std::endl;
// //
// //         for ( size_t i = 0; i < items_.size(); ++i )
// //         {
// //             // std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' << __func__ <<
// //             // std::endl;
// //
// //             if ( hover_index_.has_value() && hover_index_.value() == i )
// //             {
// //                 // std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' << __func__
// //                 // << std::endl;
// //
// //                 auto* r = wm_->getWindow()->CreateRectangle();
// //                 r->SetPos( { getRelPos().x, getRelPos().y + y } );
// //                 r->SetSize( { getSize().x, ItemHeight } );
// //                 r->SetFillColor( { 60, 60, 60, 255 } );
// //
// //                 // std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' << __func__
// //                 // << std::endl;
// //
// //                 texture_->Draw( *r );
// //             }
// //
// //             // std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' << __func__ <<
// //             // std::endl;
// //
// //             auto* t = wm_->getWindow()->CreateText();
// //             t->SetFont( font );
// //             t->SetFontSize( 12 );
// //             // std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' << __func__ <<
// //             // std::endl;
// //             t->SetText( items_[i].label );
// //             // std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' << __func__ <<
// //             // std::endl;
// //             t->SetColor( { 220, 220, 220, 255 } );
// //             t->SetPos( { 4.0f, static_cast<float>( i ) * ItemHeight + 4.0f } );
// //
// //             // std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' << __func__ <<
// //             // std::endl;
// //
// //             texture_->Draw( *t );
// //         }
// //
// //         // std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' << __func__ <<
// //         std::endl;
// //
// //         RedrawChildren();
// //     }
// //
// //     bool
// //     onMouseMove( const Event& evt ) override
// //     {
// //         dr4::Vec2f mp( evt.info.mouseMove.pos.x, evt.info.mouseMove.pos.y );
// //         float      local_y = mp.y - getAbsPos().y;
// //         int        idx     = static_cast<int>( local_y / ItemHeight );
// //         if ( idx < 0 || idx >= static_cast<int>( items_.size() ) )
// //         {
// //             hover_index_ = std::nullopt;
// //             return false;
// //         }
// //
// //         if ( !hover_index_.has_value() || hover_index_.value() != idx )
// //         {
// //             hover_index_ = idx;
// //             closeSubmenu();
// //             if ( !items_[idx].children.empty() )
// //             {
// //                 openSubmenuForIndex( idx );
// //             }
// //             needs_redraw_ = true;
// //         }
// //
// //         return true;
// //     }
// //
// //     bool
// //     onMousePress( const Event& evt ) override
// //     {
// //         if ( evt.info.mouseButton.button != dr4::MouseButtonType::LEFT )
// //         {
// //             return false;
// //         }
// //
// //         dr4::Vec2f mp( evt.info.mouseButton.pos.x, evt.info.mouseButton.pos.y );
// //         if ( !containsPoint( mp ) )
// //         {
// //             return false;
// //         }
// //
// //         float local_y = mp.y - getAbsPos().y;
// //         int   idx     = static_cast<int>( local_y / ItemHeight );
// //         if ( idx >= 0 && idx < static_cast<int>( items_.size() ) )
// //         {
// //             if ( items_[idx].children.empty() )
// //             {
// //                 if ( items_[idx].on_click )
// //                 {
// //                     items_[idx].on_click();
// //                 }
// //                 request_close_ = true;
// //             } else
// //             {
// //                 if ( submenu_open_for_index_ == idx )
// //                 {
// //                     closeSubmenu();
// //                 } else
// //                 {
// //                     closeSubmenu();
// //                     openSubmenuForIndex( idx );
// //                 }
// //             }
// //             return true;
// //         }
// //
// //         return false;
// //     }
// //
// //     bool
// //     onMouseRelease( const Event& evt ) override
// //     {
// //         return false;
// //     }
// //
// //     void
// //     RedrawChildren() const
// //     {
// //         for ( const auto& c : children_ )
// //         {
// //             c->Redraw();
// //         }
// //     }
// //
// //     bool
// //     containsPoint( const dr4::Vec2f& pt ) const
// //     {
// //         dr4::Vec2f abs = getAbsPos();
// //         dr4::Vec2f sz  = getSize();
// //         return pt.x >= abs.x && pt.x <= abs.x + sz.x && pt.y >= abs.y && pt.y <= abs.y + sz.y;
// //     }
// //
// //     bool
// //     shouldClose() const
// //     {
// //         return request_close_;
// //     }
// //
// //     void
// //     closeSubmenu()
// //     {
// //         children_.clear();
// //         submenu_open_for_index_ = -1;
// //         needs_redraw_           = true;
// //     }
// //
// //   private:
// //     void
// //     openSubmenuForIndex( int idx )
// //     {
// //         if ( idx < 0 || idx >= static_cast<int>( items_.size() ) )
// //         {
// //             return;
// //         }
// //         const auto& children_items = items_[idx].children;
// //         if ( children_items.empty() )
// //         {
// //             return;
// //         }
// //
// //         dr4::Vec2f submenu_pos = { getAbsPos().x + getSize().x, getAbsPos().y + idx *
// ItemHeight
// //         }; auto       submenu     = std::make_unique<MenuPopup>( wm_, submenu_pos,
// children_items
// //         ); submenu->setParent( const_cast<MenuPopup*>( this ) ); children_.push_back(
// std::move(
// //         submenu ) ); submenu_open_for_index_ = idx; needs_redraw_           = true;
// //     }
// //
// //   private:
// //     std::vector<MenuItem>           items_;
// //     std::unique_ptr<dr4::Rectangle> background_;
// //
// //     mutable std::vector<std::unique_ptr<Widget>> children_;
// //     mutable std::optional<int>                   hover_index_            = std::nullopt;
// //     mutable int                                  submenu_open_for_index_ = -1;
// //     bool                                         request_close_          = false;
// //     static constexpr float                       ItemHeight              = 24.0f;
// //     mutable bool                                 needs_redraw_           = true;
// // };
// //
// // // ---------------------------------------------------------------------------
// // // ToolBarFullWindow - single widget that covers full window. Topbar area is
// // // drawn at the top; popups are children positioned anywhere inside the
// // // toolbar's full texture so they never get clipped. The toolbar is intended
// // // to be added as a child of desktop (WindowManager::addWidget).
// // // ---------------------------------------------------------------------------
// //
// // class ToolBarFullWindow : public Widget {
// //   public:
// //     ToolBarFullWindow( WindowManager* wm, float topbar_height = 28.0f )
// //         : Widget( wm, 0.0f, 0.0f, wm->getWindow()->GetSize().x, wm->getWindow()->GetSize().y
// ),
// //           wm_( wm ),
// //           topbar_height_( topbar_height )
// //     {
// //         background_.reset( wm_->getWindow()->CreateRectangle() );
// //         background_->SetPos( { 0, 0 } );
// //         background_->SetSize( { getSize().x, topbar_height_ } );
// //         background_->SetFillColor( { 45, 45, 45, 255 } );
// //
// //         item_x_ = 6.0f;
// //     }
// //
// //     // Add menu definition. The toolbar creates small visible buttons at top bar
// //     // area and will create popups as children on demand.
// //     void
// //     addMenu( const std::string& name, std::vector<MenuItem> items )
// //     {
// //         MenuDef md;
// //         md.name  = name;
// //         md.items = std::move( items );
// //         md.pos   = { item_x_, 0.0f };
// //         md.size  = { 10.0f + static_cast<float>( name.size() ) * 8.0f, topbar_height_ };
// //
// //         // create a visual representation for the topbar button (not a child widget)
// //         menu_defs_.push_back( std::move( md ) );
// //         item_x_ += menu_defs_.back().size.x + 6.0f;
// //     }
// //
// //     void
// //     RedrawMyTexture() const override
// //     {
// //         // full-texture clear
// //         texture_->Clear( { 0, 0, 0, 0 } );
// //         // draw topbar background
// //         texture_->Draw( *background_ );
// //         // draw text labels for items
// //         const dr4::Font* font = wm_->getWindow()->GetDefaultFont();
// //         for ( size_t i = 0; i < menu_defs_.size(); ++i )
// //         {
// //             const auto& md = menu_defs_[i];
// //             auto*       t  = wm_->getWindow()->CreateText();
// //             t->SetFont( font );
// //             t->SetFontSize( 12 );
// //             t->SetText( md.name );
// //             t->SetColor( { 220, 220, 220, 255 } );
// //             // place text inside topbar at md.pos
// //             t->SetPos( { md.pos.x + 6.0f, 6.0f } );
// //             texture_->Draw( *t );
// //         }
// //
// //         for ( const auto& c : children_ )
// //         {
// //             c->Redraw();
// //         }
// //     }
// //
// //     bool
// //     containsPoint( const dr4::Vec2f& pt ) const
// //     {
// //         dr4::Vec2f abs = getAbsPos();
// //         dr4::Vec2f sz  = getSize();
// //         return pt.x >= abs.x && pt.x <= abs.x + sz.x && pt.y >= abs.y && pt.y <= abs.y + sz.y;
// //     }
// //
// //     bool
// //     onMousePress( const Event& evt ) override
// //     {
// //         // fprintf( stderr, "debug in %s:%d:%s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__ );
// //
// //         dr4::Vec2f mp( evt.info.mouseButton.pos.x, evt.info.mouseButton.pos.y );
// //         // if press hits any child popup -> let them handle
// //         for ( auto it = children_.rbegin(); it != children_.rend(); ++it )
// //         {
// //             if ( ( *it )->pointInside( mp ) )
// //             {
// //                 return ( *it )->onMousePress( evt );
// //             }
// //         }
// //
// //         // fprintf( stderr, "debug in %s:%d:%s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__ );
// //
// //         // press on topbar element? store pressed index and return true
// //         if ( mp.y >= 0 && mp.y <= topbar_height_ )
// //         {
// //             for ( size_t i = 0; i < menu_defs_.size(); ++i )
// //             {
// //                 const auto& md = menu_defs_[i];
// //                 if ( mp.x >= md.pos.x && mp.x <= md.pos.x + md.size.x )
// //                 {
// //                     pressed_index_ = static_cast<int>( i );
// //                     return true;
// //                 }
// //             }
// //         }
// //
// //         // fprintf( stderr, "debug in %s:%d:%s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__ );
// //
// //         // click elsewhere closes all popups
// //         closeAllPopups();
// //         return false;
// //     }
// //
// //     bool
// //     onMouseRelease( const Event& evt ) override
// //     {
// //         // std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' << __func__ <<
// //         std::endl;
// //
// //         dr4::Vec2f mp( evt.info.mouseButton.pos.x, evt.info.mouseButton.pos.y );
// //
// //         // if a topbar item was pressed and released -> toggle popup
// //         if ( pressed_index_ >= 0 )
// //         {
// //             int idx        = pressed_index_;
// //             pressed_index_ = -1;
// //             if ( idx >= 0 && idx < static_cast<int>( menu_defs_.size() ) )
// //             {
// //                 // if popup already open for this index, close all; otherwise open this popup
// //                 if ( hasPopupForIndex( idx ) )
// //                 {
// //                     closeAllPopups();
// //                 } else
// //                 {
// //                     // std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' <<
// __func__
// //                     // << std::endl;
// //                     openPopupForIndex( idx );
// //                 }
// //                 return true;
// //             }
// //         }
// //
// //         // forward release to children
// //         for ( auto it = children_.rbegin(); it != children_.rend(); ++it )
// //         {
// //             if ( ( *it )->pointInside( mp ) )
// //             {
// //                 return ( *it )->onMouseRelease( evt );
// //             }
// //         }
// //
// //         return false;
// //     }
// //
// //     bool
// //     onMouseMove( const Event& evt ) override
// //     {
// //         dr4::Vec2f mp( evt.info.mouseMove.pos.x, evt.info.mouseMove.pos.y );
// //
// //         // first, give children a chance
// //         for ( auto it = children_.rbegin(); it != children_.rend(); ++it )
// //         {
// //             if ( ( *it )->pointInside( mp ) )
// //             {
// //                 ( *it )->onMouseMove( evt );
// //                 return true;
// //             }
// //         }
// //
// //         // hover on topbar: open popup if any popup currently open (open-on-hover semantics)
// //         if ( mp.y >= 0 && mp.y <= topbar_height_ )
// //         {
// //             for ( size_t i = 0; i < menu_defs_.size(); ++i )
// //             {
// //                 const auto& md = menu_defs_[i];
// //                 if ( mp.x >= md.pos.x && mp.x <= md.pos.x + md.size.x )
// //                 {
// //                     // highlight (not implemented: visual state) and open on hover if any
// popup
// //                     // exists
// //                     if ( !children_.empty() && !hasPopupForIndex( i ) )
// //                     {
// //                         closeAllPopups();
// //                         openPopupForIndex( i );
// //                     }
// //                     return true;
// //                 }
// //             }
// //         }
// //
// //         return false;
// //     }
// //
// //     // lifecycle helpers
// //     void
// //     openPopupForIndex( size_t idx )
// //     {
// //         if ( idx >= menu_defs_.size() )
// //             return;
// //         const auto& md    = menu_defs_[idx];
// //         dr4::Vec2f  pos   = { md.pos.x, topbar_height_ };
// //         auto        popup = std::make_unique<MenuPopup>( wm_, pos, md.items );
// //         popup->setParent( this );
// //         children_.push_back( std::move( popup ) );
// //         // std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' << __func__ <<
// //         std::endl;
// //     }
// //
// //     void
// //     closeAllPopups()
// //     {
// //         children_.clear();
// //     }
// //
// //     bool
// //     hasPopupForIndex( size_t idx ) const
// //     {
// //         if ( children_.empty() )
// //             return false;
// //         // crude check: child popup pos.x equals menu pos.x
// //         const auto& md = menu_defs_[idx];
// //         for ( const auto& c : children_ )
// //         {
// //             auto p = c->getRelPos();
// //             if ( std::abs( p.x - md.pos.x ) < 0.5f )
// //                 return true;
// //         }
// //         return false;
// //     }
// //
// //     // expose children for external checks (e.g. WindowManager::handleEvents)
// //     const std::vector<std::unique_ptr<Widget>>&
// //     getPopups() const
// //     {
// //         return children_;
// //     }
// //
// //   private:
// //     struct MenuDef
// //     {
// //         std::string           name;
// //         std::vector<MenuItem> items;
// //         dr4::Vec2f            pos;
// //         dr4::Vec2f            size;
// //     };
// //
// //     WindowManager*                       wm_            = nullptr;
// //     float                                topbar_height_ = 28.0f;
// //     std::unique_ptr<dr4::Rectangle>      background_;
// //     std::vector<MenuDef>                 menu_defs_;
// //     std::vector<std::unique_ptr<Widget>> children_; // popups
// //
// //     float item_x_        = 0.0f;
// //     int   pressed_index_ = -1;
// // };
// //
// // } // namespace hui
// //
// // // ---------------------------------------------------------------------------
// // // Usage notes (not part of code):
// // // - Add a ToolBarFullWindow to WindowManager:
// // // wm->addWidget(std::make_unique<hui::ToolBarFullWindow>(wm, 28.0f));
// // // - Then call toolbar->addMenu(name, items) to populate.
// // // - MenuItem.children defines nested menus. Callbacks are invoked on click.
// // // - The toolbar uses the widget texture sized to the entire window so popups are never
// clipped.
// // // - You may want to add minor improvements: hover open delay (timer), keyboard navigation,
// // //   visual hover state for topbar items, smooth repositioning when window is resized.
//
// #pragma once
//
// #include "custom-hui-impl/container_widget.hpp"
// #include "dr4/math/color.hpp"
// #include "dr4/math/vec2.hpp"
// #include "dr4/texture.hpp"
// #include "dr4/window.hpp"
// #include "widget.hpp"
// #include "window_manager.hpp"
// #include <algorithm>
// #include <functional>
// #include <memory>
// #include <optional>
// #include <string>
// #include <vector>
//
// namespace hui {
//
// // ---------------------------------------------------------------------------
// // Menu data structures (public)
// // ---------------------------------------------------------------------------
//
// struct MenuItem
// {
//     std::string           label;
//     std::function<void()> on_click;
//     std::vector<MenuItem> children; // nested submenu
// };
//
// // ---------------------------------------------------------------------------
// // MenuPopup - исправленная версия с правильной логикой отображения и закрытия
// // ---------------------------------------------------------------------------
//
// class MenuPopup : public VectorContainerWidget {
//   public:
//     MenuPopup( WindowManager* wm, const dr4::Vec2f& pos, const std::vector<MenuItem>& items )
//         : VectorContainerWidget(
//               wm,
//               pos,
//               { 160.0f, 24.0f * static_cast<float>( std::max<size_t>( 1, items.size() ) ) } ),
//           items_( items )
//     {
//         background_.reset( wm->getWindow()->CreateRectangle() );
//         background_->SetSize( getSize() );
//         background_->SetFillColor( { 40, 40, 40, 255 } );
//         background_->SetBorderColor( { 80, 80, 80, 255 } );
//         background_->SetBorderThickness( -1.0f );
//
//         // Создаем текстовые элементы один раз при инициализации
//         createTextElements();
//     }
//
//     void
//     RedrawMyTexture() const override
//     {
//         texture_->Clear( { 0, 0, 0, 0 } );
//         texture_->Draw( *background_ );
//
//         // Рисуем все элементы меню
//         for ( size_t i = 0; i < items_.size(); ++i )
//         {
//             // Подсветка при наведении
//             if ( hover_index_.has_value() && hover_index_.value() == static_cast<int>( i ) )
//             {
//                 auto* highlight = wm_->getWindow()->CreateRectangle();
//                 highlight->SetPos( { 0, static_cast<float>( i ) * ItemHeight } );
//                 highlight->SetSize( { getSize().x, ItemHeight } );
//                 highlight->SetFillColor( { 60, 60, 60, 255 } );
//                 texture_->Draw( *highlight );
//             }
//
//             // Рисуем текст элемента
//             if ( i < text_elements_.size() && text_elements_[i] )
//             {
//                 texture_->Draw( *text_elements_[i] );
//             }
//         }
//
//         // Рисуем подменю (если есть)
//         RedrawChildren();
//     }
//
//     bool
//     onMouseMove( const Event& evt ) override
//     {
//         dr4::Vec2f mp( evt.info.mouseMove.pos.x, evt.info.mouseMove.pos.y );
//         dr4::Vec2f local_pos = mp - getAbsPos();
//
//         // Проверяем, находится ли курсор внутри меню
//         if ( local_pos.y < 0 || local_pos.y > getSize().y || local_pos.x < 0 ||
//              local_pos.x > getSize().x )
//         {
//             hover_index_ = std::nullopt;
//             return Widget::onMouseMove( evt );
//         }
//
//         int idx = static_cast<int>( local_pos.y / ItemHeight );
//         if ( idx < 0 || idx >= static_cast<int>( items_.size() ) )
//         {
//             hover_index_ = std::nullopt;
//             return Widget::onMouseMove( evt );
//         }
//
//         // Если наведение изменилось
//         if ( !hover_index_.has_value() || hover_index_.value() != idx )
//         {
//             hover_index_ = idx;
//
//             // Закрываем текущее подменю
//             closeSubmenu();
//
//             // Открываем новое подменю при наведении
//             if ( !items_[idx].children.empty() )
//             {
//                 openSubmenuForIndex( idx );
//             }
//         }
//
//         return Widget::onMouseMove( evt );
//     }
//
//     bool
//     onMousePress( const Event& evt ) override
//     {
//         // Обрабатываем только левую кнопку мыши
//         if ( evt.info.mouseButton.button != dr4::MouseButtonType::LEFT )
//         {
//             return false;
//         }
//
//         dr4::Vec2f mp( evt.info.mouseButton.pos.x, evt.info.mouseButton.pos.y );
//
//         // Если клик вне меню - закрываем его
//         if ( !containsPoint( mp ) )
//         {
//             request_close_ = true;
//             return true;
//         }
//
//         // Координаты относительно меню
//         dr4::Vec2f local_pos = mp - getAbsPos();
//         int        idx       = static_cast<int>( local_pos.y / ItemHeight );
//
//         // Проверяем корректность индекса
//         if ( idx < 0 || idx >= static_cast<int>( items_.size() ) )
//         {
//             return false;
//         }
//
//         // Если у элемента есть подменю
//         if ( !items_[idx].children.empty() )
//         {
//             // Переключаем подменю
//             if ( submenu_open_for_index_ == idx && submenu_ )
//             {
//                 closeSubmenu();
//             } else
//             {
//                 closeSubmenu();
//                 openSubmenuForIndex( idx );
//             }
//             return true;
//         }
//
//         // Если у элемента нет подменю - выполняем действие
//         if ( items_[idx].on_click )
//         {
//             items_[idx].on_click();
//         }
//
//         // Помечаем для закрытия
//         request_close_ = true;
//         return true;
//     }
//
//     bool
//     onMouseRelease( const Event& evt ) override
//     {
//         return false; // Обрабатывается в onMousePress
//     }
//
//     bool
//     containsPoint( const dr4::Vec2f& pt ) const
//     {
//         dr4::Vec2f abs = getAbsPos();
//         dr4::Vec2f sz  = getSize();
//         return pt.x >= abs.x && pt.x <= abs.x + sz.x && pt.y >= abs.y && pt.y <= abs.y + sz.y;
//     }
//
//     bool
//     shouldClose() const
//     {
//         return request_close_;
//     }
//
//     void
//     closeSubmenu()
//     {
//         if ( submenu_ )
//         {
//             submenu_.reset();
//         }
//         submenu_open_for_index_ = -1;
//     }
//
//     void
//     closeAll()
//     {
//         closeSubmenu();
//         request_close_ = true;
//     }
//
//   private:
//     void
//     createTextElements()
//     {
//         const dr4::Font* font = wm_->getWindow()->GetDefaultFont();
//
//         text_elements_.reserve( items_.size() );
//         for ( size_t i = 0; i < items_.size(); ++i )
//         {
//             auto* text = wm_->getWindow()->CreateText();
//             text->SetFont( font );
//             text->SetFontSize( 12 );
//             text->SetText( items_[i].label );
//             text->SetColor( { 220, 220, 220, 255 } );
//             text->SetPos( { 4.0f, static_cast<float>( i ) * ItemHeight + 4.0f } );
//             text_elements_.push_back( std::unique_ptr<dr4::Text>( text ) );
//         }
//     }
//
//     void
//     openSubmenuForIndex( int idx )
//     {
//         if ( idx < 0 || idx >= static_cast<int>( items_.size() ) || items_[idx].children.empty()
//         )
//         {
//             return;
//         }
//
//         // Позиция подменю: справа от текущего меню, на уровне выбранного элемента
//         dr4::Vec2f submenu_pos = { getAbsPos().x + getSize().x, getAbsPos().y + idx * ItemHeight
//         };
//
//         // Создаем подменю
//         submenu_ = std::make_unique<MenuPopup>( wm_, submenu_pos, items_[idx].children );
//         submenu_->setParent( this );
//         submenu_open_for_index_ = idx;
//     }
//
//   private:
//     std::vector<MenuItem>           items_;
//     std::unique_ptr<dr4::Rectangle> background_;
//
//     // Текстовые элементы создаются один раз при инициализации
//     std::vector<std::unique_ptr<dr4::Text>> text_elements_;
//
//     std::unique_ptr<MenuPopup> submenu_;
//     std::optional<int>         hover_index_            = std::nullopt;
//     int                        submenu_open_for_index_ = -1;
//     bool                       request_close_          = false;
//     static constexpr float     ItemHeight              = 24.0f;
// };
//
// // ---------------------------------------------------------------------------
// // ToolBar - исправленная версия с правильной архитектурой
// // ---------------------------------------------------------------------------
// class ToolBar : public Widget {
//   public:
//     ToolBar( WindowManager* wm, float height = 28.0f )
//         : Widget( wm, 0.0f, 0.0f, wm->getWindow()->GetSize().x, height ),
//           wm_( wm ),
//           height_( height )
//     {
//         background_.reset( wm->getWindow()->CreateRectangle() );
//         background_->SetPos( { 0, 0 } );
//         background_->SetSize( { getSize().x, height_ } );
//         background_->SetFillColor( { 45, 45, 45, 255 } );
//
//         item_x_ = 10.0f;
//     }
//
//     void
//     addMenu( const std::string& name, std::vector<MenuItem> items )
//     {
//         MenuDef md;
//         md.name  = name;
//         md.items = std::move( items );
//         md.pos   = { item_x_, 0.0f };
//         md.size  = { 10.0f + static_cast<float>( name.size() ) * 8.0f, height_ };
//
//         menu_defs_.push_back( std::move( md ) );
//         item_x_ += menu_defs_.back().size.x + 10.0f;
//     }
//
//     void
//     RedrawMyTexture() const override
//     {
//         texture_->Clear( { 0, 0, 0, 0 } );
//         texture_->Draw( *background_ );
//
//         const dr4::Font* font = wm_->getWindow()->GetDefaultFont();
//
//         for ( size_t i = 0; i < menu_defs_.size(); ++i )
//         {
//             const auto& md = menu_defs_[i];
//
//             // Подсветка при наведении
//             if ( hovered_menu_ == static_cast<int>( i ) )
//             {
//                 auto* highlight = wm_->getWindow()->CreateRectangle();
//                 highlight->SetPos( md.pos );
//                 highlight->SetSize( md.size );
//                 highlight->SetFillColor( { 60, 60, 60, 255 } );
//                 texture_->Draw( *highlight );
//             }
//
//             // Текст пункта меню
//             auto* text = wm_->getWindow()->CreateText();
//             text->SetFont( font );
//             text->SetFontSize( 12 );
//             text->SetText( md.name );
//             text->SetColor( { 220, 220, 220, 255 } );
//             text->SetPos( { md.pos.x + 6.0f, 6.0f } );
//             texture_->Draw( *text );
//         }
//     }
//
//     bool
//     onMouseMove( const Event& evt ) override
//     {
//         dr4::Vec2f mp( evt.info.mouseMove.pos.x, evt.info.mouseMove.pos.y );
//         dr4::Vec2f local_pos = mp - getAbsPos();
//
//         // Проверяем наведение на пункты меню
//         int new_hover = -1;
//         if ( local_pos.y >= 0 && local_pos.y <= height_ )
//         {
//             for ( size_t i = 0; i < menu_defs_.size(); ++i )
//             {
//                 if ( local_pos.x >= menu_defs_[i].pos.x &&
//                      local_pos.x <= menu_defs_[i].pos.x + menu_defs_[i].size.x )
//                 {
//                     new_hover = static_cast<int>( i );
//                     break;
//                 }
//             }
//         }
//
//         // Если наведение изменилось
//         if ( new_hover != hovered_menu_ )
//         {
//             hovered_menu_ = new_hover;
//         }
//
//         return Widget::onMouseMove( evt );
//     }
//
//     bool
//     onMousePress( const Event& evt ) override
//     {
//         if ( evt.info.mouseButton.button != dr4::MouseButtonType::LEFT )
//         {
//             return false;
//         }
//
//         dr4::Vec2f mp( evt.info.mouseButton.pos.x, evt.info.mouseButton.pos.y );
//         dr4::Vec2f local_pos = mp - getAbsPos();
//
//         // Если клик вне топбара
//         if ( local_pos.y < 0 || local_pos.y > height_ )
//         {
//             // Закрываем все открытые меню
//             closeAllPopups();
//             return false;
//         }
//
//         // Проверяем, по какому пункту меню был клик
//         for ( size_t i = 0; i < menu_defs_.size(); ++i )
//         {
//             if ( local_pos.x >= menu_defs_[i].pos.x &&
//                  local_pos.x <= menu_defs_[i].pos.x + menu_defs_[i].size.x )
//             {
//                 // Если уже есть открытое меню для этого пункта - закрываем все
//                 if ( hasPopupForIndex( i ) )
//                 {
//                     closeAllPopups();
//                 }
//                 // Иначе открываем меню
//                 else
//                 {
//                     closeAllPopups();
//                     openPopupForIndex( i );
//                 }
//                 return true;
//             }
//         }
//
//         return false;
//     }
//
//     bool
//     onMouseRelease( const Event& evt ) override
//     {
//         return false; // Обрабатывается в onMousePress
//     }
//
//     bool
//     onIdle( const Event& evt ) override
//     {
//         // Проверяем необходимость закрытия popup меню
//         for ( auto it = popups_.begin(); it != popups_.end(); )
//         {
//             if ( ( *it )->shouldClose() )
//             {
//                 it = popups_.erase( it );
//             } else
//             {
//                 ++it;
//             }
//         }
//
//         return false;
//     }
//
//     void
//     closeAllPopups()
//     {
//         popups_.clear();
//     }
//
//   private:
//     void
//     openPopupForIndex( size_t idx )
//     {
//         if ( idx >= menu_defs_.size() )
//         {
//             return;
//         }
//
//         const auto& md        = menu_defs_[idx];
//         dr4::Vec2f  popup_pos = { getAbsPos().x + md.pos.x, getAbsPos().y + height_ };
//
//         auto popup = std::make_unique<MenuPopup>( wm_, popup_pos, md.items );
//         popups_.push_back( std::move( popup ) );
//
//         std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' << __func__ << std::endl;
//     }
//
//     bool
//     hasPopupForIndex( size_t idx ) const
//     {
//         if ( popups_.empty() )
//             return false;
//
//         const auto& md = menu_defs_[idx];
//         for ( const auto& popup : popups_ )
//         {
//             dr4::Vec2f popup_pos = popup->getAbsPos();
//             dr4::Vec2f menu_pos  = getAbsPos() + md.pos;
//             // Проверяем, принадлежит ли popup к этому пункту меню
//             if ( std::abs( popup_pos.x - menu_pos.x ) < 0.5f )
//             {
//                 return true;
//             }
//         }
//         return false;
//     }
//
//     struct MenuDef
//     {
//         std::string           name;
//         std::vector<MenuItem> items;
//         dr4::Vec2f            pos;
//         dr4::Vec2f            size;
//     };
//
//     WindowManager*                          wm_     = nullptr;
//     float                                   height_ = 28.0f;
//     std::unique_ptr<dr4::Rectangle>         background_;
//     std::vector<MenuDef>                    menu_defs_;
//     std::vector<std::unique_ptr<MenuPopup>> popups_;
//     float                                   item_x_       = 0.0f;
//     int                                     hovered_menu_ = -1;
// };
//
// } // namespace hui

// tool_bar.hpp
#pragma once

#include "button.hpp"
#include "container_widget.hpp"
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
    MenuPopup( WindowManager* wm, const dr4::Vec2f& pos, const std::vector<MenuItem>& items );

    void
    RedrawMyTexture() const override;
    bool
    onMousePress( const Event& event ) override;
    bool
    containsPoint( const dr4::Vec2f& pt ) const;

  private:
    void
    createTextElements();

  private:
    std::vector<MenuItem>                   items_;
    std::unique_ptr<dr4::Rectangle>         background_;
    std::vector<std::unique_ptr<dr4::Text>> text_elements_;
    static constexpr float                  ItemHeight = 24.0f;
};

class ToolBar : public Widget {
  public:
    ToolBar( WindowManager* wm, float height = 28.0f );

    void
    addMenu( const std::string& name, std::vector<MenuItem> items );
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
        dr4::Vec2f            pos;
        dr4::Vec2f            size;
    };

    void
    createTextElements();

  private:
    WindowManager*                          wm_     = nullptr;
    float                                   height_ = 28.0f;
    std::unique_ptr<dr4::Rectangle>         background_;
    std::vector<MenuDef>                    menu_defs_;
    std::vector<std::unique_ptr<dr4::Text>> text_elements_;
    float                                   item_x_        = 10.0f;
    int                                     pressed_index_ = -1;
    int                                     hovered_menu_  = -1;
};

} // namespace hui
