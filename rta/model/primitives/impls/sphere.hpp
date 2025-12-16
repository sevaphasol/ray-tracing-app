#pragma once

#include "rta/model/primitives/primitive.hpp"
#include "rta/model/rendering/ray.hpp"
#include "rta/model/rendering/vector3.hpp"
#include <optional>

namespace rta {
namespace model {

class Sphere : public Primitive {
  public:
    Sphere( const Material& material, const Vector3f& center, float radius );

    virtual std::optional<Primitive::IntersectionInfo>
    calcRayIntersection( const Ray& ray ) const override final;

    virtual Vector3f
    calcNormal( const Vector3f& point, bool inside_object ) const override final;

    std::unique_ptr<Primitive>
    clone() const override
    {
        return std::make_unique<Sphere>( *this );
    }

    const char*
    getName() const override final
    {
        return "Sphere";
    }
    PrimitiveCode
    getCode() const override final
    {
        return PrimitiveCode::Sphere;
    }

    std::array<Vector3f, 8>
    getCircumscribedAABB() const override final;

    float
    getRadius() const
    {
        return radius_;
    }
    void
    setRadius( float radius )
    {
        radius_    = radius;
        radius_sq_ = radius * radius;
    }

  private:
    float radius_;
    float radius_sq_;
};

} // namespace model
} // namespace rta
