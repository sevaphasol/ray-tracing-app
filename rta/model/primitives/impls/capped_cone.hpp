#pragma once

#include "rta/model/primitives/primitive.hpp"
#include "rta/model/rendering/ray.hpp"
#include "rta/model/rendering/vector3.hpp"

#include <memory>
#include <optional>

namespace rta {
namespace model {

class CappedCone : public Primitive {
  public:
    CappedCone( const Material& material,
                const Vector3f& center,
                const Vector3f& pa_local,
                const Vector3f& pb_local,
                float           ra,
                float           rb );

    std::optional<Primitive::IntersectionInfo>
    calcRayIntersection( const Ray& ray ) const override final;

    Vector3f
    calcNormal( const Vector3f& point, bool inside_object ) const override final;

    std::unique_ptr<Primitive>
    clone() const override
    {
        return std::make_unique<CappedCone>( *this );
    }

    const char*
    getName() const override final
    {
        return "CappedCone";
    }
    PrimitiveCode
    getCode() const override final
    {
        return PrimitiveCode::CappedCone;
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
    getRadiusA() const
    {
        return ra_;
    }

    float
    getRadiusB() const
    {
        return rb_;
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
    setRadiusA( float r )
    {
        ra_ = r;
    }

    void
    setRadiusB( float r )
    {
        rb_ = r;
    }

  private:
    Vector3f pa_local_;
    Vector3f pb_local_;
    float    ra_;
    float    rb_;
};

} // namespace model
} // namespace rta
