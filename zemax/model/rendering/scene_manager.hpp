#pragma once

#include "zemax/model/primitives/impls/aabb.hpp"
#include "zemax/model/primitives/impls/plane.hpp"
#include "zemax/model/primitives/impls/sphere.hpp"
#include "zemax/model/primitives/material.hpp"
#include "zemax/model/primitives/primitive.hpp"
#include "zemax/model/rendering/camera.hpp"
#include "zemax/model/rendering/color.hpp"
#include "zemax/model/rendering/light.hpp"
#include "zemax/model/rendering/ray.hpp"
#include "zemax/model/rendering/vector3.hpp"
#include <assert.h>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace zemax {
namespace model {

class SceneManager {
  public:
    SceneManager( const Vector3f& camera_pos, float screen_width, float screen_height );

    size_t
    getObjectsCount()
    {
        return objects_.size();
    }

    struct ObjectInfo
    {
        Material    material;
        Vector3f    pos;
        std::string type_name;
        size_t      objects_idx;
    };

    void
    paint( int idx )
    {
        if ( idx >= 0 && idx <= objects_.size() )
        {
            objects_[idx]->paint();
        }
    }

    void
    revert_paint( int idx )
    {
        if ( idx >= 0 && idx <= objects_.size() )
        {
            objects_[idx]->revert_paint();
        }
    }

    void
    copyTargetObj( float new_obj_x, float new_obj_y, float new_obj_z )
    {
        if ( target_obj_ == nullptr )
        {
            return;
        }

        int idx = -1;
        for ( int i = 0; i < objects_.size(); ++i )
        {
            if ( objects_[i].get() == target_obj_ )
            {
                idx = i;
                break;
            }
        }

        assert( idx != -1 );

        auto new_obj = target_obj_->clone();

        Vector3f new_pos{ new_obj_x, new_obj_y, new_obj_z };
        new_obj->setOrigin( new_pos );

        target_obj_->revert_paint();
        target_obj_ = new_obj.get();

        objects_.push_back( std::move( new_obj ) );

        need_update_ = true;
    }

    std::vector<std::unique_ptr<model::Primitive>>&
    getObjects()
    {
        return objects_;
    }

    ObjectInfo
    getObjectInfo( size_t idx )
    {
        auto* obj = objects_[idx].get();

        std::string obj_name;

        if ( typeid( *obj ).hash_code() == typeid( Sphere ).hash_code() )
        {
            obj_name = "Sphere";
        } else if ( typeid( *obj ).hash_code() == typeid( Plane ).hash_code() )
        {
            obj_name = "Plane";
        } else if ( typeid( *obj ).hash_code() == typeid( AABB ).hash_code() )
        {
            obj_name = "AABB";
        } else
        {
            assert( false );
        }

        return { .material    = obj->getMaterial(),
                 .pos         = obj->getOrigin(),
                 .type_name   = obj_name,
                 .objects_idx = idx };
    }

    Primitive*
    getIntersectedObj( uint px, uint py )
    {
        Ray ray = camera_.emitRay( px, py );

        IntersectionContext ctx( ray );

        if ( !findClosestIntersection( ctx ) )
        {
            return nullptr;
        }

        return ctx.closest_object;
    }

    bool&
    needUpdate()
    {
        return need_update_;
    }

    bool
    needUpdate() const
    {
        return need_update_;
    }

    void
    addLight( Vector3f pos,
              float    embedded_intensity,
              float    diffuse_intensity,
              float    glare_intensity );

    void
    deleteTargetObj()
    {
        for ( size_t i = 0; i < objects_.size(); ++i )
        {
            if ( objects_[i].get() == target_obj_ )
            {
                objects_.erase( objects_.begin() + i );
                target_obj_ = nullptr;
                return;
            }
        }
    }

    void
    addSphere( const Material& material, const Vector3f& center, float radius );

    void
    addPlane( const Material& material, const Vector3f& base_point, const Vector3f& normal );

    void
    addAABB( const Material& material, const Vector3f& center, const Vector3f& bounds );

    void
    addHexPrism( const Material& material, const Vector3f& center, float radius, float height );

    void
    addTorus( const Material& material,
              const Vector3f& center,
              float           minor_radius,
              float           major_radius );

    void
    addCube( const Material& material, const Vector3f& center, float side );

    Color
    calcPixelColor( uint row, uint col, const Color& background_color );

    Camera&
    getCamera()
    {
        return camera_;
    }

    const Camera&
    getCamera() const
    {
        return camera_;
    }

    Primitive*
    getTargetObj()
    {
        return target_obj_;
    }

    const Primitive*
    getTargetObj() const
    {
        return target_obj_;
    }

    void
    setTargetObj( Primitive* obj )
    {
        target_obj_ = obj;
    }

  private:
    struct IntersectionContext
    {
        IntersectionContext( const Ray& view_ray, const Color& background_color )
            : view_ray( view_ray ), background_color( background_color )
        {
        }

        IntersectionContext( const Ray& view_ray ) : view_ray( view_ray ) {}

        // IntersectionContext( const IntersectionContext& that )
        //     : background_color( that.background_color )
        // {
        //     this->view_ray           = that.view_ray;
        //     this->intersection_point = that.intersection_point;
        //     this->normal             = that.normal;
        //     this->closest_object     = that.closest_object;
        //     this->depth              = that.depth;
        // }

        IntersectionContext&
        operator=( const IntersectionContext& that )
        {
            this->view_ray           = that.view_ray;
            this->intersection_point = that.intersection_point;
            this->normal             = that.normal;
            this->background_color   = that.background_color;
            this->closest_object     = that.closest_object;
            this->depth              = that.depth;

            return *this;
        }

        Ray                     view_ray;
        Vector3f                intersection_point;
        Vector3f                normal;
        Color                   background_color;
        Primitive*              closest_object = nullptr;
        size_t                  depth          = 0;
        static constexpr size_t MaxDepth       = 4;
    };

  private:
    Color
    calcRayColor( IntersectionContext& ctx );

    void
    moveLights( const Vector3f& delta );

    Color
    calcRefractedColor( IntersectionContext& ctx );

    Color
    calcReflectedColor( IntersectionContext& ctx );

    Color
    calcColor( IntersectionContext& ctx );

    Color
    calcLightsColor( IntersectionContext& ctx );

    bool
    findClosestIntersection( IntersectionContext& ctx );

  private:
    bool need_update_ = true;

    Primitive*                              target_obj_ = nullptr;
    std::vector<std::unique_ptr<Primitive>> objects_;
    std::vector<Light>                      lights_;
    Camera                                  camera_;
};

} // namespace model
} // namespace zemax
