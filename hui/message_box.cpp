#include "message_box.hpp"
#include "window_manager.hpp"

namespace hui {

MessageBox::MessageBox( hui::WindowManager*   wm,
                        float                 x,
                        float                 y,
                        float                 w,
                        float                 h,
                        const std::string&    title,
                        const std::string&    message,
                        std::function<void()> on_close )
    : DialogBox(
          wm,
          x,
          y,
          w,
          h,
          [on_close]() {
              if ( on_close )
                  on_close();
          },
          title ),
      ok_( wm, dr4::Vec2f{ w - 90.0f, h - 34.0f }, dr4::Vec2f{ 70.0f, 24.0f }, "OK" ),
      on_close_( std::move( on_close ) )
{
    ok_.setParent( this );
    ok_.setOnClick( [this]() {
        if ( on_close_ )
            on_close_();
        wm_->popModal();
    } );

    text_.reset( wm->getWindow()->CreateText() );
    text_->SetFont( wm->getWindow()->GetDefaultFont() );
    text_->SetFontSize( 15 );
    text_->SetColor( { 230, 230, 230, 255 } );
    text_->SetText( message );
    text_->SetPos( { 12.0f, TopBarHeight + 10.0f } );
}

bool
MessageBox::propagateEventToChildren( const hui::Event& event )
{
    if ( event.apply( &ok_ ) )
        return true;
    return DialogBox::propagateEventToChildren( event );
}

void
MessageBox::RedrawMyTexture() const
{
    DialogBox::RedrawMyTexture();
    if ( text_ )
        text_->DrawOn( *texture_ );
    ok_.Redraw();
}

} // namespace hui
