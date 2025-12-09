#pragma once

#include "zemax/model/primitives/primitive.hpp"
#include "zemax/model/rendering/ray.hpp"
#include "zemax/model/rendering/vector3.hpp"

#include <memory>
#include <optional>

namespace zemax {
namespace model {

class Ellipsoid : public Primitive {
  public:
    Ellipsoid( const Material& material, const Vector3f& center, const Vector3f& radii );

    std::optional<Primitive::IntersectionInfo>
    calcRayIntersection( const Ray& ray ) const override final;

    Vector3f
    calcNormal( const Vector3f& point, bool inside_object ) const override final;

    std::unique_ptr<Primitive>
    clone() const override
    {
        return std::make_unique<Ellipsoid>( *this );
    }

    const char*
    getName() const override final
    {
        return "Ellipsoid";
    }

    std::array<Vector3f, 8>
    getCircumscribedAABB() const override final;

    const Vector3f&
    getRadii() const
    {
        return radii_;
    }

    void
    setRadii( const Vector3f& r )
    {
        radii_ = r;
    }

  private:
    Vector3f radii_;
};

} // namespace model
} // namespace zemax
