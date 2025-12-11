#pragma once

#include "rta/model/primitives/primitive.hpp"
#include "rta/model/rendering/ray.hpp"
#include "rta/model/rendering/vector3.hpp"

#include <memory>
#include <optional>

namespace rta {
namespace model {

class Wedge : public Primitive {
  public:
    // s — половинный размер по осям (как в iWedge: box от -s до +s).
    Wedge( const Material& material, const Vector3f& center, const Vector3f& s );

    std::optional<Primitive::IntersectionInfo>
    calcRayIntersection( const Ray& ray ) const override final;

    Vector3f
    calcNormal( const Vector3f& point, bool inside_object ) const override final;

    std::unique_ptr<Primitive>
    clone() const override
    {
        return std::make_unique<Wedge>( *this );
    }

    const char*
    getName() const override final
    {
        return "Wedge";
    }

    std::array<Vector3f, 8>
    getCircumscribedAABB() const override final;

    const Vector3f&
    getS() const
    {
        return s_;
    }

    void
    setS( const Vector3f& s )
    {
        s_ = s;
    }

  private:
    Vector3f s_;
};

} // namespace model
} // namespace rta
