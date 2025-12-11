#pragma once

#include "hui/button.hpp"
#include "hui/window_manager.hpp"
#include "rta/model/rendering/scene_manager.hpp"
#include "rta/view/control_panels/base_control_panel.hpp"

namespace rta {
namespace view {

class CameraControlPanel : public BaseControlPanel {
  public:
    struct Theme
    {
        dr4::Vec2f         panel_pos    = { 50.0f, 50.0f };
        dr4::Vec2f         panel_size   = { 380.0f, 320.0f };
        dr4::Vec2f         button_size  = { 100.0f, 50.0f };
        hui::Button::Theme button_theme = { { 30, 30, 30, 255 },
                                            { 50, 70, 30, 255 },
                                            { 100, 150, 0, 255 },
                                            { 220, 220, 220, 255 },
                                            15 };

        float mv_step = 0.1f;
        float rt_step = 0.03f;
        float sc_step = 0.03f;

        dr4::Vec2f mv_l_pos = { 30.0f, 50.0f };
        dr4::Vec2f mv_r_pos = { 30.0f, 110.0f };
        dr4::Vec2f mv_u_pos = { 140.0f, 50.0f };
        dr4::Vec2f mv_d_pos = { 140.0f, 110.0f };
        dr4::Vec2f mv_f_pos = { 250.0f, 50.0f };
        dr4::Vec2f mv_b_pos = { 250.0f, 110.0f };
        dr4::Vec2f rt_l_pos = { 30.0f, 200.0f };
        dr4::Vec2f rt_r_pos = { 30.0f, 260.0f };
        dr4::Vec2f rt_u_pos = { 140.0f, 200.0f };
        dr4::Vec2f rt_d_pos = { 140.0f, 260.0f };
        dr4::Vec2f sc_u_pos = { 250.0f, 200.0f };
        dr4::Vec2f sc_d_pos = { 250.0f, 260.0f };

        static Theme
        Default()
        {
            return Theme();
        }
    };

    static constexpr size_t BtnsCount = 12;

  private:
    rta::model::SceneManager& scene_manager_;

  public:
    explicit CameraControlPanel( hui::WindowManager*       wm,
                                 rta::model::SceneManager& scene_manager,
                                 const Theme&              theme = Theme::Default() )
        : BaseControlPanel( wm,
                            theme.panel_pos,
                            theme.panel_size,
                            "Camera Controls",
                            theme.button_size,
                            BtnsCount ),
          scene_manager_( scene_manager )
    {
        const auto& thm     = theme;
        const auto& btn_thm = theme.button_theme;

        addHoldPressButton( thm.mv_l_pos, "Move left", btn_thm, [this, thm]() {
            moveCamera( { -thm.mv_step, 0.0f, 0.0f } );
        } );
        addHoldPressButton( thm.mv_r_pos, "Move right", btn_thm, [this, thm]() {
            moveCamera( { thm.mv_step, 0.0f, 0.0f } );
        } );
        addHoldPressButton( thm.mv_u_pos, "Move up", btn_thm, [this, thm]() {
            moveCamera( { 0.0f, thm.mv_step, 0.0f } );
        } );
        addHoldPressButton( thm.mv_d_pos, "Move down", btn_thm, [this, thm]() {
            moveCamera( { 0.0f, -thm.mv_step, 0.0f } );
        } );
        addHoldPressButton( thm.mv_f_pos, "Move forward", btn_thm, [this, thm]() {
            moveCamera( { 0.0f, 0.0f, -thm.mv_step } );
        } );
        addHoldPressButton( thm.mv_b_pos, "Move Backward", btn_thm, [this, thm]() {
            moveCamera( { 0.0f, 0.0f, thm.mv_step } );
        } );
        addHoldPressButton( thm.rt_l_pos, "Rotate left", btn_thm, [this, thm]() {
            rotateCamera( { thm.rt_step, 0.0f } );
        } );
        addHoldPressButton( thm.rt_r_pos, "Rotate right", btn_thm, [this, thm]() {
            rotateCamera( { -thm.rt_step, 0.0f } );
        } );
        addHoldPressButton( thm.rt_u_pos, "Rotate up", btn_thm, [this, thm]() {
            rotateCamera( { 0.0f, -thm.rt_step } );
        } );
        addHoldPressButton( thm.rt_d_pos, "Rotate down", btn_thm, [this, thm]() {
            rotateCamera( { 0.0f, thm.rt_step } );
        } );
        addHoldPressButton( thm.sc_u_pos, "Scale up", btn_thm, [this, thm]() {
            scaleCamera( -thm.sc_step );
        } );
        addHoldPressButton( thm.sc_d_pos, "Scale down", btn_thm, [this, thm]() {
            scaleCamera( thm.sc_step );
        } );
    }

  private:
    void
    moveCamera( const rta::model::Vector3f& delta )
    {
        scene_manager_.getCamera().move( delta );
        scene_manager_.needUpdate() = true;
    }

    void
    rotateCamera( const rta::model::Vector2f& delta )
    {
        scene_manager_.getCamera().rotate( delta );
        scene_manager_.needUpdate() = true;
    }

    void
    scaleCamera( float factor )
    {
        scene_manager_.getCamera().scale( factor );
        scene_manager_.needUpdate() = true;
    }
};

} // namespace view
} // namespace rta
