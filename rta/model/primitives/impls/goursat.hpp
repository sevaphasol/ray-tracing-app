#pragma once

#include "rta/model/primitives/primitive.hpp"
#include "rta/model/rendering/ray.hpp"

#include <memory>
#include <optional>

namespace rta {
namespace model {

class Goursat : public Primitive {
  public:
    Goursat( const Material& material, const Vector3f& center, float ka, float kb );

    std::optional<Primitive::IntersectionInfo>
    calcRayIntersection( const Ray& ray ) const override final;

    Vector3f
    calcNormal( const Vector3f& point, bool inside_object ) const override final;

    std::unique_ptr<Primitive>
    clone() const override
    {
        return std::make_unique<Goursat>( *this );
    }

    const char*
    getName() const override final
    {
        return "Goursat";
    }

    std::array<Vector3f, 8>
    getCircumscribedAABB() const override final;

    float
    getKa() const
    {
        return ka_;
    }

    float
    getKb() const
    {
        return kb_;
    }

    void
    setKa( float v )
    {
        ka_ = v;
    }

    void
    setKb( float v )
    {
        kb_ = v;
    }

  private:
    float ka_;
    float kb_;
};

} // namespace model
} // namespace rta
