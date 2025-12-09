#pragma once

#include "zemax/model/primitives/primitive.hpp"
#include "zemax/model/rendering/ray.hpp"
#include "zemax/model/rendering/vector3.hpp"

#include <memory>
#include <optional>

namespace zemax {
namespace model {

// Капсула: отрезок [pa, pb] в ЛОКАЛЬНЫХ координатах относительно origin + радиус.
class Capsule : public Primitive {
  public:
    Capsule( const Material& material,
             const Vector3f& center,
             const Vector3f& pa_local,
             const Vector3f& pb_local,
             float           radius );

    std::optional<Primitive::IntersectionInfo>
    calcRayIntersection( const Ray& ray ) const override final;

    Vector3f
    calcNormal( const Vector3f& point, bool inside_object ) const override final;

    std::unique_ptr<Primitive>
    clone() const override
    {
        return std::make_unique<Capsule>( *this );
    }

    const char*
    getName() const override final
    {
        return "Capsule";
    }

    std::array<Vector3f, 8>
    getCircumscribedAABB() const override final;

    const Vector3f&
    getPaLocal() const
    {
        return pa_local_;
    }

    const Vector3f&
    getPbLocal() const
    {
        return pb_local_;
    }

    float
    getRadius() const
    {
        return radius_;
    }

    void
    setPaLocal( const Vector3f& p )
    {
        pa_local_ = p;
    }

    void
    setPbLocal( const Vector3f& p )
    {
        pb_local_ = p;
    }

    void
    setRadius( float r )
    {
        radius_ = r;
    }

  private:
    Vector3f pa_local_;
    Vector3f pb_local_;
    float    radius_;
};

} // namespace model
} // namespace zemax
