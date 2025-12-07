#pragma once

#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/container_widget.hpp"
#include "custom-hui-impl/window_manager.hpp"
#include "dr4/texture.hpp"
#include <memory>

namespace zemax {
namespace view {

class ObjParams : public hui::ContainerWidget {
    using CloseCallback = std::function<void()>;

  public:
    ObjParams( hui::WindowManager* wm,
               float               x,
               float               y,
               float               w,
               float               h,
               CloseCallback       close_callback )
        : hui::ContainerWidget( wm, x, y, w, h ),
          rect_( wm->getWindow()->CreateRectangle() ),
          close_btn_( wm,
                      { 0, 0 },
                      { 20, 20 },
                      { 32, 0, 0, 255 },
                      { 64, 64, 64, 255 },
                      { 96, 96, 96, 255 },
                      "X",
                      { 255, 255, 255, 255 },
                      12 )
    {
        rect_->SetBorderColor( { 118, 185, 0, 255 } );
        rect_->SetBorderThickness( -2.0f );
        rect_->SetFillColor( { 32, 32, 32, 255 } );
        rect_->SetPos( { 0, 0 } );
        rect_->SetSize( { w, h } );

        close_btn_.setParent( this );
        close_btn_.setOnClick( close_callback );
    }

    bool
    propagateEventToChildren( const hui::Event& evt ) override
    {
        return evt.apply( &close_btn_ );
    }

    void
    RedrawMyTexture() const override
    {
        texture_->Draw( *rect_ );
        close_btn_.Redraw();
    }

  private:
    hui::Button close_btn_;

    std::unique_ptr<dr4::Rectangle> rect_;
};

} // namespace view
} // namespace zemax
