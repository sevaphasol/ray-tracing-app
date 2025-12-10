#pragma once

#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/closable_panel.hpp"
#include "custom-hui-impl/window_manager.hpp"
#include "zemax/config.hpp"
#include "zemax/model/rendering/scene_manager.hpp"
#include "zemax/model/rendering/vector3.hpp"
#include "zemax/view/object_params_dialogs.hpp"
#include "zemax/view/scene_objects_list.hpp"
#include <memory>
#include <optional>

namespace zemax {
namespace view {

class ControlPanel : public hui::ClosablePanel {
  public:
    explicit ControlPanel( hui::WindowManager*         wm,
                           zemax::model::SceneManager& scene_manager,
                           const dr4::Vec2f&           pos  = Config::ControlPanel::Position,
                           const dr4::Vec2f&           size = Config::ControlPanel::Size )
        : hui::ClosablePanel( wm, pos.x, pos.y, size.x, size.y, "Object Controls" ),
          scene_manager_( scene_manager )
    {
        setupButton( ButtonCode::RotateYawLeft,
                     Config::ControlPanel::Button::RotateObjLeft::Position,
                     Config::ControlPanel::Button::RotateObjLeft::Title );
        setupButton( ButtonCode::RotateYawRight,
                     Config::ControlPanel::Button::RotateObjRight::Position,
                     Config::ControlPanel::Button::RotateObjRight::Title );
        setupButton( ButtonCode::RotatePitchUp,
                     Config::ControlPanel::Button::RotateObjUp::Position,
                     Config::ControlPanel::Button::RotateObjUp::Title );
        setupButton( ButtonCode::RotatePitchDown,
                     Config::ControlPanel::Button::RotateObjDown::Position,
                     Config::ControlPanel::Button::RotateObjDown::Title );
        setupButton( ButtonCode::RotateRollLeft,
                     Config::ControlPanel::Button::RotateObjRollLeft::Position,
                     Config::ControlPanel::Button::RotateObjRollLeft::Title );
        setupButton( ButtonCode::RotateRollRight,
                     Config::ControlPanel::Button::RotateObjRollRight::Position,
                     Config::ControlPanel::Button::RotateObjRollRight::Title );
        setupButton( ButtonCode::MoveObjLeft,
                     Config::ControlPanel::Button::MoveObjLeft::Position,
                     Config::ControlPanel::Button::MoveObjLeft::Title );
        setupButton( ButtonCode::MoveObjRight,
                     Config::ControlPanel::Button::MoveObjRight::Position,
                     Config::ControlPanel::Button::MoveObjRight::Title );
        setupButton( ButtonCode::MoveObjUp,
                     Config::ControlPanel::Button::MoveObjUp::Position,
                     Config::ControlPanel::Button::MoveObjUp::Title );
        setupButton( ButtonCode::MoveObjDown,
                     Config::ControlPanel::Button::MoveObjDown::Position,
                     Config::ControlPanel::Button::MoveObjDown::Title );
        setupButton( ButtonCode::MoveObjForward,
                     Config::ControlPanel::Button::MoveObjForward::Position,
                     Config::ControlPanel::Button::MoveObjForward::Title );
        setupButton( ButtonCode::MoveObjBackward,
                     Config::ControlPanel::Button::MoveObjBackward::Position,
                     Config::ControlPanel::Button::MoveObjBackward::Title );

        buttons_[MoveObjLeft]->setOnHoldPress(
            [this]() { moveTarget( { -Config::Camera::ObjMoveFactor, 0.0f, 0.0f } ); } );
        buttons_[MoveObjRight]->setOnHoldPress(
            [this]() { moveTarget( { Config::Camera::ObjMoveFactor, 0.0f, 0.0f } ); } );
        buttons_[MoveObjUp]->setOnHoldPress(
            [this]() { moveTarget( { 0.0f, Config::Camera::ObjMoveFactor, 0.0f } ); } );
        buttons_[MoveObjDown]->setOnHoldPress(
            [this]() { moveTarget( { 0.0f, -Config::Camera::ObjMoveFactor, 0.0f } ); } );
        buttons_[MoveObjForward]->setOnHoldPress(
            [this]() { moveTarget( { 0.0f, 0.0f, -Config::Camera::ObjMoveFactor } ); } );
        buttons_[MoveObjBackward]->setOnHoldPress(
            [this]() { moveTarget( { 0.0f, 0.0f, Config::Camera::ObjMoveFactor } ); } );
        buttons_[RotateYawLeft]->setOnHoldPress(
            [this]() { rotateTarget( { 0.0f, 1.0f, 0.0f }, -Config::Camera::RotateFactor ); } );
        buttons_[RotateYawRight]->setOnHoldPress(
            [this]() { rotateTarget( { 0.0f, 1.0f, 0.0f }, Config::Camera::RotateFactor ); } );
        buttons_[RotatePitchUp]->setOnHoldPress(
            [this]() { rotateTarget( { 1.0f, 0.0f, 0.0f }, Config::Camera::RotateFactor ); } );
        buttons_[RotatePitchDown]->setOnHoldPress(
            [this]() { rotateTarget( { 1.0f, 0.0f, 0.0f }, -Config::Camera::RotateFactor ); } );
        buttons_[RotateRollLeft]->setOnHoldPress(
            [this]() { rotateTarget( { 0.0f, 0.0f, 1.0f }, -Config::Camera::RotateFactor ); } );
        buttons_[RotateRollRight]->setOnHoldPress(
            [this]() { rotateTarget( { 0.0f, 0.0f, 1.0f }, Config::Camera::RotateFactor ); } );
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

    void
    setupButton( ButtonCode code, const dr4::Vec2f& pos, const char* title )
    {
        hui::Button::Theme theme{ Config::ControlPanel::Button::DefaultColor,
                                  Config::ControlPanel::Button::HoveredColor,
                                  Config::ControlPanel::Button::PressedColor,
                                  Config::ControlPanel::Button::FontColor,
                                  Config::ControlPanel::Button::FontSize };
        buttons_[code] =
            std::move( std::make_unique<hui::Button>( wm_,
                                                      pos,
                                                      Config::ControlPanel::Button::Size,
                                                      title,
                                                      theme ) );
        buttons_[code]->setParent( this );
    }

    void
    moveTarget( const zemax::model::Vector3f& delta )
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
    rotateTarget( const zemax::model::Vector3f& axis_world, float angle )
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
            wm_->pushModal( std::make_unique<zemax::view::SphereParamsDialog>(
                wm_,
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
                std::make_unique<zemax::view::AABBParamsDialog>( wm_,
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
    zemax::model::SceneManager& scene_manager_;
};

} // namespace view
} // namespace zemax
