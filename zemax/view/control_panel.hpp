#pragma once

#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/container_widget.hpp"
#include "custom-hui-impl/window_manager.hpp"
#include "zemax/config.hpp"
#include "zemax/model/rendering/scene_manager.hpp"
#include "zemax/view/aabb_params_dialog.hpp"
#include "zemax/view/scene_objects_list.hpp"
#include "zemax/view/sphere_params_dialog.hpp"
#include <memory>
#include <optional>
#include <sstream>
#include <string>

namespace zemax {
namespace view {

class ControlPanel : public hui::ContainerWidget {
  public:
    explicit ControlPanel( hui::WindowManager*         wm,
                           zemax::model::SceneManager& scene_manager,
                           const dr4::Vec2f&           pos  = Config::ControlPanel::Position,
                           const dr4::Vec2f&           size = Config::ControlPanel::Size )
        : hui::ContainerWidget( wm, pos, size ), scene_manager_( scene_manager )
    {
        border_.reset( wm->getWindow()->CreateRectangle() );

        setDraggable( true );

        border_->SetSize( getSize() );
        border_->SetFillColor( Config::ControlPanel::BackgroundColor );
        border_->SetBorderColor( Config::ControlPanel::BorderColor );
        border_->SetBorderThickness( -Config::ControlPanel::BorderThickness );

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
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        for ( const auto& button : buttons_ )
        {
            if ( event.apply( button.get() ) )
            {
                return true;
            }
        }

        return false;
    }

    bool
    onIdle( const hui::Event& event ) override final
    {
        if ( isPressedJustNow( AddObj ) )
        {
            wm_->pushModal( std::make_unique<zemax::view::SphereParamsDialog>(
                wm_,
                800,
                250,
                500,
                400,
                scene_manager_,
                [this]() { wm_->popModal(); } ) );
        }
        if ( isPressedJustNow( CopyObj ) )
        {
            wm_->pushModal( std::make_unique<zemax::view::SceneObjectsListModal>(
                wm_,
                600.0f,
                200.0f,
                400.0f,
                360.0f,
                scene_manager_,
                [this]() { wm_->popModal(); } ) );
        }
        if ( isPressedJustNow( EditObj ) )
        {
            openEditDialogForTarget();
        }
        if ( isPressedJustNow( DeleteObj ) )
        {
            scene_manager_.deleteTargetObj();
            scene_manager_.needUpdate() = true;
        }

        propagateEventToChildren( event );
        return false;
    }

  private:
    enum ButtonCode {
        AddObj,
        CopyObj,
        EditObj,
        DeleteObj,
        ButtonCount,
    };

    std::unique_ptr<hui::Button> buttons_[ButtonCount];

    bool
    isPressedJustNow( ButtonCode code )
    {
        return dynamic_cast<hui::Button*>( buttons_[code].get() )->isPressedJustNow();
    }

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
    RedrawMyTexture() const override final
    {
        texture_->Draw( *border_ );

        for ( const auto& button : buttons_ )
        {
            button->Redraw();
        }
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
            wm_->pushModal( std::make_unique<zemax::view::AABBParamsDialog>(
                wm_,
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

  private:
    zemax::model::SceneManager&      scene_manager_;
    std::unique_ptr<dr4::Rectangle>  border_;
};

} // namespace view
} // namespace zemax
