// #pragma once
//
// #include "custom-hui-impl/button.hpp"
// #include "custom-hui-impl/label.hpp"
// #include "custom-hui-impl/scrollable_list_widget.hpp"
// #include "obj_info_box.hpp"
// #include "zemax/model/rendering/scene_manager.hpp"
// #include <functional>
// #include <memory>
// #include <string>
// #include <vector>
//
// namespace zemax {
// namespace view {
//
// class SceneObjectsListModal : public ObjInfoBox {
//   public:
//     using CloseCb = std::function<void()>;
//
//     SceneObjectsListModal( hui::WindowManager*  wm,
//                            float                x,
//                            float                y,
//                            float                w,
//                            float                h,
//                            model::SceneManager& scene_manager,
//                            CloseCb              close_cb )
//         : ObjInfoBox( wm, x, y, w, h, close_cb, "Objects list" ),
//           scene_manager_( scene_manager ),
//           close_cb_( std::move( close_cb ) )
//     {
//         const float inner_x = 12.0f;
//         const float inner_y = TopBarHeight + 12.0f;
//         const float inner_w = w - inner_x - 12.0f;
//         const float inner_h = h - ( inner_y + 12.0f );
//
//         const float scrollbar_width = 12.0f;
//
//         // Создаём ScrollableListWidget и делаем его ребёнком модалки
//         list_ = std::make_unique<hui::ScrollableListWidget>(
//             wm,
//             dr4::Vec2f{ inner_x, inner_y },
//             dr4::Vec2f{ inner_w - scrollbar_width, inner_h },
//             scrollbar_width );
//         list_->setParent( this );
//
//         rebuildListItems();
//     }
//
//     void
//     refresh()
//     {
//         rebuildListItems();
//     }
//
//     bool
//     propagateEventToChildren( const hui::Event& event ) override
//     {
//         if ( event.apply( list_.get() ) )
//         {
//             return true;
//         }
//
//         return ObjInfoBox::propagateEventToChildren( event );
//     }
//
//     void
//     RedrawMyTexture() const override
//     {
//         ObjInfoBox::RedrawMyTexture();
//
//         if ( list_ )
//         {
//             list_->Redraw();
//         }
//     }
//
//   private:
//     void
//     rebuildListItems()
//     {
//         if ( !list_ )
//         {
//             return;
//         }
//
//         // Пересоздаём список заново — это простой и безопасный способ очистки старых элементов.
//         const float inner_x         = 12.0f;
//         const float inner_y         = TopBarHeight + 12.0f;
//         const float inner_w         = size_.x - inner_x - 12.0f;
//         const float inner_h         = size_.y - ( inner_y + 12.0f );
//         const float scrollbar_width = 12.0f;
//
//         list_.reset(
//             new hui::ScrollableListWidget( wm_,
//                                            dr4::Vec2f{ inner_x, inner_y },
//                                            dr4::Vec2f{ inner_w - scrollbar_width, inner_h },
//                                            scrollbar_width ) );
//         list_->setParent( this );
//
//         std::vector<std::string> names;
//         for ( size_t i = 0; i < scene_manager_.getObjectsCount(); ++i )
//         {
//             names.push_back( scene_manager_.getObjectInfo( i ).type_name );
//         }
//
//         std::cerr << "names.size(): " << names.size() << std::endl;
//         for ( const auto& name : names )
//         {
//             std::cerr << name << std::endl;
//         }
//
//         const float item_h = 22.0f;
//         const float item_w = inner_w - scrollbar_width - 8.0f;
//
//         for ( const auto& n : names )
//         {
//             auto lbl = std::make_unique<hui::LabelWidget>( wm_,
//                                                            dr4::Vec2f{ 0.0f, 0.0f },
//                                                            dr4::Vec2f{ item_w, item_h },
//                                                            n,
//                                                            13 );
//             list_->addItem( std::move( lbl ) );
//         }
//
//         list_->rebuildLayout();
//     }
//
//   private:
//     model::SceneManager&                       scene_manager_;
//     CloseCb                                    close_cb_;
//     std::unique_ptr<hui::ScrollableListWidget> list_;
// };
//
// } // namespace view
// } // namespace zemax

