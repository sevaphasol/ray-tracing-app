#pragma once

#include "hui/button.hpp"
#include "hui/button_cancel.hpp"
#include "hui/button_ok.hpp"
#include "hui/dialog_box.hpp"
#include "hui/input_text.hpp"
#include <functional>
#include <memory>
#include <string>

namespace hui {

class FileDialogBox : public DialogBox {
  public:
    using ConfirmCb = std::function<void( const std::string& )>;
    using CancelCb  = std::function<void()>;

    FileDialogBox( hui::WindowManager* wm,
                   float               x,
                   float               y,
                   float               w,
                   float               h,
                   const std::string&  title,
                   const std::string&  default_name,
                   ConfirmCb           confirm,
                   CancelCb            cancel )
        : DialogBox(
              wm,
              x,
              y,
              w,
              h,
              [cancel]() { cancel(); },
              title ),
          confirm_( std::move( confirm ) ),
          cancel_( std::move( cancel ) ),
          input_( wm, 20.0f, TopBarHeight + 20.0f, w - 40.0f, 24.0f ),
          ok_btn_( wm, { 20.0f, h - 60.0f }, { 80.0f, 26.0f } ),
          cancel_btn_( wm, { 120.0f, h - 60.0f }, { 80.0f, 26.0f } )
    {
        input_.setParent( this );
        input_.setString( default_name );
        ok_btn_.setParent( this );
        cancel_btn_.setParent( this );

        ok_btn_.setOnClick( [this]() {
            auto name_sv = input_.getString();
            confirm_( std::string( name_sv.value_or( "" ) ) );
        } );
        cancel_btn_.setOnClick( [this]() { cancel_(); } );
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        if ( event.apply( &input_ ) )
        {
            return true;
        }
        if ( event.apply( &ok_btn_ ) )
        {
            return true;
        }
        if ( event.apply( &cancel_btn_ ) )
        {
            return true;
        }
        return DialogBox::propagateEventToChildren( event );
    }

    void
    RedrawMyTexture() const override
    {
        DialogBox::RedrawMyTexture();
        input_.Redraw();
        ok_btn_.Redraw();
        cancel_btn_.Redraw();
    }

  private:
    ConfirmCb         confirm_;
    CancelCb          cancel_;
    hui::InputText    input_;
    hui::ButtonOk     ok_btn_;
    hui::ButtonCancel cancel_btn_;
};

} // namespace hui
