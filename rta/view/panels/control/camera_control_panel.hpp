#pragma once

#include "hui/button.hpp"
#include "hui/window_manager.hpp"
#include "rta/model/rendering/scene_manager.hpp"
#include "rta/view/panels/control/base_control_panel.hpp"

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
                                 const Theme&              theme = Theme::Default() );

  private:
    void
    moveCamera( const rta::model::Vector3f& delta );
    void
    rotateCamera( const rta::model::Vector2f& delta );
    void
    scaleCamera( float factor );
};

} // namespace view
} // namespace rta
