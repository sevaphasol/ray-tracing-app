#include "objects_control_panel.hpp"
#include "hui/button.hpp"
#include "hui/window_manager.hpp"
#include "rta/model/rendering/scene_manager.hpp"
#include "rta/model/rendering/vector3.hpp"
#include "rta/view/panels/control/base_control_panel.hpp"

namespace rta {
namespace view {

rta::view::ObjectsControlPanel::ObjectsControlPanel( hui::WindowManager*       wm,
                                                     rta::model::SceneManager& scene_manager,
                                                     const Theme&              theme )
    : BaseControlPanel( wm,
                        theme.panel_pos,
                        theme.panel_size,
                        "Object Controls",
                        theme.button_size,
                        BtnsCount ),
      scene_manager_( scene_manager )
{
    const auto& thm     = theme;
    const auto& btn_thm = theme.button_theme;

    addHoldPressButton( thm.rot_yaw_left, "󰵗", btn_thm, [this, thm]() {
        rotateTarget( { 0.0f, 1.0f, 0.0f }, -thm.rotate_step );
    } );
    addHoldPressButton( thm.rot_yaw_right, "󰵘", btn_thm, [this, thm]() {
        rotateTarget( { 0.0f, 1.0f, 0.0f }, thm.rotate_step );
    } );
    addHoldPressButton( thm.rot_pitch_up, "󱃴", btn_thm, [this, thm]() {
        rotateTarget( { 1.0f, 0.0f, 0.0f }, thm.rotate_step );
    } );
    addHoldPressButton( thm.rot_pitch_down, "󱃳", btn_thm, [this, thm]() {
        rotateTarget( { 1.0f, 0.0f, 0.0f }, -thm.rotate_step );
    } );
    addHoldPressButton( thm.rot_roll_left, "", btn_thm, [this, thm]() {
        rotateTarget( { 0.0f, 0.0f, 1.0f }, -thm.rotate_step );
    } );
    addHoldPressButton( thm.rot_roll_right, "", btn_thm, [this, thm]() {
        rotateTarget( { 0.0f, 0.0f, 1.0f }, thm.rotate_step );
    } );
    addHoldPressButton( thm.mv_l_pos, "", btn_thm, [this, thm]() {
        moveTarget( { -thm.move_step, 0.0f, 0.0f } );
    } );
    addHoldPressButton( thm.mv_r_pos, "", btn_thm, [this, thm]() {
        moveTarget( { thm.move_step, 0.0f, 0.0f } );
    } );
    addHoldPressButton( thm.mv_u_pos, "", btn_thm, [this, thm]() {
        moveTarget( { 0.0f, thm.move_step, 0.0f } );
    } );
    addHoldPressButton( thm.mv_d_pos, "", btn_thm, [this, thm]() {
        moveTarget( { 0.0f, -thm.move_step, 0.0f } );
    } );
    addHoldPressButton( thm.mv_f_pos, "", btn_thm, [this, thm]() {
        moveTarget( { 0.0f, 0.0f, -thm.move_step } );
    } );
    addHoldPressButton( thm.mv_b_pos, "󰻂", btn_thm, [this, thm]() {
        moveTarget( { 0.0f, 0.0f, thm.move_step } );
    } );
}

void
rta::view::ObjectsControlPanel::moveTarget( const rta::model::Vector3f& delta )
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
rta::view::ObjectsControlPanel::rotateTarget( const rta::model::Vector3f& axis_world, float angle )
{
    auto* target = scene_manager_.getTargetObj();
    if ( target == nullptr )
    {
        return;
    }

    target->rotateAroundWorldAxis( axis_world, angle );
    scene_manager_.needUpdate() = true;
}

} // namespace view
} // namespace rta
