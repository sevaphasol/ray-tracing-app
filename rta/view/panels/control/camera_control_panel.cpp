#include "camera_control_panel.hpp"
#include "hui/button.hpp"
#include "hui/window_manager.hpp"
#include "rta/model/rendering/scene_manager.hpp"
#include "rta/view/panels/control/base_control_panel.hpp"

namespace rta {
namespace view {

rta::view::CameraControlPanel::CameraControlPanel( hui::WindowManager*       wm,
                                                   rta::model::SceneManager& scene_manager,
                                                   const Theme&              theme )
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

    addHoldPressButton( thm.mv_l_pos, "", btn_thm, [this, thm]() {
        moveCamera( { -thm.mv_step, 0.0f, 0.0f } );
    } );
    addHoldPressButton( thm.mv_r_pos, "", btn_thm, [this, thm]() {
        moveCamera( { thm.mv_step, 0.0f, 0.0f } );
    } );
    addHoldPressButton( thm.mv_u_pos, "", btn_thm, [this, thm]() {
        moveCamera( { 0.0f, thm.mv_step, 0.0f } );
    } );
    addHoldPressButton( thm.mv_d_pos, "", btn_thm, [this, thm]() {
        moveCamera( { 0.0f, -thm.mv_step, 0.0f } );
    } );
    addHoldPressButton( thm.mv_f_pos, "", btn_thm, [this, thm]() {
        moveCamera( { 0.0f, 0.0f, -thm.mv_step } );
    } );
    addHoldPressButton( thm.mv_b_pos, "󰻂", btn_thm, [this, thm]() {
        moveCamera( { 0.0f, 0.0f, thm.mv_step } );
    } );
    addHoldPressButton( thm.rt_l_pos, "󰵘", btn_thm, [this, thm]() {
        rotateCamera( { thm.rt_step, 0.0f } );
    } );
    addHoldPressButton( thm.rt_r_pos, "󰵗", btn_thm, [this, thm]() {
        rotateCamera( { -thm.rt_step, 0.0f } );
    } );
    addHoldPressButton( thm.rt_u_pos, "󱃴", btn_thm, [this, thm]() {
        rotateCamera( { 0.0f, -thm.rt_step } );
    } );
    addHoldPressButton( thm.rt_d_pos, "󱃳", btn_thm, [this, thm]() {
        rotateCamera( { 0.0f, thm.rt_step } );
    } );
    addHoldPressButton( thm.sc_u_pos, "", btn_thm, [this, thm]() {
        scaleCamera( -thm.sc_step );
    } );
    addHoldPressButton( thm.sc_d_pos, "", btn_thm, [this, thm]() {
        scaleCamera( thm.sc_step );
    } );
}

void
rta::view::CameraControlPanel::moveCamera( const rta::model::Vector3f& delta )
{
    scene_manager_.getCamera().move( delta );
    scene_manager_.needUpdate() = true;
}

void
rta::view::CameraControlPanel::rotateCamera( const rta::model::Vector2f& delta )
{
    scene_manager_.getCamera().rotate( delta );
    scene_manager_.needUpdate() = true;
}

void
rta::view::CameraControlPanel::scaleCamera( float factor )
{
    scene_manager_.getCamera().scale( factor );
    scene_manager_.needUpdate() = true;
}

} // namespace view
} // namespace rta
