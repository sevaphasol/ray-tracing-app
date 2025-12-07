#pragma once

#include "custom-hui-impl/button.hpp"
#include "custom-hui-impl/container_widget.hpp"
#include "custom-hui-impl/input_text.hpp"
#include "custom-hui-impl/text_field.hpp"
#include "custom-hui-impl/window_manager.hpp"
#include "dr4/math/color.hpp"
#include "zemax/config.hpp"
#include "zemax/model/primitives/material.hpp"
#include "zemax/model/rendering/scene_manager.hpp"
#include "zemax/model/rendering/vector2.hpp"
#include "zemax/model/rendering/vector3.hpp"
#include "zemax/view/obj_info_box.hpp"
#include "zemax/view/obj_params_dialog.hpp"
#include "zemax/view/scene_objects_list.hpp"
#include "zemax/view/scrollable_buttons_widget.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>

namespace zemax {
namespace view {

class ControlPanel : public hui::ContainerWidget {
  public:
    explicit ControlPanel( hui::WindowManager* wm, zemax::model::SceneManager& scene_manager )
        : hui::ContainerWidget( wm, Config::ControlPanel::Position, Config::ControlPanel::Size ),
          scene_manager_( scene_manager ),
          font_( wm->getWindow()->GetDefaultFont() ),
          camera_( scene_manager.getCamera() ),
          scrollbar_( wm,
                      Config::ControlPanel::ScrollBar::Position,
                      Config::ControlPanel::ScrollBar::Size )
    {
        border_.reset( wm->getWindow()->CreateRectangle() );

        setDraggable( true );

        // setRelPos( Config::ControlPanel::Position );
        // setSize( Config::ControlPanel::Size );

        border_->SetSize( getSize() );
        border_->SetFillColor( Config::ControlPanel::BackgroundColor );
        border_->SetBorderColor( Config::ControlPanel::BorderColor );
        border_->SetBorderThickness( -Config::ControlPanel::BorderThickness );

        setupButton( ButtonCode::MoveLeft,
                     Config::ControlPanel::Button::MvL::Position,
                     Config::ControlPanel::Button::MvL::Title );
        setupButton( ButtonCode::MoveRight,
                     Config::ControlPanel::Button::MvR::Position,
                     Config::ControlPanel::Button::MvR::Title );
        setupButton( ButtonCode::MoveUp,
                     Config::ControlPanel::Button::MvU::Position,
                     Config::ControlPanel::Button::MvU::Title );
        setupButton( ButtonCode::MoveDown,
                     Config::ControlPanel::Button::MvD::Position,
                     Config::ControlPanel::Button::MvD::Title );
        setupButton( ButtonCode::MoveForward,
                     Config::ControlPanel::Button::MvF::Position,
                     Config::ControlPanel::Button::MvF::Title );
        setupButton( ButtonCode::MoveBackward,
                     Config::ControlPanel::Button::MvB::Position,
                     Config::ControlPanel::Button::MvB::Title );
        setupButton( ButtonCode::RotateLeft,
                     Config::ControlPanel::Button::RtL::Position,
                     Config::ControlPanel::Button::RtL::Title );
        setupButton( ButtonCode::RotateRight,
                     Config::ControlPanel::Button::RtR::Position,
                     Config::ControlPanel::Button::RtR::Title );
        setupButton( ButtonCode::RotateUp,
                     Config::ControlPanel::Button::RtU::Position,
                     Config::ControlPanel::Button::RtU::Title );
        setupButton( ButtonCode::RotateDown,
                     Config::ControlPanel::Button::RtD::Position,
                     Config::ControlPanel::Button::RtD::Title );
        setupButton( ButtonCode::ScaleUp,
                     Config::ControlPanel::Button::ScaleUp::Position,
                     Config::ControlPanel::Button::ScaleUp::Title );
        setupButton( ButtonCode::ScaleDown,
                     Config::ControlPanel::Button::ScaleDown::Position,
                     Config::ControlPanel::Button::ScaleDown::Title );

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
                     Config::ControlPanel::Button::MoveObjForwrd::Position,
                     Config::ControlPanel::Button::MoveObjForwrd::Title );
        setupButton( ButtonCode::MoveObjBackward,
                     Config::ControlPanel::Button::MoveObjBackward::Position,
                     Config::ControlPanel::Button::MoveObjBackward::Title );

        setupButton( ButtonCode::AddObj,
                     Config::ControlPanel::Button::AddObj::Position,
                     Config::ControlPanel::Button::AddObj::Title );
        setupButton( ButtonCode::CopyObj,
                     Config::ControlPanel::Button::CopyObj::Position,
                     Config::ControlPanel::Button::CopyObj::Title );
        setupButton( ButtonCode::DeleteObj,
                     Config::ControlPanel::Button::DeleteObj::Position,
                     Config::ControlPanel::Button::DeleteObj::Title );

