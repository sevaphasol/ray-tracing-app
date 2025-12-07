#pragma once

#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/container_widget.hpp"
#include "custom-hui-impl/window_manager.hpp"
#include "zemax/config.hpp"
#include "zemax/model/rendering/scene_manager.hpp"
#include <memory>

namespace zemax {
namespace view {

class CameraControlPanel : public hui::ContainerWidget {
  public:
    explicit CameraControlPanel( hui::WindowManager*         wm,
                                 zemax::model::SceneManager& scene_manager,
                                 const dr4::Vec2f&           pos  = Config::CameraPanel::Position,
                                 const dr4::Vec2f&           size = Config::CameraPanel::Size )
        : hui::ContainerWidget( wm, pos, size ), scene_manager_( scene_manager )
    {
        border_.reset( wm->getWindow()->CreateRectangle() );
        border_->SetSize( getSize() );
        border_->SetFillColor( Config::CameraPanel::BackgroundColor );
        border_->SetBorderColor( Config::CameraPanel::BorderColor );
        border_->SetBorderThickness( -Config::CameraPanel::BorderThickness );

        setupButton( MoveLeft,
                     Config::CameraPanel::Button::MvL::Position,
                     Config::CameraPanel::Button::MvL::Title );
        setupButton( MoveRight,
                     Config::CameraPanel::Button::MvR::Position,
                     Config::CameraPanel::Button::MvR::Title );
        setupButton( MoveUp,
                     Config::CameraPanel::Button::MvU::Position,
                     Config::CameraPanel::Button::MvU::Title );
        setupButton( MoveDown,
                     Config::CameraPanel::Button::MvD::Position,
                     Config::CameraPanel::Button::MvD::Title );
        setupButton( MoveForward,
                     Config::CameraPanel::Button::MvF::Position,
                     Config::CameraPanel::Button::MvF::Title );
        setupButton( MoveBackward,
                     Config::CameraPanel::Button::MvB::Position,
                     Config::CameraPanel::Button::MvB::Title );
        setupButton( RotateLeft,
                     Config::CameraPanel::Button::RtL::Position,
                     Config::CameraPanel::Button::RtL::Title );
        setupButton( RotateRight,
                     Config::CameraPanel::Button::RtR::Position,
                     Config::CameraPanel::Button::RtR::Title );
        setupButton( RotateUp,
                     Config::CameraPanel::Button::RtU::Position,
                     Config::CameraPanel::Button::RtU::Title );
        setupButton( RotateDown,
                     Config::CameraPanel::Button::RtD::Position,
                     Config::CameraPanel::Button::RtD::Title );
        setupButton( ScaleUp,
                     Config::CameraPanel::Button::ScaleUp::Position,
                     Config::CameraPanel::Button::ScaleUp::Title );
        setupButton( ScaleDown,
                     Config::CameraPanel::Button::ScaleDown::Position,
                     Config::CameraPanel::Button::ScaleDown::Title );
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        for ( const auto& button : buttons_ )
        {
            if ( event.apply( button.get() ) )
            {
                return true;
            }
        }

        return false;
    }

    bool
    onIdle( const hui::Event& event ) override final
    {
        if ( isPressed( MoveLeft ) )
        {
            scene_manager_.getCamera().move( { -Config::Camera::MoveFactor, 0.0f, 0.0f } );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( MoveRight ) )
        {
            scene_manager_.getCamera().move( { Config::Camera::MoveFactor, 0.0f, 0.0f } );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( MoveUp ) )
        {
            scene_manager_.getCamera().move( { 0.0f, Config::Camera::MoveFactor, 0.0f } );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( MoveDown ) )
        {
            scene_manager_.getCamera().move( { 0.0f, -Config::Camera::MoveFactor, 0.0f } );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( MoveForward ) )
        {
            scene_manager_.getCamera().move( { 0.0f, 0.0f, -Config::Camera::MoveFactor } );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( MoveBackward ) )
        {
            scene_manager_.getCamera().move( { 0.0f, 0.0f, Config::Camera::MoveFactor } );
            scene_manager_.needUpdate() = true;
        }

        if ( isPressed( RotateLeft ) )
        {
            scene_manager_.getCamera().rotate( { Config::Camera::RotateFactor, 0.0f } );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( RotateRight ) )
        {
            scene_manager_.getCamera().rotate( { -Config::Camera::RotateFactor, 0.0f } );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( RotateUp ) )
        {
            scene_manager_.getCamera().rotate( { 0.0f, -Config::Camera::RotateFactor } );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( RotateDown ) )
        {
            scene_manager_.getCamera().rotate( { 0.0f, Config::Camera::RotateFactor } );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( ScaleUp ) )
        {
            scene_manager_.getCamera().scale( -Config::Camera::ScaleFactor );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( ScaleDown ) )
        {
            scene_manager_.getCamera().scale( Config::Camera::ScaleFactor );
            scene_manager_.needUpdate() = true;
        }

        propagateEventToChildren( event );
        return false;
    }

    void
    RedrawMyTexture() const override final
    {
        texture_->Draw( *border_ );
        for ( const auto& button : buttons_ )
        {
            button->Redraw();
        }
    }

  private:
    enum ButtonCode {
        MoveLeft,
        MoveRight,
        MoveUp,
        MoveDown,
        MoveForward,
        MoveBackward,
        RotateLeft,
        RotateRight,
        RotateUp,
        RotateDown,
        ScaleUp,
        ScaleDown,
        ButtonCount,
    };

    std::unique_ptr<hui::Button> buttons_[ButtonCount];

    bool
    isPressed( ButtonCode code )
    {
        return dynamic_cast<hui::Button*>( buttons_[code].get() )->isPressed();
    }

    void
    setupButton( ButtonCode code, const dr4::Vec2f& pos, const char* title )
    {
        buttons_[code] =
            std::move( std::make_unique<hui::Button>( wm_,
                                                      pos,
                                                      Config::CameraPanel::Button::Size,
                                                      Config::CameraPanel::Button::DefaultColor,
                                                      Config::CameraPanel::Button::HoveredColor,
                                                      Config::CameraPanel::Button::PressedColor,
                                                      title,
                                                      Config::CameraPanel::Button::FontColor,
                                                      Config::CameraPanel::Button::FontSize ) );
        buttons_[code]->setParent( this );
    }

  private:
    zemax::model::SceneManager&      scene_manager_;
    std::unique_ptr<dr4::Rectangle>  border_;
};

} // namespace view
} // namespace zemax
