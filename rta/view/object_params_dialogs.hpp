#pragma once

#include "hui/dialog_box.hpp"
#include "rta/model/primitives/impls/aabb.hpp"
#include "rta/model/primitives/impls/capped_cone.hpp"
#include "rta/model/primitives/impls/capped_cylinder.hpp"
#include "rta/model/primitives/impls/capsule.hpp"
#include "rta/model/primitives/impls/ellipse.hpp"
#include "rta/model/primitives/impls/ellipsoid.hpp"
#include "rta/model/primitives/impls/goursat.hpp"
#include "rta/model/primitives/impls/hex_prism.hpp"
#include "rta/model/primitives/impls/rounded_box.hpp"
#include "rta/model/primitives/impls/rounded_cone.hpp"
#include "rta/model/primitives/impls/sphere.hpp"
#include "rta/model/primitives/impls/torus.hpp"
#include "rta/model/primitives/impls/triangle.hpp"
#include "rta/model/primitives/impls/wedge.hpp"
#include "rta/model/rendering/scene_manager.hpp"
#include "rta/view/optical_obj_params_dialog.hpp"
#include <functional>
#include <optional>
#include <vector>

namespace rta {
namespace view {

class SphereParamsDialog : public OpticalObjParamsDialog {
  public:
    SphereParamsDialog( hui::WindowManager*       wm,
                        float                     x,
                        float                     y,
                        float                     w,
                        float                     h,
                        rta::model::SceneManager& scene_manager,
                        CloseCb                   close_cb )
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

    SphereParamsDialog( hui::WindowManager*       wm,
                        float                     x,
                        float                     y,
                        float                     w,
                        float                     h,
                        rta::model::SceneManager& scene_manager,
                        size_t                    obj_idx,
                        CloseCb                   close_cb )
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
    AABBParamsDialog( hui::WindowManager*       wm,
                      float                     x,
                      float                     y,
                      float                     w,
                      float                     h,
                      rta::model::SceneManager& scene_manager,
                      CloseCb                   close_cb )
        : OpticalObjParamsDialog(
              wm,
              x,
              y,
              w,
              h,
              "Add AABB",
              scene_manager,
              Mode::Create,
              std::nullopt,
              { { "Half size X", "p1" }, { "Half size Y", "p2" }, { "Half size Z", "p3" } },
              close_cb ),
          close_cb_( std::move( close_cb ) )
    {
        prefillCommonDefaults();
        prefillSpecificDefaults();
    }

