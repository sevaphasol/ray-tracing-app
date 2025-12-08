#pragma once

#include "zemax/model/primitives/primitive.hpp"
#include "zemax/model/rendering/ray.hpp"
#include "zemax/model/rendering/vector3.hpp"

namespace zemax {
namespace model {

class Plane : public Primitive {
  public:
    Plane( const Material& material, const Vector3f& base_point, const Vector3f& normal );

    virtual std::optional<Primitive::IntersectionInfo>
    calcRayIntersection( const Ray& ray ) const override final;

    virtual Vector3f
    calcNormal( const Vector3f& point, bool inside_object ) const override final;

    std::unique_ptr<Primitive>
    clone() const override
    {
        return std::make_unique<Plane>( *this );
    }

    const char*
    getName() const override final
    {
        return "Plane";
    }

    std::array<Vector3f, 8>
    getCircumscribedAABB() const override final;

  private:
    Vector3f normal_;
};

} // namespace model
} // namespace zemax
