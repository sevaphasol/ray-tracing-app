#pragma once

#include "hui/button.hpp"
#include "hui/window_manager.hpp"
#include "rta/model/rendering/scene_manager.hpp"
#include "rta/model/rendering/vector3.hpp"
#include "rta/view/control_panels/base_control_panel.hpp"

namespace rta {
namespace view {

class ObjectsControlPanel : public BaseControlPanel {
  public:
    struct Theme
    {
        dr4::Vec2f         panel_pos    = { 50.0f, 465.0f };
        dr4::Vec2f         panel_size   = { 380.0f, 385.0f };
        dr4::Vec2f         button_size  = { 100.0f, 50.0f };
        hui::Button::Theme button_theme = { { 30, 30, 30, 255 },
                                            { 50, 70, 30, 255 },
                                            { 100, 150, 0, 255 },
                                            { 220, 220, 220, 255 },
                                            15 };

        float move_step   = 0.1f;
        float rotate_step = 0.03f;

        dr4::Vec2f rot_yaw_left   = { 30.0f, 80.0f };
        dr4::Vec2f rot_yaw_right  = { 30.0f, 140.0f };
        dr4::Vec2f rot_pitch_up   = { 140.0f, 80.0f };
        dr4::Vec2f rot_pitch_down = { 140.0f, 140.0f };
        dr4::Vec2f rot_roll_left  = { 250.0f, 80.0f };
        dr4::Vec2f rot_roll_right = { 250.0f, 140.0f };

        dr4::Vec2f mv_l_pos = { 30.0f, 220.0f };
        dr4::Vec2f mv_r_pos = { 30.0f, 280.0f };
        dr4::Vec2f mv_u_pos = { 140.0f, 220.0f };
        dr4::Vec2f mv_d_pos = { 140.0f, 280.0f };
        dr4::Vec2f mv_f_pos = { 250.0f, 220.0f };
        dr4::Vec2f mv_b_pos = { 250.0f, 280.0f };

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
    explicit ObjectsControlPanel( hui::WindowManager*       wm,
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

        addHoldPressButton( thm.rot_yaw_left, "Y-", btn_thm, [this, thm]() {
            rotateTarget( { 0.0f, 1.0f, 0.0f }, -thm.rotate_step );
        } );
        addHoldPressButton( thm.rot_yaw_right, "Y+", btn_thm, [this, thm]() {
            rotateTarget( { 0.0f, 1.0f, 0.0f }, thm.rotate_step );
        } );
        addHoldPressButton( thm.rot_pitch_up, "X-", btn_thm, [this, thm]() {
            rotateTarget( { 1.0f, 0.0f, 0.0f }, thm.rotate_step );
        } );
        addHoldPressButton( thm.rot_pitch_down, "X+", btn_thm, [this, thm]() {
            rotateTarget( { 1.0f, 0.0f, 0.0f }, -thm.rotate_step );
        } );
        addHoldPressButton( thm.rot_roll_left, "Z-", btn_thm, [this, thm]() {
            rotateTarget( { 0.0f, 0.0f, 1.0f }, -thm.rotate_step );
        } );
        addHoldPressButton( thm.rot_roll_right, "Z+", btn_thm, [this, thm]() {
            rotateTarget( { 0.0f, 0.0f, 1.0f }, thm.rotate_step );
        } );
        addHoldPressButton( thm.mv_l_pos, "Move left", btn_thm, [this, thm]() {
            moveTarget( { -thm.move_step, 0.0f, 0.0f } );
        } );
        addHoldPressButton( thm.mv_r_pos, "Move right", btn_thm, [this, thm]() {
            moveTarget( { thm.move_step, 0.0f, 0.0f } );
        } );
        addHoldPressButton( thm.mv_u_pos, "Move up", btn_thm, [this, thm]() {
            moveTarget( { 0.0f, thm.move_step, 0.0f } );
        } );
        addHoldPressButton( thm.mv_d_pos, "Move down", btn_thm, [this, thm]() {
            moveTarget( { 0.0f, -thm.move_step, 0.0f } );
        } );
        addHoldPressButton( thm.mv_f_pos, "Move fwd", btn_thm, [this, thm]() {
            moveTarget( { 0.0f, 0.0f, -thm.move_step } );
        } );
        addHoldPressButton( thm.mv_b_pos, "Move bwd", btn_thm, [this, thm]() {
            moveTarget( { 0.0f, 0.0f, thm.move_step } );
        } );
    }

  private:
    void
    moveTarget( const rta::model::Vector3f& delta )
    {
        auto* target = scene_manager_.getTargetObj();
        if ( target == nullptr )
        {
            return;
        }

        target->move( delta );
        scene_manager_.needUpdate() = true;
    }

    void
    rotateTarget( const rta::model::Vector3f& axis_world, float angle )
    {
        auto* target = scene_manager_.getTargetObj();
        if ( target == nullptr )
        {
            return;
        }

        target->rotateAroundWorldAxis( axis_world, angle );
        scene_manager_.needUpdate() = true;
    }
};

} // namespace view
} // namespace rta
