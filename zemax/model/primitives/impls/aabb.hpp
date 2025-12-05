#pragma once

#include "zemax/model/primitives/primitive.hpp"
#include "zemax/model/rendering/ray.hpp"
#include "zemax/model/rendering/vector3.hpp"

namespace zemax {
namespace model {

class AABB : public Primitive {
  public:
    AABB( const Material& material, const Vector3f& center, const Vector3f& half_size );

    virtual std::optional<Primitive::IntersectionInfo>
    calcRayIntersection( const Ray& ray ) const override final;

    virtual Vector3f
    calcNormal( const Vector3f& point, bool inside_object ) const override final;

    std::unique_ptr<Primitive>
    clone() const override
    {
        return std::make_unique<AABB>( *this );
    }

    const char*
    getName() override final
    {
        return "AABB";
    }

    std::array<Vector3f, 8>
    getCircumscribedAABB() const override final;

  private:
    Vector3f half_size_;
};

} // namespace model
} // namespace zemax