    AABBParamsDialog( hui::WindowManager*       wm,
                      float                     x,
                      float                     y,
                      float                     w,
                      float                     h,
                      rta::model::SceneManager& scene_manager,
                      size_t                    obj_idx,
                      CloseCb                   close_cb )
        : OpticalObjParamsDialog(
              wm,
              x,
              y,
              w,
              h,
              "Edit AABB",
              scene_manager,
              Mode::Edit,
              obj_idx,
              { { "Half size X", "p1" }, { "Half size Y", "p2" }, { "Half size Z", "p3" } },
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
    TorusParamsDialog( hui::WindowManager*       wm,
                       float                     x,
                       float                     y,
                       float                     w,
                       float                     h,
                       rta::model::SceneManager& scene_manager,
                       CloseCb                   close_cb )
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

    TorusParamsDialog( hui::WindowManager*       wm,
                       float                     x,
                       float                     y,
                       float                     w,
                       float                     h,
                       rta::model::SceneManager& scene_manager,
                       size_t                    obj_idx,
                       CloseCb                   close_cb )
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
    HexPrismParamsDialog( hui::WindowManager*       wm,
                          float                     x,
                          float                     y,
                          float                     w,
                          float                     h,
                          rta::model::SceneManager& scene_manager,
                          CloseCb                   close_cb )
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

    HexPrismParamsDialog( hui::WindowManager*       wm,
                          float                     x,
                          float                     y,
                          float                     w,
                          float                     h,
                          rta::model::SceneManager& scene_manager,
                          size_t                    obj_idx,
                          CloseCb                   close_cb )
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

class GoursatParamsDialog : public OpticalObjParamsDialog {
  public:
    GoursatParamsDialog( hui::WindowManager*       wm,
                         float                     x,
                         float                     y,
                         float                     w,
                         float                     h,
                         rta::model::SceneManager& scene_manager,
                         CloseCb                   close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Add Goursat",
                                  scene_manager,
                                  Mode::Create,
                                  std::nullopt,
                                  { { "ka", "ka" }, { "kb", "kb" } },
                                  close_cb ),
          close_cb_( std::move( close_cb ) )
    {
        prefillCommonDefaults();
        prefillSpecificDefaults();
    }

    GoursatParamsDialog( hui::WindowManager*       wm,
                         float                     x,
                         float                     y,
                         float                     w,
                         float                     h,
                         rta::model::SceneManager& scene_manager,
                         size_t                    obj_idx,
                         CloseCb                   close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Edit Goursat",
                                  scene_manager,
                                  Mode::Edit,
                                  obj_idx,
                                  { { "ka", "ka" }, { "kb", "kb" } },
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
        if ( auto* g = dynamic_cast<model::Goursat*>( obj.get() ) )
        {
            if ( auto* f = findField( "ka" ) )
                f->input->setString( fmt2( g->getKa() ) );
            if ( auto* f = findField( "kb" ) )
                f->input->setString( fmt2( g->getKb() ) );
        }
    }

    void
    prefillSpecificDefaults() override
    {
        if ( auto* f = findField( "ka" ) )
            f->input->setString( "1.00" );
        if ( auto* f = findField( "kb" ) )
            f->input->setString( "1.00" );
    }

    bool
    applySpecific( const CommonFields& common ) override
    {
        auto ka = parse( findField( "ka" ), []( double v ) { return v > 0; } );
        auto kb = parse( findField( "kb" ), []( double v ) { return v > 0; } );
        if ( !ka || !kb )
            return false;

        if ( mode_ == Mode::Create )
        {
            scene_manager_.addGoursat( common.material, common.pos, *ka, *kb );
            auto* created = scene_manager_.getObjects().back().get();
            created->setDisplayName( common.name );
            scene_manager_.setTargetObj( created );
        } else if ( obj_idx_.has_value() )
        {
            scene_manager_.getObjects()[*obj_idx_] =
                std::make_unique<model::Goursat>( common.material,
                                                  common.pos,
                                                  static_cast<float>( *ka ),
                                                  static_cast<float>( *kb ) );
            scene_manager_.setTargetObj( scene_manager_.getObjects()[*obj_idx_].get() );
            scene_manager_.getObjects()[*obj_idx_]->setDisplayName( common.name );
        }
        scene_manager_.needUpdate() = true;
        if ( close_cb_ )
            close_cb_();
        return true;
    }

  private:
    CloseCb close_cb_;
};

class RoundedBoxParamsDialog : public OpticalObjParamsDialog {
  public:
    RoundedBoxParamsDialog( hui::WindowManager*       wm,
                            float                     x,
                            float                     y,
                            float                     w,
                            float                     h,
                            rta::model::SceneManager& scene_manager,
                            CloseCb                   close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Add RoundedBox",
                                  scene_manager,
                                  Mode::Create,
                                  std::nullopt,
                                  { { "Half size X", "p1" },
                                    { "Half size Y", "p2" },
                                    { "Half size Z", "p3" },
                                    { "Radius", "p4" } },
                                  close_cb ),
          close_cb_( std::move( close_cb ) )
    {
        prefillCommonDefaults();
        prefillSpecificDefaults();
    }

    RoundedBoxParamsDialog( hui::WindowManager*       wm,
                            float                     x,
                            float                     y,
                            float                     w,
                            float                     h,
                            rta::model::SceneManager& scene_manager,
                            size_t                    obj_idx,
                            CloseCb                   close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Edit RoundedBox",
                                  scene_manager,
                                  Mode::Edit,
                                  obj_idx,
                                  { { "Half size X", "p1" },
                                    { "Half size Y", "p2" },
                                    { "Half size Z", "p3" },
                                    { "Radius", "p4" } },
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
        if ( auto* rb = dynamic_cast<model::RoundedBox*>( obj.get() ) )
        {
            auto hs = rb->getHalfSize();
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( hs.x ) );
            if ( auto* f = findField( "p2" ) )
                f->input->setString( fmt2( hs.y ) );
            if ( auto* f = findField( "p3" ) )
                f->input->setString( fmt2( hs.z ) );
            if ( auto* f = findField( "p4" ) )
                f->input->setString( fmt2( rb->getRadius() ) );
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
        if ( auto* f = findField( "p4" ) )
            f->input->setString( "0.20" );
    }

    bool
    applySpecific( const CommonFields& common ) override
    {
        auto p1 = parse( findField( "p1" ), []( double v ) { return v > 0; } );
        auto p2 = parse( findField( "p2" ), []( double v ) { return v > 0; } );
        auto p3 = parse( findField( "p3" ), []( double v ) { return v > 0; } );
        auto p4 = parse( findField( "p4" ), []( double v ) { return v > 0; } );
        if ( !p1 || !p2 || !p3 || !p4 )
            return false;

        auto make_obj = [&]() {
            return std::make_unique<model::RoundedBox>( common.material,
                                                        common.pos,
                                                        model::Vector3f( *p1, *p2, *p3 ),
                                                        static_cast<float>( *p4 ) );
        };

        if ( mode_ == Mode::Create )
        {
            scene_manager_.getObjects().push_back( make_obj() );
            scene_manager_.getObjects().back()->setDisplayName( common.name );
            scene_manager_.setTargetObj( scene_manager_.getObjects().back().get() );
        } else if ( obj_idx_.has_value() )
        {
            scene_manager_.getObjects()[*obj_idx_] = make_obj();
            scene_manager_.getObjects()[*obj_idx_]->setDisplayName( common.name );
            scene_manager_.setTargetObj( scene_manager_.getObjects()[*obj_idx_].get() );
        }
        scene_manager_.needUpdate() = true;
        if ( close_cb_ )
            close_cb_();
        return true;
    }

  private:
    CloseCb close_cb_;
};

class EllipsoidParamsDialog : public OpticalObjParamsDialog {
  public:
    EllipsoidParamsDialog( hui::WindowManager*       wm,
                           float                     x,
                           float                     y,
                           float                     w,
                           float                     h,
                           rta::model::SceneManager& scene_manager,
                           CloseCb                   close_cb )
        : OpticalObjParamsDialog(
              wm,
              x,
              y,
              w,
              h,
              "Add Ellipsoid",
              scene_manager,
              Mode::Create,
              std::nullopt,
              { { "Radius X", "p1" }, { "Radius Y", "p2" }, { "Radius Z", "p3" } },
              close_cb ),
          close_cb_( std::move( close_cb ) )
    {
        prefillCommonDefaults();
        prefillSpecificDefaults();
    }

    EllipsoidParamsDialog( hui::WindowManager*       wm,
                           float                     x,
                           float                     y,
                           float                     w,
                           float                     h,
                           rta::model::SceneManager& scene_manager,
                           size_t                    obj_idx,
                           CloseCb                   close_cb )
        : OpticalObjParamsDialog(
              wm,
              x,
              y,
              w,
              h,
              "Edit Ellipsoid",
              scene_manager,
              Mode::Edit,
              obj_idx,
              { { "Radius X", "p1" }, { "Radius Y", "p2" }, { "Radius Z", "p3" } },
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
        if ( auto* e = dynamic_cast<model::Ellipsoid*>( obj.get() ) )
        {
            auto r = e->getRadii();
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( r.x ) );
            if ( auto* f = findField( "p2" ) )
                f->input->setString( fmt2( r.y ) );
            if ( auto* f = findField( "p3" ) )
                f->input->setString( fmt2( r.z ) );
        }
    }

    void
    prefillSpecificDefaults() override
    {
        if ( auto* f = findField( "p1" ) )
            f->input->setString( "1.00" );
        if ( auto* f = findField( "p2" ) )
            f->input->setString( "1.50" );
        if ( auto* f = findField( "p3" ) )
            f->input->setString( "0.75" );
    }

    bool
    applySpecific( const CommonFields& common ) override
    {
        auto p1 = parse( findField( "p1" ), []( double v ) { return v > 0; } );
        auto p2 = parse( findField( "p2" ), []( double v ) { return v > 0; } );
        auto p3 = parse( findField( "p3" ), []( double v ) { return v > 0; } );
        if ( !p1 || !p2 || !p3 )
            return false;

        auto make_obj = [&]() {
            return std::make_unique<model::Ellipsoid>( common.material,
                                                       common.pos,
                                                       model::Vector3f( *p1, *p2, *p3 ) );
        };

        if ( mode_ == Mode::Create )
        {
            scene_manager_.getObjects().push_back( make_obj() );
            scene_manager_.getObjects().back()->setDisplayName( common.name );
            scene_manager_.setTargetObj( scene_manager_.getObjects().back().get() );
        } else if ( obj_idx_.has_value() )
        {
            scene_manager_.getObjects()[*obj_idx_] = make_obj();
            scene_manager_.getObjects()[*obj_idx_]->setDisplayName( common.name );
            scene_manager_.setTargetObj( scene_manager_.getObjects()[*obj_idx_].get() );
        }
        scene_manager_.needUpdate() = true;
        if ( close_cb_ )
            close_cb_();
        return true;
    }

  private:
    CloseCb close_cb_;
};

class CapsuleParamsDialog : public OpticalObjParamsDialog {
  public:
    CapsuleParamsDialog( hui::WindowManager*       wm,
                         float                     x,
                         float                     y,
                         float                     w,
                         float                     h,
                         rta::model::SceneManager& scene_manager,
                         CloseCb                   close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Add Capsule",
                                  scene_manager,
                                  Mode::Create,
                                  std::nullopt,
                                  { { "Height", "p1" }, { "Radius", "p2" } },
                                  close_cb ),
          close_cb_( std::move( close_cb ) )
    {
        prefillCommonDefaults();
        prefillSpecificDefaults();
    }

