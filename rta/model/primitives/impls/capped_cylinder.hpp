#pragma once

#include "rta/model/primitives/primitive.hpp"
#include "rta/model/rendering/ray.hpp"
#include "rta/model/rendering/vector3.hpp"

#include <memory>
#include <optional>

namespace rta {
namespace model {

class CappedCylinder : public Primitive {
  public:
    CappedCylinder( const Material& material,
                    const Vector3f& center,
                    const Vector3f& a_local,
                    const Vector3f& b_local,
                    float           radius );

    std::optional<Primitive::IntersectionInfo>
    calcRayIntersection( const Ray& ray ) const override final;

    Vector3f
    calcNormal( const Vector3f& point, bool inside_object ) const override final;

    std::unique_ptr<Primitive>
    clone() const override
    {
        return std::make_unique<CappedCylinder>( *this );
    }

    const char*
    getName() const override final
    {
        return "CappedCylinder";
    }
    PrimitiveCode
    getCode() const override final
    {
        return PrimitiveCode::CappedCylinder;
    }

    std::array<Vector3f, 8>
    getCircumscribedAABB() const override final;

    const Vector3f&
    getALocal() const
    {
        return a_local_;
    }

    const Vector3f&
    getBLocal() const
    {
        return b_local_;
    }

    float
    getRadius() const
    {
        return ra_;
    }

    void
    setALocal( const Vector3f& v )
    {
        a_local_ = v;
    }

    void
    setBLocal( const Vector3f& v )
    {
        b_local_ = v;
    }

    void
    setRadius( float r )
    {
        ra_ = r;
    }

  private:
    Vector3f a_local_;
    Vector3f b_local_;
    float    ra_;
};

} // namespace model
} // namespace rta
