#include "label.hpp"
#include "window_manager.hpp"

namespace hui {

LabelWidget::LabelWidget( hui::WindowManager* wm,
                          const dr4::Vec2f&   pos,
                          const dr4::Vec2f&   size,
                          const std::string&  text,
                          unsigned int        font_size )
    : Widget( wm, pos.x, pos.y, size.x, size.y ), text_( text ), font_size_( font_size )
{
    auto* win = wm_->getWindow();
    text_obj_.reset( win->CreateText() );
    text_obj_->SetFont( win->GetDefaultFont() );
    text_obj_->SetFontSize( static_cast<int>( font_size_ ) );
    text_obj_->SetText( text_ );
    text_obj_->SetColor( { 200, 200, 200, 255 } );
}

void
LabelWidget::setText( const std::string& text )
{
    text_ = text;
    if ( text_obj_ )
    {
        text_obj_->SetText( text_ );
    }
}

void
LabelWidget::RedrawMyTexture() const
{
    if ( !text_obj_ )
        return;

    // небольшой внутренний отступ слева/сверху
    const float pad_x = 4.0f;
    const float pad_y = 2.0f;

    // Размещаем текст в локальной системе координат текстуры (texture_),
    // поэтому pos задаём относительно 0..size_
    text_obj_->SetPos( { pad_x, pad_y } );

    // Если текст длиннее доступной ширины — text_obj_ сам ограничит bounds/GetBounds(), но
    // можно также обрезать строку заранее. Пока пусть текст рисуется как есть.
    text_obj_->DrawOn( *texture_ );
}

} // namespace hui