    CapsuleParamsDialog( hui::WindowManager*       wm,
                         float                     x,
                         float                     y,
                         float                     w,
                         float                     h,
                         rta::model::SceneManager& scene_manager,
                         size_t                    obj_idx,
                         CloseCb                   close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Edit Capsule",
                                  scene_manager,
                                  Mode::Edit,
                                  obj_idx,
                                  { { "Height", "p1" }, { "Radius", "p2" } },
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
        if ( auto* c = dynamic_cast<model::Capsule*>( obj.get() ) )
        {
            auto  pa = c->getPaLocal();
            auto  pb = c->getPbLocal();
            float h  = std::abs( ( pb - pa ).y );
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( h ) );
            if ( auto* f = findField( "p2" ) )
                f->input->setString( fmt2( c->getRadius() ) );
        }
    }

    void
    prefillSpecificDefaults() override
    {
        if ( auto* f = findField( "p1" ) )
            f->input->setString( "2.00" );
        if ( auto* f = findField( "p2" ) )
            f->input->setString( "0.50" );
    }

    bool
    applySpecific( const CommonFields& common ) override
    {
        auto h = parse( findField( "p1" ), []( double v ) { return v > 0; } );
        auto r = parse( findField( "p2" ), []( double v ) { return v > 0; } );
        if ( !h || !r )
            return false;

        auto make_obj = [&]() {
            return std::make_unique<model::Capsule>(
                common.material,
                common.pos,
                model::Vector3f{ 0.0f, -0.5f * static_cast<float>( *h ), 0.0f },
                model::Vector3f{ 0.0f, 0.5f * static_cast<float>( *h ), 0.0f },
                static_cast<float>( *r ) );
        };

        if ( mode_ == Mode::Create )
        {
            scene_manager_.getObjects().push_back( make_obj() );
            scene_manager_.getObjects().back()->setDisplayName( common.name );
            scene_manager_.setTargetObj( scene_manager_.getObjects().back().get() );
        } else if ( obj_idx_.has_value() )
        {
            scene_manager_.getObjects()[*obj_idx_] = make_obj();
            scene_manager_.getObjects()[*obj_idx_]->setDisplayName( common.name );
            scene_manager_.setTargetObj( scene_manager_.getObjects()[*obj_idx_].get() );
        }
        scene_manager_.needUpdate() = true;
        if ( close_cb_ )
            close_cb_();
        return true;
    }

