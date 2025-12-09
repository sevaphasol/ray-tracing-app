#pragma once

#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/label.hpp"
#include "custom-hui-impl/widget.hpp"
#include "custom-hui-impl/window_manager.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include <cstdlib>
#include <memory>
#include <string>

namespace hui {

// Generic dialog panel with a top bar, title and close button.
class DialogBox : public hui::ContainerWidget {
  private:
    std::unique_ptr<dr4::Rectangle> rect_;
    std::unique_ptr<dr4::Rectangle> top_bar_;
    hui::LabelWidget                label_;

  protected:
    hui::Button close_btn_;
    using CloseBtnCallBack = std::function<void()>;

    static constexpr float         TopBarHeight          = 25.0f;
    static constexpr float         TopBarBorderThickness = -2.0f;
    static constexpr float         RectBorderThickness   = -2.0f;
    static constexpr float         CloseBtnSize = TopBarHeight - std::abs( TopBarBorderThickness );
    static inline const size_t     CloseBtnFontSize     = 12;
    static inline const dr4::Color CloseBtnDefaultColor = { 128, 0, 0, 255 };
    static inline const dr4::Color CloseBtnHoveredColor = { 164, 0, 0, 255 };
    static inline const dr4::Color CloseBtnPressedColor = { 96, 0, 0, 255 };
    static inline const dr4::Color CloseBtnFontColor    = { 255, 255, 255, 255 };
    static inline const dr4::Color RectFillColor        = { 32, 32, 32, 128 };
    static inline const dr4::Color RectBorderColor      = { 118, 185, 0, 255 };
    static inline const dr4::Color TopBarFillColor      = { 32, 32, 32, 128 };
    static inline const dr4::Color TopBarBorderColor    = { 118, 185, 0, 255 };
    static inline const dr4::Color LabelFontColor       = { 255, 255, 255, 255 };
    static constexpr float         LabelFontSize        = 12;
    static constexpr float         LabelPadX            = std::abs( TopBarBorderThickness ) + 2.0f;
    static constexpr float         LabelPadY            = std::abs( TopBarBorderThickness ) + 2.0f;

  public:
    DialogBox( hui::WindowManager* wm,
               float               x,
               float               y,
               float               w,
               float               h,
               CloseBtnCallBack    close_call_back,
               const std::string&  label )
        : hui::ContainerWidget( wm, x, y, w, h ),
          close_btn_( wm,
                      {
                          w - CloseBtnSize - std::abs( TopBarBorderThickness ),
                          std::abs( TopBarBorderThickness ),
                      },
                      { CloseBtnSize, CloseBtnSize },
                      CloseBtnDefaultColor,
                      CloseBtnHoveredColor,
                      CloseBtnPressedColor,
                      "X",
                      CloseBtnFontColor,
                      CloseBtnFontSize ),
          label_( wm, { LabelPadX, LabelPadY }, { w - CloseBtnSize, CloseBtnSize }, label )
    {
        setDraggable( true );

        rect_.reset( wm->getWindow()->CreateRectangle() );
        rect_->SetSize( { w, h - TopBarHeight } );
        rect_->SetPos( { 0, TopBarHeight } );
        rect_->SetFillColor( RectFillColor );
        rect_->SetBorderColor( RectBorderColor );
        rect_->SetBorderThickness( RectBorderThickness );

        top_bar_.reset( wm->getWindow()->CreateRectangle() );
        top_bar_->SetSize( { w, TopBarHeight + std::abs( TopBarBorderThickness ) } );
        top_bar_->SetPos( { 0, 0 } );
        top_bar_->SetFillColor( TopBarFillColor );
        top_bar_->SetBorderColor( TopBarBorderColor );
        top_bar_->SetBorderThickness( TopBarBorderThickness );

        close_btn_.setParent( this );
        close_btn_.setOnClick( close_call_back );

        label_.setParent( this );
        label_.setText( label );
        label_.setRelPos( LabelPadX, LabelPadY );
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        return event.apply( &close_btn_ );
    }

    virtual void
    RedrawMyTexture() const override
    {
        rect_->DrawOn( *texture_ );
        top_bar_->DrawOn( *texture_ );
        close_btn_.Redraw();
        label_.Redraw();
    }
};

} // namespace hui
