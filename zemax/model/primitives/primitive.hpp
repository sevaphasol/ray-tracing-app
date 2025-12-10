#pragma once

#include "zemax/model/primitives/material.hpp"
#include "zemax/model/rendering/ray.hpp"
#include "zemax/model/rendering/vector2.hpp"
#include "zemax/model/rendering/vector3.hpp"
#include <array>
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace zemax {
namespace model {

class Primitive {
  public:
    virtual ~Primitive() = default;
    Primitive( const Material& material, const Vector3f& origin )
        : painted_( false ),
          origin_( origin ),
          axis_x_( 1.0f, 0.0f, 0.0f ),
          axis_y_( 0.0f, 1.0f, 0.0f ),
          axis_z_( 0.0f, 0.0f, 1.0f ),
          material_( material )
    {
    }

    virtual const char*
    getName() const = 0;

    void
    setDisplayName( const std::string& name )
    {
        display_name_ = name;
    }

    std::string
    getDisplayName() const
    {
        if ( display_name_.empty() )
        {
            return std::string( getName() );
        }
        return display_name_;
    }

    virtual std::unique_ptr<Primitive>
    clone() const = 0;

    void
    paint()
    {
        material_.painted = true;
    }

    void
    revert_paint()
    {
        material_.painted = false;
    }

    void
    setColor( Color color )
    {
        material_.color = color;
    }

    void
    setMaterial( Material material )
    {
        material_ = material;
    }

    Material
    getMaterial() const
    {
        return material_;
    }

    Vector3f
    getOrigin() const
    {
        return origin_;
    }

    void
    setOrigin( const Vector3f& new_origin )
    {
        origin_ = new_origin;
    }

    virtual void
    move( const Vector3f& delta )
    {
        origin_ += delta;
    }

    struct IntersectionInfo
    {
        float                   close_distance;
        float                   far_distance;
        bool                    inside_object;
        std::optional<Vector3f> normal;
    };

    virtual std::optional<Primitive::IntersectionInfo>
    calcRayIntersection( const Ray& ray ) const = 0;

    virtual Vector3f
    calcNormal( const Vector3f& point, bool inside_object ) const = 0;

    virtual std::array<Vector3f, 8>
    getCircumscribedAABB() const = 0;

    // Orientation helpers
    const Vector3f&
    getAxisX() const
    {
        return axis_x_;
    }

    const Vector3f&
    getAxisY() const
    {
        return axis_y_;
    }

    const Vector3f&
    getAxisZ() const
    {
        return axis_z_;
    }

    void
    setOrientation( const Vector3f& x, const Vector3f& y, const Vector3f& z )
    {
        // Simple Gram-Schmidt to keep basis orthonormal-ish.
        axis_x_ = x;
        if ( axis_x_.getLenSq() > 0.0f )
        {
            axis_x_.normalize();
        } else
        {
            axis_x_ = { 1.0f, 0.0f, 0.0f };
        }

        Vector3f y_ortho = y - axis_x_ * scalarMul( y, axis_x_ );
        if ( y_ortho.getLenSq() > 0.0f )
        {
            axis_y_ = y_ortho.normalize();
        } else
        {
            // pick arbitrary perpendicular if input degenerate
            axis_y_ = { 0.0f, 1.0f, 0.0f };
        }

        axis_z_ = cross( axis_x_, axis_y_ );
        float z_len_sq = axis_z_.getLenSq();
        if ( z_len_sq > 0.0f )
        {
            axis_z_ *= 1.0f / std::sqrt( z_len_sq );
        } else
        {
            axis_z_ = { 0.0f, 0.0f, 1.0f };
        }
    }

    void
    rotateAroundWorldAxis( const Vector3f& axis_world, float angle )
    {
        Vector3f axis = axis_world;
        float    len  = axis.getLen();
        if ( len == 0.0f )
        {
            return;
        }
        axis *= 1.0f / len;

        float c = std::cos( angle );
        float s = std::sin( angle );

        auto rotateVec = [&]( const Vector3f& v ) -> Vector3f {
            // Rodrigues' rotation formula
            return v * c + cross( axis, v ) * s + axis * ( scalarMul( axis, v ) * ( 1.0f - c ) );
        };

        axis_x_ = rotateVec( axis_x_ );
        axis_y_ = rotateVec( axis_y_ );
        axis_z_ = rotateVec( axis_z_ );
    }

    void
    rotateAroundLocalAxis( const Vector3f& axis_local, float angle )
    {
        rotateAroundWorldAxis( localToWorldDir( axis_local ), angle );
    }

  protected:
    Vector3f
    worldToLocalPoint( const Vector3f& p_world ) const
    {
        Vector3f v = p_world - origin_;
        return Vector3f( scalarMul( v, axis_x_ ), scalarMul( v, axis_y_ ), scalarMul( v, axis_z_ ) );
    }

    Vector3f
    worldToLocalDir( const Vector3f& d_world ) const
    {
        return Vector3f( scalarMul( d_world, axis_x_ ),
                         scalarMul( d_world, axis_y_ ),
                         scalarMul( d_world, axis_z_ ) );
    }

    Vector3f
    localToWorldPoint( const Vector3f& p_local ) const
    {
        return origin_ + axis_x_ * p_local.x + axis_y_ * p_local.y + axis_z_ * p_local.z;
    }

    Vector3f
    localToWorldDir( const Vector3f& d_local ) const
    {
        return axis_x_ * d_local.x + axis_y_ * d_local.y + axis_z_ * d_local.z;
    }

    Vector3f
    localToWorldNormal( const Vector3f& n_local ) const
    {
        Vector3f n = localToWorldDir( n_local );
        float    len = n.getLen();
        if ( len == 0.0f )
        {
            return n;
        }
        return n * ( 1.0f / len );
    }

  private:
    bool painted_;

    Vector3f origin_;
    Vector3f axis_x_{ 1.0f, 0.0f, 0.0f };
    Vector3f axis_y_{ 0.0f, 1.0f, 0.0f };
    Vector3f axis_z_{ 0.0f, 0.0f, 1.0f };
    Material material_;
    std::string display_name_;
};

} // namespace model
} // namespace zemax
