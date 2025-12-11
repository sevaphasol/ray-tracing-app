#pragma once

#include "rta/model/primitives/primitive.hpp"
#include "rta/model/rendering/ray.hpp"
#include "rta/model/rendering/vector3.hpp"

#include <memory>
#include <optional>

namespace rta {
namespace model {

class Ellipse : public Primitive {
  public:
    Ellipse( const Material& material,
             const Vector3f& center,
             const Vector3f& u,
             const Vector3f& v );

    std::optional<Primitive::IntersectionInfo>
    calcRayIntersection( const Ray& ray ) const override final;

    Vector3f
    calcNormal( const Vector3f& point, bool inside_object ) const override final;

    std::unique_ptr<Primitive>
    clone() const override
    {
        return std::make_unique<Ellipse>( *this );
    }

    const char*
    getName() const override final
    {
        return "Ellipse";
    }

    std::array<Vector3f, 8>
    getCircumscribedAABB() const override final;

    const Vector3f&
    getU() const
    {
        return u_;
    }

    const Vector3f&
    getV() const
    {
        return v_;
    }

    void
    setU( const Vector3f& u )
    {
        u_ = u;
    }

    void
    setV( const Vector3f& v )
    {
        v_ = v;
    }

  private:
    Vector3f u_;
    Vector3f v_;
};

} // namespace model
} // namespace rta