// zemax/view/scene_objects_list.hpp
#pragma once
#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/label.hpp"
#include "custom-hui-impl/scrollable_list_widget.hpp"
#include "obj_info_box.hpp"
#include "zemax/model/primitives/impls/aabb.hpp"
#include "zemax/model/primitives/impls/plane.hpp"
#include "zemax/model/primitives/impls/sphere.hpp"
#include "zemax/model/rendering/scene_manager.hpp"
#include "zemax/view/aabb_params_dialog.hpp"
#include "zemax/view/sphere_params_dialog.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace zemax {
namespace view {

class SceneObjectsListModal : public ObjInfoBox {
  public:
    using CloseCb = std::function<void()>;
    SceneObjectsListModal( hui::WindowManager*  wm,
                           float                x,
                           float                y,
                           float                w,
                           float                h,
                           model::SceneManager& scene_manager,
                           CloseCb              close_cb )
        : ObjInfoBox( wm, x, y, w, h, close_cb, "Objects list" ),
          scene_manager_( scene_manager ),
          close_cb_( std::move( close_cb ) )
    {
        const float inner_x         = 12.0f;
        const float inner_y         = TopBarHeight + 12.0f;
        const float inner_w         = w - inner_x - 12.0f;
        const float inner_h         = h - ( inner_y + 12.0f );
        const float scrollbar_width = 12.0f;

        // Create ScrollableListWidget and make it a child of the modal
        list_ = std::make_unique<hui::ScrollableListWidget>(
            wm,
            dr4::Vec2f{ inner_x, inner_y },
            dr4::Vec2f{ inner_w - scrollbar_width, inner_h },
            scrollbar_width );
        list_->setParent( this );
        rebuildListItems();
    }

    void
    refresh()
    {
        rebuildListItems();
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        if ( event.apply( list_.get() ) )
        {
            return true;
        }
        return ObjInfoBox::propagateEventToChildren( event );
    }

    void
    RedrawMyTexture() const override
    {
        ObjInfoBox::RedrawMyTexture();
        if ( list_ )
        {
            list_->Redraw();
        }
    }

  private:
    void
    rebuildListItems()
    {
        if ( !list_ )
        {
            return;
        }

        // Recreate list from scratch - this is a simple and safe way to reset elements
        const float inner_x         = 12.0f;
        const float inner_y         = TopBarHeight + 12.0f;
        const float inner_w         = size_.x - inner_x - 12.0f;
        const float inner_h         = size_.y - ( inner_y + 12.0f );
        const float scrollbar_width = 12.0f;

        list_.reset(
            new hui::ScrollableListWidget( wm_,
                                           dr4::Vec2f{ inner_x, inner_y },
                                           dr4::Vec2f{ inner_w - scrollbar_width, inner_h },
                                           scrollbar_width ) );
        list_->setParent( this );

        // Get object names
        std::vector<std::string> names;
        std::vector<size_t>      object_indices;
        for ( size_t i = 0; i < scene_manager_.getObjectsCount(); ++i )
        {
            auto info = scene_manager_.getObjectInfo( i );
            names.push_back( info.type_name + " (" + std::to_string( i ) + ")" );
            object_indices.push_back( info.objects_idx );
        }

        const float item_h = 22.0f;
        const float item_w = inner_w - scrollbar_width - 8.0f;

        // Create buttons instead of labels
        for ( size_t i = 0; i < names.size(); ++i )
        {
            auto item_idx = object_indices[i]; // Capture the actual object index
            auto btn =
                std::make_unique<hui::Button>( wm_,
                                               dr4::Vec2f{ 0.0f, 0.0f },
                                               dr4::Vec2f{ item_w, item_h },
                                               dr4::Color{ 50, 50, 50, 255 },    // Default color
                                               dr4::Color{ 70, 70, 70, 255 },    // Hover color
                                               dr4::Color{ 30, 30, 30, 255 },    // Pressed color
                                               names[i],
                                               dr4::Color{ 220, 220, 220, 255 }, // Text color
                                               13                                // Font size
                );

            // Set callback for button click
            btn->setOnClick( [this, item_idx]() {
                scene_manager_.setTargetObj( scene_manager_.getObjects()[item_idx].get() );
                openEditDialog( item_idx );
            } );

            list_->addItem( std::move( btn ) );
        }

        list_->rebuildLayout();
    }

    void
    openEditDialog( size_t obj_idx )
    {
        // Get object info
        auto obj_info = scene_manager_.getObjectInfo( obj_idx );

        // Determine the type of object and open appropriate dialog
        if ( obj_info.type_name == "Sphere" )
        {
            // Open sphere parameter dialog
            wm_->pushModal( std::make_unique<SphereParamsDialog>( wm_,
                                                                  800,
                                                                  250,
                                                                  500,
                                                                  400,
                                                                  scene_manager_,
                                                                  obj_idx,
                                                                  [this]() {
                                                                      wm_->popModal();
                                                                      this->refresh();
                                                                      scene_manager_.setTargetObj(
                                                                          nullptr );
                                                                  } ) );
        } else if ( obj_info.type_name == "AABB" )
        {
            // Open AABB parameter dialog
            wm_->pushModal(
                std::make_unique<
                    AABBParamsDialog>( wm_, 800, 250, 550, 450, scene_manager_, obj_idx, [this]() {
                    wm_->popModal();
                    this->refresh();
                    scene_manager_.setTargetObj( nullptr );
                } ) );
        }
        // Add more object types as needed
    }

  private:
    model::SceneManager&                       scene_manager_;
    CloseCb                                    close_cb_;
    std::unique_ptr<hui::ScrollableListWidget> list_;
};

} // namespace view
} // namespace zemax
