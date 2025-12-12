#pragma once
#include "hui/button.hpp"
#include "hui/dialog_box.hpp"
#include "hui/label.hpp"
#include "hui/scrollable_buttons_list_widget.hpp"
#include "rta/model/primitives/impls/aabb.hpp"
#include "rta/model/primitives/impls/plane.hpp"
#include "rta/model/primitives/impls/sphere.hpp"
#include "rta/model/rendering/scene_manager.hpp"
#include "rta/view/object_params_dialogs.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace rta {
namespace view {

class SceneObjectsListModal : public hui::DialogBox {
  public:
    using CloseCb = std::function<void()>;
    SceneObjectsListModal( hui::WindowManager*           wm,
                           float                         x,
                           float                         y,
                           float                         w,
                           float                         h,
                           model::SceneManager&          scene_manager,
                           CloseCb                       close_cb,
                           std::function<void( size_t )> on_select )
        : hui::DialogBox(
              wm,
              x,
              y,
              w,
              h,
              [this, close_cb]() {
                  this->hide();
                  if ( close_cb )
                  {
                      close_cb();
                  }
              },
              "Objects list" ),
          scene_manager_( scene_manager ),
          close_cb_( std::move( close_cb ) ),
          on_select_( std::move( on_select ) )
    {
        const float inner_x         = 12.0f;
        const float inner_y         = TopBarHeight + 12.0f;
        const float inner_w         = w - inner_x - 12.0f;
        const float inner_h         = h - ( inner_y + 12.0f );
        const float scrollbar_width = 12.0f;

        // Create scrollable list of buttons and make it a child of the modal
        list_ = std::make_unique<hui::ScrollableButtonsListWidget>(
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
        if ( !visible_ )
        {
            return;
        }
        rebuildListItems();
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        if ( !visible_ )
        {
            return false;
        }

        if ( event.apply( list_.get() ) )
        {
            return true;
        }
        return hui::DialogBox::propagateEventToChildren( event );
    }

    void
    RedrawMyTexture() const override
    {
        if ( !visible_ )
        {
            return;
        }

        hui::DialogBox::RedrawMyTexture();
        if ( list_ )
        {
            list_->Redraw();
        }
    }

    void
    show()
    {
        visible_ = true;
        refresh();
    }

    void
    hide()
    {
        visible_ = false;
    }

    bool
    isVisible() const
    {
        return visible_;
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
            new hui::ScrollableButtonsListWidget( wm_,
                                                  dr4::Vec2f{ inner_x, inner_y },
                                                  dr4::Vec2f{ inner_w - scrollbar_width, inner_h },
                                                  scrollbar_width ) );
        list_->setParent( this );

        // Get object names
        std::vector<std::string> names;
        std::vector<size_t>      object_indices;
        for ( size_t i = 0; i < scene_manager_.getObjectsCount(); ++i )
        {
            auto        info  = scene_manager_.getObjectInfo( i );
            std::string label = info.display_name.empty() ? info.type_name : info.display_name;
            names.push_back( label + " (" + std::to_string( i ) + ")" );
            object_indices.push_back( info.objects_idx );
        }

        const float item_h = 22.0f;
        const float item_w = inner_w - scrollbar_width - 8.0f;

        // Create buttons instead of labels
        for ( size_t i = 0; i < names.size(); ++i )
        {
            auto               item_idx = object_indices[i]; // Capture the actual object index
            hui::Button::Theme theme{ dr4::Color{ 50, 50, 50, 255 },
                                      dr4::Color{ 70, 70, 70, 255 },
                                      dr4::Color{ 30, 30, 30, 255 },
                                      dr4::Color{ 220, 220, 220, 255 },
                                      13 };
            auto               btn = std::make_unique<hui::Button>( wm_,
                                                      dr4::Vec2f{ 0.0f, 0.0f },
                                                      dr4::Vec2f{ item_w, item_h },
                                                      names[i],
                                                      theme );

            btn->setOnClick( [this, item_idx]() {
                if ( on_select_ )
                {
                    on_select_( item_idx );
                }
            } );

            list_->addButton( std::move( btn ) );
        }

        list_->rebuildLayout();
    }

  private:
    model::SceneManager&                       scene_manager_;
    CloseCb                                    close_cb_;
    std::unique_ptr<hui::ScrollableButtonsListWidget> list_;
    std::function<void( size_t )>              on_select_;
    bool                                       visible_ = true;
};

} // namespace view
} // namespace rta
