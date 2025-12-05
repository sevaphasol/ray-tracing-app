#pragma once

#include "zemax/model/primitives/primitive.hpp"
#include "zemax/model/rendering/ray.hpp"
#include <memory>
#include <optional>

namespace zemax {
namespace model {

class Torus : public Primitive {
  public:
    Torus( const Material& material,
           const Vector3f& center,
           float           major_radius,
           float           minor_radius );

    std::optional<Primitive::IntersectionInfo>
    calcRayIntersection( const Ray& ray ) const override final;

    Vector3f
    calcNormal( const Vector3f& point, bool inside_object ) const override final;

    std::unique_ptr<Primitive>
    clone() const override
    {
        return std::make_unique<Torus>( *this );
    }

    const char*
    getName() override final
    {
        return "Torus";
    }

    virtual std::array<Vector3f, 8>
    getCircumscribedAABB() const override final;

  private:
    float major_radius_;
    float minor_radius_;
};

} // namespace model
} // namespace zemax
