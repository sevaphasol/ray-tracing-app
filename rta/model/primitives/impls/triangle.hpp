#pragma once

#include "rta/model/primitives/primitive.hpp"
#include "rta/model/rendering/ray.hpp"
#include "rta/model/rendering/vector3.hpp"

#include <memory>
#include <optional>

namespace rta {
namespace model {

class Triangle : public Primitive {
  public:
    Triangle( const Material& material,
              const Vector3f& v0_world,
              const Vector3f& v1_world,
              const Vector3f& v2_world );

    std::optional<Primitive::IntersectionInfo>
    calcRayIntersection( const Ray& ray ) const override final;

    Vector3f
    calcNormal( const Vector3f& point, bool inside_object ) const override final;

    std::unique_ptr<Primitive>
    clone() const override
    {
        return std::make_unique<Triangle>( *this );
    }

    const char*
    getName() const override final
    {
        return "Triangle";
    }
    PrimitiveCode
    getCode() const override final
    {
        return PrimitiveCode::Triangle;
    }

    std::array<Vector3f, 8>
    getCircumscribedAABB() const override final;

    Vector3f
    getV0() const
    {
        return getOrigin() + v0_local_;
    }

    Vector3f
    getV1() const
    {
        return getOrigin() + v1_local_;
    }

    Vector3f
    getV2() const
    {
        return getOrigin() + v2_local_;
    }

    const Vector3f&
    getV0Local() const
    {
        return v0_local_;
    }

    const Vector3f&
    getV1Local() const
    {
        return v1_local_;
    }

    const Vector3f&
    getV2Local() const
    {
        return v2_local_;
    }

    void
    setV0World( const Vector3f& v )
    {
        Vector3f o = getOrigin();
        v0_local_  = v - o;
    }

    void
    setV1World( const Vector3f& v )
    {
        Vector3f o = getOrigin();
        v1_local_  = v - o;
    }

    void
    setV2World( const Vector3f& v )
    {
        Vector3f o = getOrigin();
        v2_local_  = v - o;
    }

  private:
    Vector3f v0_local_;
    Vector3f v1_local_;
    Vector3f v2_local_;
};

} // namespace model
} // namespace rta
