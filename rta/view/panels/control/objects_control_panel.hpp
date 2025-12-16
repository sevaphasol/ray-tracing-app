#pragma once

#include "hui/button.hpp"
#include "hui/window_manager.hpp"
#include "rta/model/rendering/scene_manager.hpp"
#include "rta/model/rendering/vector3.hpp"
#include "rta/view/panels/control/base_control_panel.hpp"

namespace rta {
namespace view {

class ObjectsControlPanel : public BaseControlPanel {
  public:
    struct Theme
    {

        dr4::Vec2f         panel_pos    = { 50.0f, 465.0f };
        dr4::Vec2f         panel_size   = { 295.0f, 385.0f };
        dr4::Vec2f         button_size  = { 70.0f, 70.0f };
        hui::Button::Theme button_theme = { { 30, 30, 30, 255 },
                                            { 50, 70, 30, 255 },
                                            { 100, 150, 0, 255 },
                                            { 220, 220, 220, 255 },
                                            35 };

        float move_step   = 0.1f;
        float rotate_step = 0.03f;

        dr4::Vec2f mv_l_pos       = { 30.0f, 50.0f };
        dr4::Vec2f mv_r_pos       = { 30.0f, 130.0f };
        dr4::Vec2f mv_u_pos       = { 110.0f, 50.0f };
        dr4::Vec2f mv_d_pos       = { 110.0f, 130.0f };
        dr4::Vec2f mv_f_pos       = { 190.0f, 50.0f };
        dr4::Vec2f mv_b_pos       = { 190.0f, 130.0f };
        dr4::Vec2f rot_yaw_right  = { 30.0f, 210.0f };
        dr4::Vec2f rot_yaw_left   = { 30.0f, 290.0f };
        dr4::Vec2f rot_pitch_down = { 110.0f, 210.0f };
        dr4::Vec2f rot_pitch_up   = { 110.0f, 290.0f };
        dr4::Vec2f rot_roll_left  = { 190.0f, 210.0f };
        dr4::Vec2f rot_roll_right = { 190.0f, 290.0f };

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
                                  const Theme&              theme = Theme::Default() );

  private:
    void
    moveTarget( const rta::model::Vector3f& delta );
    void
    rotateTarget( const rta::model::Vector3f& axis_world, float angle );
};

} // namespace view
} // namespace rta
