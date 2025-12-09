#pragma once

#include "custom-hui-impl/dialog_box.hpp"
#include "zemax/model/primitives/impls/aabb.hpp"
#include "zemax/model/primitives/impls/sphere.hpp"
#include "zemax/model/primitives/impls/torus.hpp"
#include "zemax/model/primitives/impls/hex_prism.hpp"
#include "zemax/model/rendering/scene_manager.hpp"
#include "zemax/view/optical_obj_params_dialog.hpp"
#include <functional>
#include <optional>
#include <vector>

namespace zemax {
namespace view {

class SphereParamsDialog : public OpticalObjParamsDialog {
  public:
    SphereParamsDialog( hui::WindowManager*         wm,
                        float                       x,
                        float                       y,
                        float                       w,
                        float                       h,
                        zemax::model::SceneManager& scene_manager,
                        CloseCb                     close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Add Sphere",
                                  scene_manager,
                                  Mode::Create,
                                  std::nullopt,
                                  { { "Radius", "p1" } },
                                  close_cb ),
          close_cb_( std::move( close_cb ) )
    {
        prefillCommonDefaults();
        prefillSpecificDefaults();
    }

    SphereParamsDialog( hui::WindowManager*         wm,
                        float                       x,
                        float                       y,
                        float                       w,
                        float                       h,
                        zemax::model::SceneManager& scene_manager,
                        size_t                      obj_idx,
                        CloseCb                     close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Edit Sphere",
                                  scene_manager,
                                  Mode::Edit,
                                  obj_idx,
                                  { { "Radius", "p1" } },
                                  close_cb ),
          close_cb_( std::move( close_cb ) )
    {
        auto info = scene_manager_.getObjectInfo( obj_idx );
        prefillCommon( info );
        prefillSpecific();
    }

  private:
    void
    prefillSpecific() override
    {
        if ( !obj_idx_.has_value() )
            return;
        auto& obj = scene_manager_.getObjects()[*obj_idx_];
        if ( auto* sphere = dynamic_cast<model::Sphere*>( obj.get() ) )
        {
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( sphere->getRadius() ) );
        }
    }

    void
    prefillSpecificDefaults() override
    {
        if ( auto* f = findField( "p1" ) )
            f->input->setString( "1.00" );
    }

    bool
    applySpecific( const CommonFields& common ) override
    {
        auto radius = parse( findField( "p1" ), []( double v ) { return v > 0; } );
        if ( !radius )
            return false;

        if ( mode_ == Mode::Create )
        {
            scene_manager_.addSphere( common.material, common.pos, static_cast<float>( *radius ) );
            auto* created = scene_manager_.getObjects().back().get();
            created->setDisplayName( common.name );
            scene_manager_.setTargetObj( created );
        } else if ( mode_ == Mode::Edit && obj_idx_.has_value() )
        {
            auto& obj = scene_manager_.getObjects()[*obj_idx_];
            if ( auto* sphere = dynamic_cast<model::Sphere*>( obj.get() ) )
            {
                sphere->setOrigin( common.pos );
                sphere->setMaterial( common.material );
                sphere->setRadius( static_cast<float>( *radius ) );
                sphere->setDisplayName( common.name );
            }
        }

        scene_manager_.needUpdate() = true;
        if ( close_cb_ )
            close_cb_();
        return true;
    }

  private:
    CloseCb close_cb_;
};

class AABBParamsDialog : public OpticalObjParamsDialog {
  public:
    AABBParamsDialog( hui::WindowManager*         wm,
                      float                       x,
                      float                       y,
                      float                       w,
                      float                       h,
                      zemax::model::SceneManager& scene_manager,
                      CloseCb                     close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Add AABB",
                                  scene_manager,
                                  Mode::Create,
                                  std::nullopt,
                                  { { "Half size X", "p1" },
                                    { "Half size Y", "p2" },
                                    { "Half size Z", "p3" } },
                                  close_cb ),
          close_cb_( std::move( close_cb ) )
    {
        prefillCommonDefaults();
        prefillSpecificDefaults();
    }

    AABBParamsDialog( hui::WindowManager*         wm,
                      float                       x,
                      float                       y,
                      float                       w,
                      float                       h,
                      zemax::model::SceneManager& scene_manager,
                      size_t                      obj_idx,
                      CloseCb                     close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Edit AABB",
                                  scene_manager,
                                  Mode::Edit,
                                  obj_idx,
                                  { { "Half size X", "p1" },
                                    { "Half size Y", "p2" },
                                    { "Half size Z", "p3" } },
                                  close_cb ),
          close_cb_( std::move( close_cb ) )
    {
        auto info = scene_manager_.getObjectInfo( obj_idx );
        prefillCommon( info );
        prefillSpecific();
    }