        setupTextField( TextFieldCode::NewObjX,
                        Config::ControlPanel::TextField::X::Position,
                        Config::ControlPanel::TextField::X::Title );
        setupTextField( TextFieldCode::NewObjY,
                        Config::ControlPanel::TextField::Y::Position,
                        Config::ControlPanel::TextField::Y::Title );
        setupTextField( TextFieldCode::NewObjZ,
                        Config::ControlPanel::TextField::Z::Position,
                        Config::ControlPanel::TextField::Z::Title );
        setupTextField( TextFieldCode::NewObjS,
                        Config::ControlPanel::TextField::Size::Position,
                        Config::ControlPanel::TextField::Size::Title );
        setupTextField( TextFieldCode::NewObjR,
                        Config::ControlPanel::TextField::R::Position,
                        Config::ControlPanel::TextField::R::Title );
        setupTextField( TextFieldCode::NewObjG,
                        Config::ControlPanel::TextField::G::Position,
                        Config::ControlPanel::TextField::G::Title );
        setupTextField( TextFieldCode::NewObjB,
                        Config::ControlPanel::TextField::B::Position,
                        Config::ControlPanel::TextField::B::Title );
        setupTextField( TextFieldCode::NewObjF,
                        Config::ControlPanel::TextField::RefFactor::Position,
                        Config::ControlPanel::TextField::RefFactor::Title );

        scrollbar_.setParent( this );
        setupScrollBarButton( ScrollBarButtonCode::Sphere, "Sphere" );
        setupScrollBarButton( ScrollBarButtonCode::Plane, "Plane" );
        setupScrollBarButton( ScrollBarButtonCode::AABB, "AABB" );
    }

    void
    setFont( const dr4::Font* font )
    {
        font_ = font;
    }

    static std::string
    getObjInfoString( model::SceneManager::ObjectInfo info )
    {
        std::stringstream ss;

        ss << info.pos.x << " " << info.pos.y << " " << info.pos.z << "\n";
        ss << info.type_name;

        return ss.str();
    }

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        for ( const auto& button : buttons_ )
        {
            if ( event.apply( button.get() ) )
            {
                try
                {
                    auto it = dynamic_cast<const hui::IdleEvent&>( event );

                    return true;
                } catch ( ... )
                {
                    // std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' <<
                    // __func__
                    // << std::endl;
                    return true;
                }
            }
        }

        for ( const auto& text_field : text_fields_ )
        {
            if ( event.apply( text_field.get() ) )
            {
                try
                {
                    auto it = dynamic_cast<const hui::IdleEvent&>( event );

                    return true;
                } catch ( ... )
                {
                    // std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' <<
                    // __func__
                    // << std::endl;
                    return true;
                }
            }
        }

        if ( event.apply( &scrollbar_ ) )
        {
            try
            {
                auto it = dynamic_cast<const hui::IdleEvent&>( event );

                return true;
            } catch ( ... )
            {
                // std::cerr << "DEBUG IN " << __FILE__ << ':' << __LINE__ << ':' << __func__
                // << std::endl;
                return true;
            }
        }

        return false;
    }