  private:
    CloseCb close_cb_;
};

class RoundedConeParamsDialog : public OpticalObjParamsDialog {
  public:
    RoundedConeParamsDialog( hui::WindowManager*       wm,
                             float                     x,
                             float                     y,
                             float                     w,
                             float                     h,
                             rta::model::SceneManager& scene_manager,
                             CloseCb                   close_cb )
        : OpticalObjParamsDialog(
              wm,
              x,
              y,
              w,
              h,
              "Add RoundedCone",
              scene_manager,
              Mode::Create,
              std::nullopt,
              { { "Height", "p1" }, { "Radius A", "p2" }, { "Radius B", "p3" } },
              close_cb ),
          close_cb_( std::move( close_cb ) )
    {
        prefillCommonDefaults();
        prefillSpecificDefaults();
    }

    RoundedConeParamsDialog( hui::WindowManager*       wm,
                             float                     x,
                             float                     y,
                             float                     w,
                             float                     h,
                             rta::model::SceneManager& scene_manager,
                             size_t                    obj_idx,
                             CloseCb                   close_cb )
        : OpticalObjParamsDialog(
              wm,
              x,
              y,
              w,
              h,
              "Edit RoundedCone",
              scene_manager,
              Mode::Edit,
              obj_idx,
              { { "Height", "p1" }, { "Radius A", "p2" }, { "Radius B", "p3" } },
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
        if ( auto* rc = dynamic_cast<model::RoundedCone*>( obj.get() ) )
        {
            auto  pa = rc->getPaLocal();
            auto  pb = rc->getPbLocal();
            float h  = std::abs( ( pb - pa ).y );
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( h ) );
            if ( auto* f = findField( "p2" ) )
                f->input->setString( fmt2( rc->getRadiusA() ) );
            if ( auto* f = findField( "p3" ) )
                f->input->setString( fmt2( rc->getRadiusB() ) );
        }
    }

    void
    prefillSpecificDefaults() override
    {
        if ( auto* f = findField( "p1" ) )
            f->input->setString( "2.00" );
        if ( auto* f = findField( "p2" ) )
            f->input->setString( "1.00" );
        if ( auto* f = findField( "p3" ) )
            f->input->setString( "0.50" );
    }