  private:
    void
    prefillSpecific() override
    {
        if ( !obj_idx_.has_value() )
            return;
        auto& obj = scene_manager_.getObjects()[*obj_idx_];
        if ( auto* aabb = dynamic_cast<model::AABB*>( obj.get() ) )
        {
            auto hs = aabb->getHalfSize();
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( hs.x ) );
            if ( auto* f = findField( "p2" ) )
                f->input->setString( fmt2( hs.y ) );
            if ( auto* f = findField( "p3" ) )
                f->input->setString( fmt2( hs.z ) );
        }
    }

    void
    prefillSpecificDefaults() override
    {
        if ( auto* f = findField( "p1" ) )
            f->input->setString( "1.00" );
        if ( auto* f = findField( "p2" ) )
            f->input->setString( "1.00" );
        if ( auto* f = findField( "p3" ) )
            f->input->setString( "1.00" );
    }

    bool
    applySpecific( const CommonFields& common ) override
    {
        auto p1 = parse( findField( "p1" ), []( double v ) { return v > 0; } );
        auto p2 = parse( findField( "p2" ), []( double v ) { return v > 0; } );
        auto p3 = parse( findField( "p3" ), []( double v ) { return v > 0; } );
        if ( !p1 || !p2 || !p3 )
            return false;

        if ( mode_ == Mode::Create )
        {
            scene_manager_.addAABB( common.material,
                                    common.pos,
                                    { static_cast<float>( *p1 ),
                                      static_cast<float>( *p2 ),
                                      static_cast<float>( *p3 ) } );
            auto* created = scene_manager_.getObjects().back().get();
            created->setDisplayName( common.name );
            scene_manager_.setTargetObj( created );
        } else if ( mode_ == Mode::Edit && obj_idx_.has_value() )
        {
            auto& obj = scene_manager_.getObjects()[*obj_idx_];
            if ( auto* aabb = dynamic_cast<model::AABB*>( obj.get() ) )
            {
                aabb->setOrigin( common.pos );
                aabb->setMaterial( common.material );
                aabb->setHalfSize( { static_cast<float>( *p1 ),
                                     static_cast<float>( *p2 ),
                                     static_cast<float>( *p3 ) } );
                aabb->setDisplayName( common.name );
            }
        }

        scene_manager_.needUpdate() = true;
        if ( close_cb_ )
            close_cb_();
        return true;
    }

  private:
    CloseCb close_cb_;
};

class TorusParamsDialog : public OpticalObjParamsDialog {
  public:
    TorusParamsDialog( hui::WindowManager*         wm,
                       float                       x,
                       float                       y,
                       float                       w,
                       float                       h,
                       zemax::model::SceneManager& scene_manager,
                       CloseCb                     close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Add Torus",
                                  scene_manager,
                                  Mode::Create,
                                  std::nullopt,
                                  { { "Major radius", "p1" }, { "Minor radius", "p2" } },
                                  close_cb ),
          close_cb_( std::move( close_cb ) )
    {
        prefillCommonDefaults();
        prefillSpecificDefaults();
    }

    TorusParamsDialog( hui::WindowManager*         wm,
                       float                       x,
                       float                       y,
                       float                       w,
                       float                       h,
                       zemax::model::SceneManager& scene_manager,
                       size_t                      obj_idx,
                       CloseCb                     close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Edit Torus",
                                  scene_manager,
                                  Mode::Edit,
                                  obj_idx,
                                  { { "Major radius", "p1" }, { "Minor radius", "p2" } },
                                  close_cb ),
          close_cb_( std::move( close_cb ) )
    {
        auto info = scene_manager_.getObjectInfo( obj_idx );
        prefillCommon( info );
        prefillSpecific();
    }

