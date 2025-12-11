#pragma once

#include "button.hpp"
#include "label.hpp"
#include "widget.hpp"
#include "window_manager.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include <cstdlib>
#include <memory>
#include <string>

namespace hui {

// Generic dialog panel with a top bar, title and close button.
class DialogBox : public hui::ContainerWidget {
  public:
    struct Theme
    {
        dr4::Color      rect_fill;
        dr4::Color      rect_border;
        float           rect_border_thickness;
        dr4::Color      topbar_fill;
        dr4::Color      topbar_border;
        float           topbar_border_thickness;
        dr4::Color      label_color;
        float           label_font_size;
        dr4::Vec2f      label_padding;
        hui::Button::Theme close_button;
    };

    static const inline Theme DefaultTheme = {
        /*rect_fill=*/{ 32, 32, 32, 128 },
        /*rect_border=*/{ 118, 185, 0, 255 },
        /*rect_border_thickness=*/-2.0f,
        /*topbar_fill=*/{ 32, 32, 32, 128 },
        /*topbar_border=*/{ 118, 185, 0, 255 },
        /*topbar_border_thickness=*/-2.0f,
        /*label_color=*/{ 255, 255, 255, 255 },
        /*label_font_size=*/12.0f,
        /*label_padding=*/{ std::abs( -2.0f ) + 2.0f, std::abs( -2.0f ) + 2.0f },
        /*close_button=*/
        hui::Button::Theme{ { 128, 0, 0, 255 },
                            { 164, 0, 0, 255 },
                            { 96, 0, 0, 255 },
                            { 255, 255, 255, 255 },
                            12 }
    };

  private:
    std::unique_ptr<dr4::Rectangle> rect_;
    std::unique_ptr<dr4::Rectangle> top_bar_;
    hui::LabelWidget                label_;

  protected:
    hui::Button close_btn_;
    using CloseBtnCallBack = std::function<void()>;

    static constexpr float         TopBarHeight          = 25.0f;
    static constexpr float         TopBarBorderThickness = -2.0f;
    static constexpr float         CloseBtnSize = TopBarHeight - std::abs( TopBarBorderThickness );

  public:
    DialogBox( hui::WindowManager* wm,
               float               x,
               float               y,
               float               w,
               float               h,
               CloseBtnCallBack    close_call_back,
               const std::string&  label,
               const Theme&        theme = DefaultTheme )
        : hui::ContainerWidget( wm, x, y, w, h ),
          close_btn_( wm,
                      {
                          w - CloseBtnSize - std::abs( TopBarBorderThickness ),
                          std::abs( TopBarBorderThickness ),
                      },
                      { CloseBtnSize, CloseBtnSize },
                      "X",
                      theme.close_button ),
          label_( wm, theme.label_padding, { w - CloseBtnSize, CloseBtnSize }, label ),
          theme_( theme )
    {
        setDraggable( true );

        rect_.reset( wm->getWindow()->CreateRectangle() );
        rect_->SetSize( { w, h - TopBarHeight } );
        rect_->SetPos( { 0, TopBarHeight } );
        rect_->SetFillColor( theme_.rect_fill );
        rect_->SetBorderColor( theme_.rect_border );
        rect_->SetBorderThickness( theme_.rect_border_thickness );

        top_bar_.reset( wm->getWindow()->CreateRectangle() );
        top_bar_->SetSize( { w, TopBarHeight + std::abs( TopBarBorderThickness ) } );
        top_bar_->SetPos( { 0, 0 } );
        top_bar_->SetFillColor( theme_.topbar_fill );
        top_bar_->SetBorderColor( theme_.topbar_border );
        top_bar_->SetBorderThickness( theme_.topbar_border_thickness );

        close_btn_.setParent( this );
        close_btn_.setOnClick( close_call_back );

        label_.setParent( this );
        label_.setText( label );
        label_.setRelPos( theme_.label_padding.x, theme_.label_padding.y );
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

  private:
    Theme theme_;
};

} // namespace hui
