#pragma once

#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/container_widget.hpp"
#include "custom-hui-impl/scrollbar.hpp"
#include <memory>

namespace zemax {
namespace view {

class ScrollableButtonsWidget : public hui::ContainerWidget {
  public:
    explicit ScrollableButtonsWidget( cum::Manager*     pm,
                                      dr4::Window*      win,
                                      const dr4::Vec2f& pos  = { 0.0f, 0.0f },
                                      const dr4::Vec2f& size = { 0.0f, 0.0f } )
        : ContainerWidget( pm, win, pos, size ), scroll_bar_( pm, win, 0, 0 )
    {
        // setPosition( Config::ControlPanel::Size );
        // setSize( Config::ControlPanel::Size );

        border_.reset( win->CreateRectangle() );

        border_->SetSize( size );
        border_->SetFillColor( { 0, 0, 0, 0 } );
        border_->SetBorderColor( { 118, 185, 0, 255 } );
        border_->SetBorderThickness( -2.f );

        scroll_bar_.setParent( this );
    };

    explicit ScrollableButtonsWidget( cum::Manager* pm,
                                      dr4::Window*  win,
                                      float         x,
                                      float         y,
                                      float         w,
                                      float         h )
        : ScrollableButtonsWidget( pm, win, dr4::Vec2f( x, y ), dr4::Vec2f( w, h ) )
    {
    }

    virtual ~ScrollableButtonsWidget() = default;

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        if ( event.apply( &scroll_bar_ ) )
        {
            return true;
        }

        if ( !buttons_.empty() )
        {
            if ( event.apply( buttons_[cur_active_button_].get() ) )
            {
                return true;
            }
        }

        return false;
    }

    bool
    onIdle( const hui::Event& event ) override
    {
        // std::cerr << "prev_active_button_ " << prev_active_button_ << std::endl;
        // std::cerr << "cur_active_button_  " << cur_active_button_ << std::endl;

        if ( scroll_bar_.isScrolled() )
        {
            float scroll_factor = scroll_bar_.getScrollFactor();

            int tmp_idx = ( buttons_.size() - 1 ) * scroll_factor;
            std::cerr << tmp_idx << std::endl;

            if ( tmp_idx != cur_active_button_ )
            {
                prev_active_button_ = cur_active_button_;
            }

            cur_active_button_ = tmp_idx;
        }

        is_scroled_ = true;

        return propagateEventToChildren( event );
    }

    bool
    isActiveButtonPressed()
    {
        if ( !buttons_.empty() )
        {
            return buttons_[cur_active_button_]->isPressed();
        }

        return false;
    }

    bool
    isScrolled() const
    {
        return is_scroled_;
    }

    size_t
    getCurActiveButton() const
    {
        return cur_active_button_;
    }

    size_t
    getPrevActiveButton() const
    {
        return prev_active_button_;
    }

    // std::vector<std::unique_ptr<Button>>&
    // getButtons()
    // {
    // return buttons_;
    // }

    void
    addButton( std::unique_ptr<hui::Button> button )
    {
        button->setParent( this );
        buttons_.push_back( std::move( button ) );
    }

  private:
    virtual void
    RedrawMyTexture() const override final
    {
        // dr4::Transform widget_transform = transform.combine( getTransform() );

        scroll_bar_.Redraw();

        // window.draw( scroll_bar_, widget_transform );
        if ( !buttons_.empty() )
        {
            buttons_[cur_active_button_]->Redraw();
        }

        // texture_->Draw(bringToFront(border_))

        texture_->Draw( *border_ );

        // window.draw( border_, widget_transform );
    }

  private:
    hui::ScrollBar scroll_bar_;

    std::unique_ptr<dr4::Rectangle> border_;

    int cur_active_button_  = 0;
    int prev_active_button_ = -1;

    bool is_scroled_ = false;

    std::vector<std::unique_ptr<hui::Button>> buttons_;
};

} // namespace view
} // namespace zemax