    bool
    applySpecific( const CommonFields& common ) override
    {
        auto h  = parse( findField( "p1" ), []( double v ) { return v > 0; } );
        auto ra = parse( findField( "p2" ), []( double v ) { return v > 0; } );
        auto rb = parse( findField( "p3" ), []( double v ) { return v > 0; } );
        if ( !h || !ra || !rb )
            return false;

        auto make_obj = [&]() {
            return std::make_unique<model::RoundedCone>(
                common.material,
                common.pos,
                model::Vector3f{ 0.0f, -0.5f * static_cast<float>( *h ), 0.0f },
                model::Vector3f{ 0.0f, 0.5f * static_cast<float>( *h ), 0.0f },
                static_cast<float>( *ra ),
                static_cast<float>( *rb ) );
        };

        if ( mode_ == Mode::Create )
        {
            scene_manager_.getObjects().push_back( make_obj() );
            scene_manager_.getObjects().back()->setDisplayName( common.name );
            scene_manager_.setTargetObj( scene_manager_.getObjects().back().get() );
        } else if ( obj_idx_.has_value() )
        {
            scene_manager_.getObjects()[*obj_idx_] = make_obj();
            scene_manager_.getObjects()[*obj_idx_]->setDisplayName( common.name );
            scene_manager_.setTargetObj( scene_manager_.getObjects()[*obj_idx_].get() );
        }
        scene_manager_.needUpdate() = true;
        if ( close_cb_ )
            close_cb_();
        return true;
    }

  private:
    CloseCb close_cb_;
};

class CappedConeParamsDialog : public OpticalObjParamsDialog {
  public:
    CappedConeParamsDialog( hui::WindowManager*       wm,
                            float                     x,
                            float                     y,
                            float                     w,
                            float                     h,
                            rta::model::SceneManager& scene_manager,
                            CloseCb                   close_cb )
        : OpticalObjParamsDialog(
              wm,
              x,
              y,
              w,
              h,
              "Add CappedCone",
              scene_manager,
              Mode::Create,
              std::nullopt,
              { { "Height", "p1" }, { "Radius A", "p2" }, { "Radius B", "p3" } },
              close_cb ),
          close_cb_( std::move( close_cb ) )
    {
        prefillCommonDefaults();
        prefillSpecificDefaults();
    }

    CappedConeParamsDialog( hui::WindowManager*       wm,
                            float                     x,
                            float                     y,
                            float                     w,
                            float                     h,
                            rta::model::SceneManager& scene_manager,
                            size_t                    obj_idx,
                            CloseCb                   close_cb )
        : OpticalObjParamsDialog(
              wm,
              x,
              y,
              w,
              h,
              "Edit CappedCone",
              scene_manager,
              Mode::Edit,
              obj_idx,
              { { "Height", "p1" }, { "Radius A", "p2" }, { "Radius B", "p3" } },
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
        if ( auto* cc = dynamic_cast<model::CappedCone*>( obj.get() ) )
        {
            auto  pa = cc->getPaLocal();
            auto  pb = cc->getPbLocal();
            float h  = std::abs( ( pb - pa ).y );
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( h ) );
            if ( auto* f = findField( "p2" ) )
                f->input->setString( fmt2( cc->getRadiusA() ) );
            if ( auto* f = findField( "p3" ) )
                f->input->setString( fmt2( cc->getRadiusB() ) );
        }
    }

    void
    prefillSpecificDefaults() override
    {
        if ( auto* f = findField( "p1" ) )
            f->input->setString( "2.00" );
        if ( auto* f = findField( "p2" ) )
            f->input->setString( "1.00" );
        if ( auto* f = findField( "p3" ) )
            f->input->setString( "0.50" );
    }

    bool
    applySpecific( const CommonFields& common ) override
    {
        auto h  = parse( findField( "p1" ), []( double v ) { return v > 0; } );
        auto ra = parse( findField( "p2" ), []( double v ) { return v > 0; } );
        auto rb = parse( findField( "p3" ), []( double v ) { return v > 0; } );
        if ( !h || !ra || !rb )
            return false;

        auto make_obj = [&]() {
            return std::make_unique<model::CappedCone>(
                common.material,
                common.pos,
                model::Vector3f{ 0.0f, -0.5f * static_cast<float>( *h ), 0.0f },
                model::Vector3f{ 0.0f, 0.5f * static_cast<float>( *h ), 0.0f },
                static_cast<float>( *ra ),
                static_cast<float>( *rb ) );
        };

        if ( mode_ == Mode::Create )
        {
            scene_manager_.getObjects().push_back( make_obj() );
            scene_manager_.getObjects().back()->setDisplayName( common.name );
            scene_manager_.setTargetObj( scene_manager_.getObjects().back().get() );
        } else if ( obj_idx_.has_value() )
        {
            scene_manager_.getObjects()[*obj_idx_] = make_obj();
            scene_manager_.getObjects()[*obj_idx_]->setDisplayName( common.name );
            scene_manager_.setTargetObj( scene_manager_.getObjects()[*obj_idx_].get() );
        }
        scene_manager_.needUpdate() = true;
        if ( close_cb_ )
            close_cb_();
        return true;
    }

  private:
    CloseCb close_cb_;
};

