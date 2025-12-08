#pragma once

#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/window_manager.hpp"
#include "zemax/config.hpp"
#include "zemax/model/rendering/scene_manager.hpp"
#include "zemax/model/rendering/vector3.hpp"
#include "zemax/view/aabb_params_dialog.hpp"
#include "zemax/view/closable_panel.hpp"
#include "zemax/view/scene_objects_list.hpp"
#include "zemax/view/sphere_params_dialog.hpp"
#include <memory>
#include <optional>

namespace zemax {
namespace view {

class ControlPanel : public ClosablePanel {
  public:
    explicit ControlPanel( hui::WindowManager*         wm,
                           zemax::model::SceneManager& scene_manager,
                           const dr4::Vec2f&           pos  = Config::ControlPanel::Position,
                           const dr4::Vec2f&           size = Config::ControlPanel::Size )
        : ClosablePanel( wm, pos.x, pos.y, size.x, size.y, "Object Controls" ),
          scene_manager_( scene_manager )
    {
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

        setupButton( ButtonCode::AddObj,
                     Config::ControlPanel::Button::AddObj::Position,
                     Config::ControlPanel::Button::AddObj::Title );
        setupButton( ButtonCode::CopyObj,
                     Config::ControlPanel::Button::CopyObj::Position,
                     Config::ControlPanel::Button::CopyObj::Title );
        setupButton( ButtonCode::EditObj,
                     Config::ControlPanel::Button::EditObj::Position,
                     Config::ControlPanel::Button::EditObj::Title );
        setupButton( ButtonCode::DeleteObj,
                     Config::ControlPanel::Button::DeleteObj::Position,
                     Config::ControlPanel::Button::DeleteObj::Title );

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

        buttons_[AddObj]->setOnClick( [this]() {
            wm_->pushModal( std::make_unique<zemax::view::SphereParamsDialog>(
                wm_,
                800,
                250,
                500,
                400,
                scene_manager_,
                [this]() { wm_->popModal(); } ) );
        } );
        buttons_[CopyObj]->setOnClick( [this]() { copyTarget(); } );
        // Edit now handled by persistent editor panel
        buttons_[EditObj]->setOnClick( []() {} );
        buttons_[DeleteObj]->setOnClick( [this]() {
            scene_manager_.deleteTargetObj();
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

        return ObjInfoBox::propagateEventToChildren( event );
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

        ObjInfoBox::RedrawMyTexture();

        for ( const auto& button : buttons_ )
        {
            button->Redraw();
        }
    }

  private:
    enum ButtonCode {
        MoveObjLeft,
        MoveObjRight,
        MoveObjUp,
        MoveObjDown,
        MoveObjForward,
        MoveObjBackward,
        AddObj,
        CopyObj,
        EditObj,
        DeleteObj,
        ButtonCount,
    };

    std::unique_ptr<hui::Button> buttons_[ButtonCount];

    void
    setupButton( ButtonCode code, const dr4::Vec2f& pos, const char* title )
    {
        buttons_[code] =
            std::move( std::make_unique<hui::Button>( wm_,
                                                      pos,
                                                      Config::ControlPanel::Button::Size,
                                                      Config::ControlPanel::Button::DefaultColor,
                                                      Config::ControlPanel::Button::HoveredColor,
                                                      Config::ControlPanel::Button::PressedColor,
                                                      title,
                                                      Config::ControlPanel::Button::FontColor,
                                                      Config::ControlPanel::Button::FontSize ) );
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
        auto* target = scene_manager_.getTargetObj();
        if ( target == nullptr )
        {
            return;
        }

        auto  origin = target->getOrigin();
        float dx     = Config::Camera::ObjMoveFactor * 2.0f;
        scene_manager_.copyTargetObj( origin.x + dx, origin.y, origin.z );
    }

  private:
    zemax::model::SceneManager& scene_manager_;
};

} // namespace view
} // namespace zemax
