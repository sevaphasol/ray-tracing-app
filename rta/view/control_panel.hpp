#pragma once

#include "hui/button.hpp"
#include "hui/window_manager.hpp"
#include "rta/model/rendering/scene_manager.hpp"
#include "rta/model/rendering/vector3.hpp"
#include "rta/view/control_panel_base.hpp"
#include "rta/view/object_params_dialogs.hpp"
#include "rta/view/scene_objects_list.hpp"
#include <memory>
#include <optional>

namespace rta {
namespace view {

class ControlPanel : public ControlPanelBase {
  public:
    struct Theme
    {
        dr4::Vec2f         panel_pos{ 50.0f, 465.0f };
        dr4::Vec2f         panel_size{ 380.0f, 385.0f };
        dr4::Vec2f         button_size{ 100.0f, 50.0f };
        hui::Button::Theme button_theme{ { 30, 30, 30, 255 },
                                         { 50, 70, 30, 255 },
                                         { 100, 150, 0, 255 },
                                         { 220, 220, 220, 255 },
                                         15 };
        float              move_step   = 0.1f;
        float              rotate_step = 0.03f;

        dr4::Vec2f rot_yaw_left{ 30.0f, 80.0f };
        dr4::Vec2f rot_yaw_right{ 30.0f, 140.0f };
        dr4::Vec2f rot_pitch_up{ 140.0f, 80.0f };
        dr4::Vec2f rot_pitch_down{ 140.0f, 140.0f };
        dr4::Vec2f rot_roll_left{ 250.0f, 80.0f };
        dr4::Vec2f rot_roll_right{ 250.0f, 140.0f };

        dr4::Vec2f move_left{ 30.0f, 220.0f };
        dr4::Vec2f move_right{ 30.0f, 280.0f };
        dr4::Vec2f move_up{ 140.0f, 220.0f };
        dr4::Vec2f move_down{ 140.0f, 280.0f };
        dr4::Vec2f move_fwd{ 250.0f, 220.0f };
        dr4::Vec2f move_back{ 250.0f, 280.0f };

        dr4::Vec2f add_obj{ 30.0f, 300.0f };
        dr4::Vec2f copy_obj{ 140.0f, 300.0f };
        dr4::Vec2f edit_obj{ 250.0f, 300.0f };
        dr4::Vec2f delete_obj{ 30.0f, 360.0f };

        static Theme
        Default()
        {
            return Theme();
        }
    };