    virtual bool
    onIdle( const hui::Event& event ) override final
    {
        if ( isPressed( MoveLeft ) )
        {
            scene_manager_.getCamera().move( { -Config::Camera::MoveFactor, 0.0f, 0.0f } );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( MoveRight ) )
        {

            scene_manager_.getCamera().move( { Config::Camera::MoveFactor, 0.0f, 0.0f } );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( MoveUp ) )
        {

            scene_manager_.getCamera().move( { 0.0f, Config::Camera::MoveFactor, 0.0f } );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( MoveDown ) )
        {
            scene_manager_.getCamera().move( { 0.0f, -Config::Camera::MoveFactor, 0.0f } );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( MoveForward ) )
        {

            scene_manager_.getCamera().move( { 0.0f, 0.0f, -Config::Camera::MoveFactor } );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( MoveBackward ) )
        {
            scene_manager_.getCamera().move( { 0.0f, 0.0f, Config::Camera::MoveFactor } );
            scene_manager_.needUpdate() = true;
        }

        if ( isPressed( RotateLeft ) )
        {
            scene_manager_.getCamera().rotate( { Config::Camera::RotateFactor, 0.0f } );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( RotateRight ) )
        {
            scene_manager_.getCamera().rotate( { -Config::Camera::RotateFactor, 0.0f } );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( RotateUp ) )
        {
            scene_manager_.getCamera().rotate( { 0.0f, -Config::Camera::RotateFactor } );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( RotateDown ) )
        {
            scene_manager_.getCamera().rotate( { 0.0f, Config::Camera::RotateFactor } );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( ScaleUp ) )
        {
            scene_manager_.getCamera().scale( -Config::Camera::ScaleFactor );
            scene_manager_.needUpdate() = true;
        }
        if ( isPressed( ScaleDown ) )
        {
            scene_manager_.getCamera().scale( Config::Camera::ScaleFactor );
            scene_manager_.needUpdate() = true;
        }

        if ( isPressed( MoveObjLeft ) )
        {
            if ( scene_manager_.getTargetObj() != nullptr )
            {
                scene_manager_.getTargetObj()->move(
                    zemax::model::Vector3f( -Config::Camera::ObjMoveFactor, 0, 0 ) );
                scene_manager_.needUpdate() = true;
            }
        }
        if ( isPressed( MoveObjRight ) )
        {
            if ( scene_manager_.getTargetObj() != nullptr )
            {
                scene_manager_.getTargetObj()->move(
                    zemax::model::Vector3f( Config::Camera::ObjMoveFactor, 0, 0 ) );
                scene_manager_.needUpdate() = true;
            }
        }
        if ( isPressed( MoveObjUp ) )
        {
            if ( scene_manager_.getTargetObj() != nullptr )
            {
                scene_manager_.getTargetObj()->move(
                    zemax::model::Vector3f( 0, Config::Camera::ObjMoveFactor, 0 ) );
                scene_manager_.needUpdate() = true;
            }
        }
        if ( isPressed( MoveObjDown ) )
        {
            if ( scene_manager_.getTargetObj() != nullptr )
            {
                scene_manager_.getTargetObj()->move(
                    zemax::model::Vector3f( 0, -Config::Camera::ObjMoveFactor, 0 ) );
                scene_manager_.needUpdate() = true;
            }
        }
        if ( isPressed( MoveObjForward ) )
        {
            if ( scene_manager_.getTargetObj() != nullptr )
            {
                scene_manager_.getTargetObj()->move(
                    zemax::model::Vector3f( 0, 0, -Config::Camera::ObjMoveFactor ) );
                scene_manager_.needUpdate() = true;
            }
        }
        if ( isPressed( MoveObjBackward ) )
        {
            if ( scene_manager_.getTargetObj() != nullptr )
            {
                scene_manager_.getTargetObj()->move(
                    zemax::model::Vector3f( 0, 0, Config::Camera::ObjMoveFactor ) );
                scene_manager_.needUpdate() = true;
            }
        }

        if ( isPressedJustNow( AddObj ) )
        {
            wm_->pushModal( std::make_unique<zemax::view::SphereParamsDialog>(
                wm_,
                800,
                250,
                500,
                400,
                scene_manager_,
                [this]() {
                    wm_->popModal();
                    scene_manager_.setTargetObj( nullptr );
                } ) );

            // wm_->pushModal( std::make_unique<zemax::view::SphereCreationDialog>(
            // wm_,
            // 850,
            // 300,
            // 500,
            // 300,
            // [&]( const model::Material& mat, const model::Vector3f& origin, float radius ) {
            // scene_manager_.addSphere( mat, origin, radius );
            // },
            // [this]() { wm_->popModal(); } ) );

            //             auto new_obj_x = text_fields_[TextFieldCode::NewObjX]->getDouble();
            //             auto new_obj_y = text_fields_[TextFieldCode::NewObjY]->getDouble();
            //             auto new_obj_z = text_fields_[TextFieldCode::NewObjZ]->getDouble();
            //             auto new_obj_s = text_fields_[TextFieldCode::NewObjS]->getDouble();
            //
            //             auto new_obj_r = text_fields_[TextFieldCode::NewObjR]->getUint32();
            //             auto new_obj_g = text_fields_[TextFieldCode::NewObjG]->getUint32();
            //             auto new_obj_b = text_fields_[TextFieldCode::NewObjB]->getUint32();
            //             auto new_obj_f = text_fields_[TextFieldCode::NewObjF]->getDouble();
            //
            //             if ( new_obj_x.has_value() && new_obj_y.has_value() &&
            //             new_obj_z.has_value() &&
            //                  new_obj_s.has_value() && new_obj_r.has_value() &&
            //                  new_obj_g.has_value() && new_obj_b.has_value() &&
            //                  new_obj_f.has_value() )
            //             {
            //                 auto origin = zemax::model::Vector3f( new_obj_x.value(),
            //                                                       new_obj_y.value(),
            //                                                       new_obj_z.value() );
            //
            //                 auto color =
            //                     zemax::model::Color( new_obj_r.value(), new_obj_g.value(),
            //                     new_obj_b.value() );
            //
            //                 auto reflection_factor = new_obj_f.value();
            //
            //                 model::Material material( color, reflection_factor );
            //
            //                 switch ( scrollbar_.getCurActiveButton() )
            //                 {
            //                     case ScrollBarButtonCode::Sphere:
            //                         scene_manager_.addSphere( material, origin,
            //                         new_obj_s.value()
            //                         ); break;
            //                     case ScrollBarButtonCode::AABB:
            //                         scene_manager_.addAABB( material,
            //                                                 origin,
            //                                                 zemax::model::Vector3f(
            //                                                 new_obj_s.value(),
            //                                                                         new_obj_s.value(),
            //                                                                         new_obj_s.value()
            //                                                                         ) );
            //                         break;
            //                     case ScrollBarButtonCode::Plane:
            //                         scene_manager_.addPlane( material, origin, { 0.0f, 1.0f,
            //                         0.0f } ); break;
            //                     default:
            //                         break;
            //                 }
            //
            //                 scene_manager_.needUpdate() = true;
            //             }
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
                [this]() {
                    wm_->popModal();
                    scene_manager_.setTargetObj( nullptr );
                } ) );