  private:
    void
    prefillSpecific() override
    {
        if ( !obj_idx_.has_value() )
            return;
        auto& obj = scene_manager_.getObjects()[*obj_idx_];
        if ( auto* tor = dynamic_cast<model::Torus*>( obj.get() ) )
        {
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( tor->getMajorRadius() ) );
            if ( auto* f = findField( "p2" ) )
                f->input->setString( fmt2( tor->getMinorRadius() ) );
        }
    }

    void
    prefillSpecificDefaults() override
    {
        if ( auto* f = findField( "p1" ) )
            f->input->setString( "2.00" );
        if ( auto* f = findField( "p2" ) )
            f->input->setString( "0.80" );
    }

    bool
    applySpecific( const CommonFields& common ) override
    {
        auto p1 = parse( findField( "p1" ), []( double v ) { return v > 0; } );
        auto p2 = parse( findField( "p2" ), []( double v ) { return v > 0; } );
        if ( !p1 || !p2 )
            return false;

        if ( mode_ == Mode::Create )
        {
            scene_manager_.addTorus( common.material,
                                     common.pos,
                                     static_cast<float>( *p2 ),
                                     static_cast<float>( *p1 ) );
            auto* created = scene_manager_.getObjects().back().get();
            created->setDisplayName( common.name );
            scene_manager_.setTargetObj( created );
        } else if ( mode_ == Mode::Edit && obj_idx_.has_value() )
        {
            auto& obj = scene_manager_.getObjects()[*obj_idx_];
            if ( auto* tor = dynamic_cast<model::Torus*>( obj.get() ) )
            {
                tor->setOrigin( common.pos );
                tor->setMaterial( common.material );
                tor->setMajorRadius( static_cast<float>( *p1 ) );
                tor->setMinorRadius( static_cast<float>( *p2 ) );
                tor->setDisplayName( common.name );
            }
        }

        scene_manager_.needUpdate() = true;
        if ( close_cb_ )
            close_cb_();
        return true;
    }

  private:
    CloseCb close_cb_;
};

class HexPrismParamsDialog : public OpticalObjParamsDialog {
  public:
    HexPrismParamsDialog( hui::WindowManager*         wm,
                          float                       x,
                          float                       y,
                          float                       w,
                          float                       h,
                          zemax::model::SceneManager& scene_manager,
                          CloseCb                     close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Add HexPrism",
                                  scene_manager,
                                  Mode::Create,
                                  std::nullopt,
                                  { { "Radius", "p1" }, { "Height", "p2" } },
                                  close_cb ),
          close_cb_( std::move( close_cb ) )
    {
        prefillCommonDefaults();
        prefillSpecificDefaults();
    }

    HexPrismParamsDialog( hui::WindowManager*         wm,
                          float                       x,
                          float                       y,
                          float                       w,
                          float                       h,
                          zemax::model::SceneManager& scene_manager,
                          size_t                      obj_idx,
                          CloseCb                     close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Edit HexPrism",
                                  scene_manager,
                                  Mode::Edit,
                                  obj_idx,
                                  { { "Radius", "p1" }, { "Height", "p2" } },
                                  close_cb ),
          close_cb_( std::move( close_cb ) )
    {
        auto info = scene_manager_.getObjectInfo( obj_idx );
        prefillCommon( info );
        prefillSpecific();
    }

  private:
    void
    prefillSpecific() override
    {
        if ( !obj_idx_.has_value() )
            return;
        auto& obj = scene_manager_.getObjects()[*obj_idx_];
        if ( auto* hex = dynamic_cast<model::HexPrism*>( obj.get() ) )
        {
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( hex->getRadius() ) );
            if ( auto* f = findField( "p2" ) )
                f->input->setString( fmt2( hex->getHeight() ) );
        }
    }

    void
    prefillSpecificDefaults() override
    {
        if ( auto* f = findField( "p1" ) )
            f->input->setString( "1.00" );
        if ( auto* f = findField( "p2" ) )
            f->input->setString( "2.00" );
    }

    bool
    applySpecific( const CommonFields& common ) override
    {
        auto p1 = parse( findField( "p1" ), []( double v ) { return v > 0; } );
        auto p2 = parse( findField( "p2" ), []( double v ) { return v > 0; } );
        if ( !p1 || !p2 )
            return false;

        if ( mode_ == Mode::Create )
        {
            scene_manager_.addHexPrism( common.material,
                                        common.pos,
                                        static_cast<float>( *p1 ),
                                        static_cast<float>( *p2 ) );
            auto* created = scene_manager_.getObjects().back().get();
            created->setDisplayName( common.name );
            scene_manager_.setTargetObj( created );
        } else if ( mode_ == Mode::Edit && obj_idx_.has_value() )
        {
            auto& obj = scene_manager_.getObjects()[*obj_idx_];
            if ( auto* hex = dynamic_cast<model::HexPrism*>( obj.get() ) )
            {
                hex->setOrigin( common.pos );
                hex->setMaterial( common.material );
                hex->setRadius( static_cast<float>( *p1 ) );
                hex->setHeight( static_cast<float>( *p2 ) );
                hex->setDisplayName( common.name );
            }
        }

        scene_manager_.needUpdate() = true;
        if ( close_cb_ )
            close_cb_();
        return true;
    }

  private:
    CloseCb close_cb_;
};

} // namespace view
} // namespace zemax
