
#pragma once

#include "zemax/model/primitives/primitive.hpp"
#include "zemax/model/rendering/ray.hpp"
#include "zemax/model/rendering/vector3.hpp"

#include <memory>
#include <optional>

namespace zemax {
namespace model {

class RoundedBox : public Primitive {
  public:
    RoundedBox( const Material& material,
                const Vector3f& center,
                const Vector3f& half_size,
                float           radius );

    std::optional<Primitive::IntersectionInfo>
    calcRayIntersection( const Ray& ray ) const override final;

    Vector3f
    calcNormal( const Vector3f& point, bool inside_object ) const override final;

    std::unique_ptr<Primitive>
    clone() const override
    {
        return std::make_unique<RoundedBox>( *this );
    }

    const char*
    getName() const override final
    {
        return "RoundedBox";
    }

    std::array<Vector3f, 8>
    getCircumscribedAABB() const override final;

    const Vector3f&
    getHalfSize() const
    {
        return half_size_;
    }

    float
    getRadius() const
    {
        return radius_;
    }

    void
    setHalfSize( const Vector3f& h )
    {
        half_size_ = h;
    }

    void
    setRadius( float r )
    {
        radius_ = r;
    }

  private:
    Vector3f half_size_;
    float    radius_;
};

} // namespace model
} // namespace zemax