class CappedCylinderParamsDialog : public OpticalObjParamsDialog {
  public:
    CappedCylinderParamsDialog( hui::WindowManager*       wm,
                                float                     x,
                                float                     y,
                                float                     w,
                                float                     h,
                                rta::model::SceneManager& scene_manager,
                                CloseCb                   close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Add CappedCylinder",
                                  scene_manager,
                                  Mode::Create,
                                  std::nullopt,
                                  { { "Height", "p1" }, { "Radius", "p2" } },
                                  close_cb ),
          close_cb_( std::move( close_cb ) )
    {
        prefillCommonDefaults();
        prefillSpecificDefaults();
    }

    CappedCylinderParamsDialog( hui::WindowManager*       wm,
                                float                     x,
                                float                     y,
                                float                     w,
                                float                     h,
                                rta::model::SceneManager& scene_manager,
                                size_t                    obj_idx,
                                CloseCb                   close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Edit CappedCylinder",
                                  scene_manager,
                                  Mode::Edit,
                                  obj_idx,
                                  { { "Height", "p1" }, { "Radius", "p2" } },
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
        if ( auto* cc = dynamic_cast<model::CappedCylinder*>( obj.get() ) )
        {
            auto  a = cc->getALocal();
            auto  b = cc->getBLocal();
            float h = std::abs( ( b - a ).y );
            if ( auto* f = findField( "p1" ) )
                f->input->setString( fmt2( h ) );
            if ( auto* f = findField( "p2" ) )
                f->input->setString( fmt2( cc->getRadius() ) );
        }
    }

    void
    prefillSpecificDefaults() override
    {
        if ( auto* f = findField( "p1" ) )
            f->input->setString( "2.00" );
        if ( auto* f = findField( "p2" ) )
            f->input->setString( "0.50" );
    }

    bool
    applySpecific( const CommonFields& common ) override
    {
        auto h = parse( findField( "p1" ), []( double v ) { return v > 0; } );
        auto r = parse( findField( "p2" ), []( double v ) { return v > 0; } );
        if ( !h || !r )
            return false;

        auto make_obj = [&]() {
            return std::make_unique<model::CappedCylinder>(
                common.material,
                common.pos,
                model::Vector3f{ 0.0f, -0.5f * static_cast<float>( *h ), 0.0f },
                model::Vector3f{ 0.0f, 0.5f * static_cast<float>( *h ), 0.0f },
                static_cast<float>( *r ) );
        };

        if ( mode_ == Mode::Create )
        {
            scene_manager_.getObjects().push_back( make_obj() );
            scene_manager_.getObjects().back()->setDisplayName( common.name );
            scene_manager_.setTargetObj( scene_manager_.getObjects().back().get() );
        } else if ( obj_idx_.has_value() )
        {
            scene_manager_.getObjects()[*obj_idx_] = make_obj();
            scene_manager_.getObjects()[*obj_idx_]->setDisplayName( common.name );
            scene_manager_.setTargetObj( scene_manager_.getObjects()[*obj_idx_].get() );
        }
        scene_manager_.needUpdate() = true;
        if ( close_cb_ )
            close_cb_();
        return true;
    }

  private:
    CloseCb close_cb_;
};

class WedgeParamsDialog : public OpticalObjParamsDialog {
  public:
    WedgeParamsDialog( hui::WindowManager*       wm,
                       float                     x,
                       float                     y,
                       float                     w,
                       float                     h,
                       rta::model::SceneManager& scene_manager,
                       CloseCb                   close_cb )
        : OpticalObjParamsDialog(
              wm,
              x,
              y,
              w,
              h,
              "Add Wedge",
              scene_manager,
              Mode::Create,
              std::nullopt,
              { { "Half size X", "p1" }, { "Half size Y", "p2" }, { "Half size Z", "p3" } },
              close_cb ),
          close_cb_( std::move( close_cb ) )
    {
        prefillCommonDefaults();
        prefillSpecificDefaults();
    }

    WedgeParamsDialog( hui::WindowManager*       wm,
                       float                     x,
                       float                     y,
                       float                     w,
                       float                     h,
                       rta::model::SceneManager& scene_manager,
                       size_t                    obj_idx,
                       CloseCb                   close_cb )
        : OpticalObjParamsDialog(
              wm,
              x,
              y,
              w,
              h,
              "Edit Wedge",
              scene_manager,
              Mode::Edit,
              obj_idx,
              { { "Half size X", "p1" }, { "Half size Y", "p2" }, { "Half size Z", "p3" } },
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
        if ( auto* w = dynamic_cast<model::Wedge*>( obj.get() ) )
        {
            auto hs = w->getS();
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

        auto make_obj = [&]() {
            return std::make_unique<model::Wedge>( common.material,
                                                   common.pos,
                                                   model::Vector3f( *p1, *p2, *p3 ) );
        };

        if ( mode_ == Mode::Create )
        {
            scene_manager_.getObjects().push_back( make_obj() );
            scene_manager_.getObjects().back()->setDisplayName( common.name );
            scene_manager_.setTargetObj( scene_manager_.getObjects().back().get() );
        } else if ( obj_idx_.has_value() )
        {
            scene_manager_.getObjects()[*obj_idx_] = make_obj();
            scene_manager_.getObjects()[*obj_idx_]->setDisplayName( common.name );
            scene_manager_.setTargetObj( scene_manager_.getObjects()[*obj_idx_].get() );
        }
        scene_manager_.needUpdate() = true;
        if ( close_cb_ )
            close_cb_();
        return true;
    }

  private:
    CloseCb close_cb_;
};

class EllipseParamsDialog : public OpticalObjParamsDialog {
  public:
    EllipseParamsDialog( hui::WindowManager*       wm,
                         float                     x,
                         float                     y,
                         float                     w,
                         float                     h,
                         rta::model::SceneManager& scene_manager,
                         CloseCb                   close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Add Ellipse",
                                  scene_manager,
                                  Mode::Create,
                                  std::nullopt,
                                  { { "U.x", "ux" },
                                    { "U.y", "uy" },
                                    { "U.z", "uz" },
                                    { "V.x", "vx" },
                                    { "V.y", "vy" },
                                    { "V.z", "vz" } },
                                  close_cb ),
          close_cb_( std::move( close_cb ) )
    {
        prefillCommonDefaults();
        prefillSpecificDefaults();
    }