    explicit ControlPanel( hui::WindowManager*       wm,
                           rta::model::SceneManager& scene_manager,
                           const Theme&              theme = Theme::Default() )
        : ControlPanelBase( wm,
                            theme.panel_pos,
                            theme.panel_size,
                            theme.button_size,
                            "Object Controls" ),
          scene_manager_( scene_manager ),
          theme_( theme )
    {
        auto& t = theme_.button_theme;
        addButton( ButtonCode::RotateYawLeft, theme_.rot_yaw_left, "Y-", t );
        addButton( ButtonCode::RotateYawRight, theme_.rot_yaw_right, "Y+", t );
        addButton( ButtonCode::RotatePitchUp, theme_.rot_pitch_up, "X+", t );
        addButton( ButtonCode::RotatePitchDown, theme_.rot_pitch_down, "X-", t );
        addButton( ButtonCode::RotateRollLeft, theme_.rot_roll_left, "Z-", t );
        addButton( ButtonCode::RotateRollRight, theme_.rot_roll_right, "Z+", t );
        addButton( ButtonCode::MoveObjLeft, theme_.move_left, "left", t );
        addButton( ButtonCode::MoveObjRight, theme_.move_right, "right", t );
        addButton( ButtonCode::MoveObjUp, theme_.move_up, "up", t );
        addButton( ButtonCode::MoveObjDown, theme_.move_down, "down", t );
        addButton( ButtonCode::MoveObjForward, theme_.move_fwd, "fwd", t );
        addButton( ButtonCode::MoveObjBackward, theme_.move_back, "bwd", t );

        buttons_[MoveObjLeft]->setOnHoldPress(
            [this]() { moveTarget( { -theme_.move_step, 0.0f, 0.0f } ); } );
        buttons_[MoveObjRight]->setOnHoldPress(
            [this]() { moveTarget( { theme_.move_step, 0.0f, 0.0f } ); } );
        buttons_[MoveObjUp]->setOnHoldPress(
            [this]() { moveTarget( { 0.0f, theme_.move_step, 0.0f } ); } );
        buttons_[MoveObjDown]->setOnHoldPress(
            [this]() { moveTarget( { 0.0f, -theme_.move_step, 0.0f } ); } );
        buttons_[MoveObjForward]->setOnHoldPress(
            [this]() { moveTarget( { 0.0f, 0.0f, -theme_.move_step } ); } );
        buttons_[MoveObjBackward]->setOnHoldPress(
            [this]() { moveTarget( { 0.0f, 0.0f, theme_.move_step } ); } );

        buttons_[RotateYawLeft]->setOnHoldPress(
            [this]() { rotateTarget( { 0.0f, 1.0f, 0.0f }, -theme_.rotate_step ); } );
        buttons_[RotateYawRight]->setOnHoldPress(
            [this]() { rotateTarget( { 0.0f, 1.0f, 0.0f }, theme_.rotate_step ); } );
        buttons_[RotatePitchUp]->setOnHoldPress(
            [this]() { rotateTarget( { 1.0f, 0.0f, 0.0f }, theme_.rotate_step ); } );
        buttons_[RotatePitchDown]->setOnHoldPress(
            [this]() { rotateTarget( { 1.0f, 0.0f, 0.0f }, -theme_.rotate_step ); } );
        buttons_[RotateRollLeft]->setOnHoldPress(
            [this]() { rotateTarget( { 0.0f, 0.0f, 1.0f }, -theme_.rotate_step ); } );
        buttons_[RotateRollRight]->setOnHoldPress(
            [this]() { rotateTarget( { 0.0f, 0.0f, 1.0f }, theme_.rotate_step ); } );
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

    void
    RedrawMyTexture() const override final
    {
        if ( !visible_ )
        {
            return;
        }

        hui::DialogBox::RedrawMyTexture();

        for ( const auto& button : buttons_ )
        {
            button->Redraw();
        }
    }

  private:
    enum ButtonCode {
        RotateYawLeft,
        RotateYawRight,
        RotatePitchUp,
        RotatePitchDown,
        RotateRollLeft,
        RotateRollRight,
        MoveObjLeft,
        MoveObjRight,
        MoveObjUp,
        MoveObjDown,
        MoveObjForward,
        MoveObjBackward,
        ButtonCount,
    };

    std::unique_ptr<hui::Button> buttons_[ButtonCount];
    Theme                        theme_;

    void
    addButton( ButtonCode                code,
               const dr4::Vec2f&         pos,
               const char*               title,
               const hui::Button::Theme& theme )
    {
        buttons_[code] =
            std::make_unique<hui::Button>( wm_, pos, theme_.button_size, title, theme );
        buttons_[code]->setParent( this );
    }

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

    std::optional<size_t>
    getTargetIndex() const
    {
        auto* target = scene_manager_.getTargetObj();
        if ( target == nullptr )
        {
            return std::nullopt;
        }

        auto& objects = scene_manager_.getObjects();
        for ( size_t i = 0; i < objects.size(); ++i )
        {
            if ( objects[i].get() == target )
            {
                return i;
            }
        }

        return std::nullopt;
    }

    void
    openEditDialogForIndex( size_t idx )
    {
        auto info = scene_manager_.getObjectInfo( idx );

        if ( info.type_name == "Sphere" )
        {
            wm_->pushModal(
                std::make_unique<rta::view::SphereParamsDialog>( wm_,
                                                                 800,
                                                                 250,
                                                                 500,
                                                                 400,
                                                                 scene_manager_,
                                                                 idx,
                                                                 [this]() { wm_->popModal(); } ) );
        } else if ( info.type_name == "AABB" )
        {
            wm_->pushModal(
                std::make_unique<rta::view::AABBParamsDialog>( wm_,
                                                               800,
                                                               250,
                                                               550,
                                                               450,
                                                               scene_manager_,
                                                               idx,
                                                               [this]() { wm_->popModal(); } ) );
        }
    }

    void
    openEditDialogForTarget()
    {
        auto idx = getTargetIndex();
        if ( !idx.has_value() )
        {
            return;
        }

        openEditDialogForIndex( idx.value() );
    }

    void
    copyTarget()
    {
        // Copy now handled inside ObjectEditorPanel
    }

  private:
    rta::model::SceneManager& scene_manager_;
};

} // namespace view
} // namespace rta
