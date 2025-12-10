#pragma once

#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/closable_panel.hpp"
#include "custom-hui-impl/window_manager.hpp"
#include "zemax/model/rendering/scene_manager.hpp"
#include <memory>

namespace zemax {
namespace view {

class CameraControlPanel : public hui::ClosablePanel {
  public:
    struct Theme
    {
        dr4::Vec2f         panel_pos{ 50.0f, 50.0f };
        dr4::Vec2f         panel_size{ 380.0f, 320.0f };
        dr4::Vec2f         button_size{ 100.0f, 50.0f };
        hui::Button::Theme button_theme{ { 30, 30, 30, 255 },
                                         { 50, 70, 30, 255 },
                                         { 100, 150, 0, 255 },
                                         { 220, 220, 220, 255 },
                                         15 };
        float move_step   = 0.1f;
        float rotate_step = 0.03f;
        float scale_step  = 0.03f;

        dr4::Vec2f mv_l{ 30.0f, 50.0f };
        dr4::Vec2f mv_r{ 30.0f, 110.0f };
        dr4::Vec2f mv_u{ 140.0f, 50.0f };
        dr4::Vec2f mv_d{ 140.0f, 110.0f };
        dr4::Vec2f mv_f{ 250.0f, 50.0f };
        dr4::Vec2f mv_b{ 250.0f, 110.0f };
        dr4::Vec2f rt_l{ 30.0f, 200.0f };
        dr4::Vec2f rt_r{ 30.0f, 260.0f };
        dr4::Vec2f rt_u{ 140.0f, 200.0f };
        dr4::Vec2f rt_d{ 140.0f, 260.0f };
        dr4::Vec2f sc_up{ 250.0f, 200.0f };
        dr4::Vec2f sc_down{ 250.0f, 260.0f };

        static Theme Default() { return Theme(); }
    };

    explicit CameraControlPanel( hui::WindowManager*         wm,
                                 zemax::model::SceneManager& scene_manager,
                                 const Theme&                theme = Theme::Default() )
        : hui::ClosablePanel(
              wm, theme.panel_pos.x, theme.panel_pos.y, theme.panel_size.x, theme.panel_size.y, "Camera Controls" ),
          scene_manager_( scene_manager ),
          theme_( theme )
    {
        auto& t = theme_.button_theme;
        setupButton( MoveLeft, theme_.mv_l, "Move left", t );
        setupButton( MoveRight, theme_.mv_r, "Move right", t );
        setupButton( MoveUp, theme_.mv_u, "Move up", t );
        setupButton( MoveDown, theme_.mv_d, "Move down", t );
        setupButton( MoveForward, theme_.mv_f, "Move forward", t );
        setupButton( MoveBackward, theme_.mv_b, "Move Backward", t );
        setupButton( RotateLeft, theme_.rt_l, "Rotate left", t );
        setupButton( RotateRight, theme_.rt_r, "Rotate right", t );
        setupButton( RotateUp, theme_.rt_u, "Rotate up", t );
        setupButton( RotateDown, theme_.rt_d, "Rotate down", t );
        setupButton( ScaleUp, theme_.sc_up, "Scale up", t );
        setupButton( ScaleDown, theme_.sc_down, "Scale down", t );

        buttons_[MoveLeft]->setOnHoldPress( [this]() {
            scene_manager_.getCamera().move( { -theme_.move_step, 0.0f, 0.0f } );
            scene_manager_.needUpdate() = true;
        } );
        buttons_[MoveRight]->setOnHoldPress( [this]() {
            scene_manager_.getCamera().move( { theme_.move_step, 0.0f, 0.0f } );
            scene_manager_.needUpdate() = true;
        } );
        buttons_[MoveUp]->setOnHoldPress( [this]() {
            scene_manager_.getCamera().move( { 0.0f, theme_.move_step, 0.0f } );
            scene_manager_.needUpdate() = true;
        } );
        buttons_[MoveDown]->setOnHoldPress( [this]() {
            scene_manager_.getCamera().move( { 0.0f, -theme_.move_step, 0.0f } );
            scene_manager_.needUpdate() = true;
        } );
        buttons_[MoveForward]->setOnHoldPress( [this]() {
            scene_manager_.getCamera().move( { 0.0f, 0.0f, -theme_.move_step } );
            scene_manager_.needUpdate() = true;
        } );
        buttons_[MoveBackward]->setOnHoldPress( [this]() {
            scene_manager_.getCamera().move( { 0.0f, 0.0f, theme_.move_step } );
            scene_manager_.needUpdate() = true;
        } );

        buttons_[RotateLeft]->setOnHoldPress( [this]() {
            scene_manager_.getCamera().rotate( { theme_.rotate_step, 0.0f } );
            scene_manager_.needUpdate() = true;
        } );
        buttons_[RotateRight]->setOnHoldPress( [this]() {
            scene_manager_.getCamera().rotate( { -theme_.rotate_step, 0.0f } );
            scene_manager_.needUpdate() = true;
        } );
        buttons_[RotateUp]->setOnHoldPress( [this]() {
            scene_manager_.getCamera().rotate( { 0.0f, -theme_.rotate_step } );
            scene_manager_.needUpdate() = true;
        } );
        buttons_[RotateDown]->setOnHoldPress( [this]() {
            scene_manager_.getCamera().rotate( { 0.0f, theme_.rotate_step } );
            scene_manager_.needUpdate() = true;
        } );

        buttons_[ScaleUp]->setOnHoldPress( [this]() {
            scene_manager_.getCamera().scale( -theme_.scale_step );
            scene_manager_.needUpdate() = true;
        } );
        buttons_[ScaleDown]->setOnHoldPress( [this]() {
            scene_manager_.getCamera().scale( theme_.scale_step );
            scene_manager_.needUpdate() = true;
        } );
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        if ( !visible_ )
        {
            return false;
        }

        for ( const auto& button : buttons_ )
        {
            if ( event.apply( button.get() ) )
            {
                return true;
            }
        }

        return hui::DialogBox::propagateEventToChildren( event );
    }

    bool
    onIdle( const hui::Event& event ) override final
    {
        if ( !visible_ )
        {
            return false;
        }

        propagateEventToChildren( event );
        return false;
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
    Theme                        theme_;

    void
    setupButton( ButtonCode code, const dr4::Vec2f& pos, const char* title, const hui::Button::Theme& theme )
    {
        buttons_[code] = std::make_unique<hui::Button>( wm_, pos, theme_.button_size, title, theme );
        buttons_[code]->setParent( this );
    }

    void
    RedrawMyTexture() const override
    {
        hui::DialogBox::RedrawMyTexture();

        for ( const auto& btn : buttons_ )
        {
            btn->Redraw();
        }
    }

  private:
    zemax::model::SceneManager& scene_manager_;
};

} // namespace view
} // namespace zemax
