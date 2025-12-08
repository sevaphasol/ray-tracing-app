#pragma once

#include "zemax/model/primitives/material.hpp"
#include "zemax/model/rendering/ray.hpp"
#include "zemax/model/rendering/vector2.hpp"
#include "zemax/model/rendering/vector3.hpp"
#include <array>
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
        : material_( material ), origin_( origin )
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

  private:
    bool painted_;

    Vector3f origin_;
    Material material_;
    std::string display_name_;
};

} // namespace model
} // namespace zemax