            //             auto new_obj_x = text_fields_[TextFieldCode::NewObjX]->getDouble();
            //             auto new_obj_y = text_fields_[TextFieldCode::NewObjY]->getDouble();
            //             auto new_obj_z = text_fields_[TextFieldCode::NewObjZ]->getDouble();
            //
            //             if ( new_obj_x.has_value() && new_obj_y.has_value() &&
            //             new_obj_z.has_value() )
            //             {
            //                 scene_manager_.copyTargetObj( new_obj_x.value(),
            //                                               new_obj_y.value(),
            //                                               new_obj_z.value() );
            //             }
            //
            //             scene_manager_.needUpdate() = true;
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
    const dr4::Font* font_;

    enum ButtonCode {
        MoveLeft,
        MoveRight,
        MoveUp,
        MoveDown,
        MoveForward,
        MoveBackward,
        RotateLeft,
        RotateRight,
        RotateUp,
        RotateDown,
        ScaleUp,
        ScaleDown,
        MoveObjLeft,
        MoveObjRight,
        MoveObjUp,
        MoveObjDown,
        MoveObjForward,
        MoveObjBackward,
        AddObj,
        CopyObj,
        DeleteObj,
        ButtonCount,
    };

    std::unique_ptr<hui::Button> buttons_[ButtonCount];

    enum TextFieldCode {
        NewObjX,
        NewObjY,
        NewObjZ,
        NewObjS,
        NewObjR,
        NewObjG,
        NewObjB,
        NewObjF,
        TextFieldCount,
    };

    std::unique_ptr<hui::InputText> text_fields_[TextFieldCount];

    enum ScrollBarButtonCode {
        Sphere,
        Plane,
        AABB,
        ScrollBarButtonCodeCount,
    };

    ScrollableButtonsWidget scrollbar_;

    bool
    isPressedJustNow( ButtonCode code )
    {
        return dynamic_cast<hui::Button*>( buttons_[code].get() )->isPressedJustNow();
    }

    bool
    isPressed( ButtonCode code )
    {
        return dynamic_cast<hui::Button*>( buttons_[code].get() )->isPressed();
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
    setupTextField( TextFieldCode code, const dr4::Vec2f& pos, const std::string& title )
    {
        text_fields_[code] = std::move(
            std::make_unique<hui::InputText>( wm_, pos, Config::ControlPanel::TextField::Size ) );
        text_fields_[code]->setParent( this );
    }

    void
    setupScrollBarButton( ScrollBarButtonCode code, const char* label )
    {
        scrollbar_.addButton(
            std::make_unique<hui::Button>( wm_,
                                           Config::ControlPanel::ScrollBar::Button::Position,
                                           Config::ControlPanel::ScrollBar::Button::Size,
                                           Config::ControlPanel::Button::DefaultColor,
                                           Config::ControlPanel::Button::HoveredColor,
                                           Config::ControlPanel::Button::PressedColor,
                                           label,
                                           Config::ControlPanel::Button::FontColor,
                                           Config::ControlPanel::Button::FontSize ) );
    }

    virtual void
    RedrawMyTexture() const override final
    {
        texture_->Draw( *border_ );

        for ( const auto& button : buttons_ )
        {
            button->Redraw();
        }

        for ( const auto& text_field : text_fields_ )
        {
            text_field->Redraw();
        }

        scrollbar_.Redraw();
    }

  private:
    zemax::model::SceneManager& scene_manager_;
    zemax::model::Camera&       camera_;

    std::unique_ptr<dr4::Rectangle> border_;

    std::unique_ptr<zemax::view::ObjParams> obj_params_;
};

} // namespace view
} // namespace zemax