    EllipseParamsDialog( hui::WindowManager*       wm,
                         float                     x,
                         float                     y,
                         float                     w,
                         float                     h,
                         rta::model::SceneManager& scene_manager,
                         size_t                    obj_idx,
                         CloseCb                   close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Edit Ellipse",
                                  scene_manager,
                                  Mode::Edit,
                                  obj_idx,
                                  { { "U.x", "ux" },
                                    { "U.y", "uy" },
                                    { "U.z", "uz" },
                                    { "V.x", "vx" },
                                    { "V.y", "vy" },
                                    { "V.z", "vz" } },
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
        if ( auto* e = dynamic_cast<model::Ellipse*>( obj.get() ) )
        {
            auto u = e->getU();
            auto v = e->getV();
            if ( auto* f = findField( "ux" ) )
                f->input->setString( fmt2( u.x ) );
            if ( auto* f = findField( "uy" ) )
                f->input->setString( fmt2( u.y ) );
            if ( auto* f = findField( "uz" ) )
                f->input->setString( fmt2( u.z ) );
            if ( auto* f = findField( "vx" ) )
                f->input->setString( fmt2( v.x ) );
            if ( auto* f = findField( "vy" ) )
                f->input->setString( fmt2( v.y ) );
            if ( auto* f = findField( "vz" ) )
                f->input->setString( fmt2( v.z ) );
        }
    }

    void
    prefillSpecificDefaults() override
    {
        if ( auto* f = findField( "ux" ) )
            f->input->setString( "1.00" );
        if ( auto* f = findField( "uy" ) )
            f->input->setString( "0.00" );
        if ( auto* f = findField( "uz" ) )
            f->input->setString( "0.00" );

        if ( auto* f = findField( "vx" ) )
            f->input->setString( "0.00" );
        if ( auto* f = findField( "vy" ) )
            f->input->setString( "1.00" );
        if ( auto* f = findField( "vz" ) )
            f->input->setString( "0.00" );
    }

    bool
    applySpecific( const CommonFields& common ) override
    {
        auto ux = parse( findField( "ux" ), []( double ) { return true; } );
        auto uy = parse( findField( "uy" ), []( double ) { return true; } );
        auto uz = parse( findField( "uz" ), []( double ) { return true; } );
        auto vx = parse( findField( "vx" ), []( double ) { return true; } );
        auto vy = parse( findField( "vy" ), []( double ) { return true; } );
        auto vz = parse( findField( "vz" ), []( double ) { return true; } );
        if ( !ux || !uy || !uz || !vx || !vy || !vz )
            return false;

        auto make_obj = [&]() {
            return std::make_unique<model::Ellipse>( common.material,
                                                     common.pos,
                                                     model::Vector3f( *ux, *uy, *uz ),
                                                     model::Vector3f( *vx, *vy, *vz ) );
        };

        if ( mode_ == Mode::Create )
        {
            scene_manager_.getObjects().push_back( make_obj() );
            scene_manager_.getObjects().back()->setDisplayName( common.name );
            scene_manager_.setTargetObj( scene_manager_.getObjects().back().get() );
        } else if ( obj_idx_.has_value() )
        {
            scene_manager_.getObjects()[*obj_idx_] = make_obj();
            scene_manager_.getObjects()[*obj_idx_]->setDisplayName( common.name );
            scene_manager_.setTargetObj( scene_manager_.getObjects()[*obj_idx_].get() );
        }
        scene_manager_.needUpdate() = true;
        if ( close_cb_ )
            close_cb_();
        return true;
    }

  private:
    CloseCb close_cb_;
};

class TriangleParamsDialog : public OpticalObjParamsDialog {
  public:
    TriangleParamsDialog( hui::WindowManager*       wm,
                          float                     x,
                          float                     y,
                          float                     w,
                          float                     h,
                          rta::model::SceneManager& scene_manager,
                          CloseCb                   close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Add Triangle",
                                  scene_manager,
                                  Mode::Create,
                                  std::nullopt,
                                  { { "V0.x", "v0x" },
                                    { "V0.y", "v0y" },
                                    { "V0.z", "v0z" },
                                    { "V1.x", "v1x" },
                                    { "V1.y", "v1y" },
                                    { "V1.z", "v1z" },
                                    { "V2.x", "v2x" },
                                    { "V2.y", "v2y" },
                                    { "V2.z", "v2z" } },
                                  close_cb ),
          close_cb_( std::move( close_cb ) )
    {
        prefillCommonDefaults();
        prefillSpecificDefaults();
    }

    TriangleParamsDialog( hui::WindowManager*       wm,
                          float                     x,
                          float                     y,
                          float                     w,
                          float                     h,
                          rta::model::SceneManager& scene_manager,
                          size_t                    obj_idx,
                          CloseCb                   close_cb )
        : OpticalObjParamsDialog( wm,
                                  x,
                                  y,
                                  w,
                                  h,
                                  "Edit Triangle",
                                  scene_manager,
                                  Mode::Edit,
                                  obj_idx,
                                  { { "V0.x", "v0x" },
                                    { "V0.y", "v0y" },
                                    { "V0.z", "v0z" },
                                    { "V1.x", "v1x" },
                                    { "V1.y", "v1y" },
                                    { "V1.z", "v1z" },
                                    { "V2.x", "v2x" },
                                    { "V2.y", "v2y" },
                                    { "V2.z", "v2z" } },
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
        if ( auto* t = dynamic_cast<model::Triangle*>( obj.get() ) )
        {
            auto v0  = t->getV0();
            auto v1  = t->getV1();
            auto v2  = t->getV2();
            auto set = [&]( const char* key, const model::Vector3f& v, int idx ) {
                if ( auto* f = findField( key ) )
                {
                    float val = idx == 0 ? v.x : ( idx == 1 ? v.y : v.z );
                    f->input->setString( fmt2( val ) );
                }
            };
            set( "v0x", v0, 0 );
            set( "v0y", v0, 1 );
            set( "v0z", v0, 2 );
            set( "v1x", v1, 0 );
            set( "v1y", v1, 1 );
            set( "v1z", v1, 2 );
            set( "v2x", v2, 0 );
            set( "v2y", v2, 1 );
            set( "v2z", v2, 2 );
        }
    }

    void
    prefillSpecificDefaults() override
    {
        // Simple triangle on XY plane
        std::vector<std::pair<const char*, const char*>> defaults = { { "v0x", "0.0" },
                                                                      { "v0y", "0.0" },
                                                                      { "v0z", "0.0" },
                                                                      { "v1x", "1.0" },
                                                                      { "v1y", "0.0" },
                                                                      { "v1z", "0.0" },
                                                                      { "v2x", "0.0" },
                                                                      { "v2y", "1.0" },
                                                                      { "v2z", "0.0" } };
        for ( auto& p : defaults )
        {
            if ( auto* f = findField( p.first ) )
                f->input->setString( p.second );
        }
    }

    bool
    applySpecific( const CommonFields& common ) override
    {
        auto fetch = [&]( const char* key ) {
            return parse( findField( key ), []( double ) { return true; } );
        };
        auto v0x = fetch( "v0x" ), v0y = fetch( "v0y" ), v0z = fetch( "v0z" );
        auto v1x = fetch( "v1x" ), v1y = fetch( "v1y" ), v1z = fetch( "v1z" );
        auto v2x = fetch( "v2x" ), v2y = fetch( "v2y" ), v2z = fetch( "v2z" );

        if ( !v0x || !v0y || !v0z || !v1x || !v1y || !v1z || !v2x || !v2y || !v2z )
            return false;

        auto make_obj = [&]() {
            return std::make_unique<model::Triangle>( common.material,
                                                      model::Vector3f( *v0x, *v0y, *v0z ),
                                                      model::Vector3f( *v1x, *v1y, *v1z ),
                                                      model::Vector3f( *v2x, *v2y, *v2z ) );
        };

        if ( mode_ == Mode::Create )
        {
            scene_manager_.getObjects().push_back( make_obj() );
            scene_manager_.getObjects().back()->setDisplayName( common.name );
            scene_manager_.setTargetObj( scene_manager_.getObjects().back().get() );
        } else if ( obj_idx_.has_value() )
        {
            scene_manager_.getObjects()[*obj_idx_] = make_obj();
            scene_manager_.getObjects()[*obj_idx_]->setDisplayName( common.name );
            scene_manager_.setTargetObj( scene_manager_.getObjects()[*obj_idx_].get() );
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
} // namespace rta
