// zemax/model/primitives/impls/hex_prism.hpp
#pragma once
#include "zemax/model/primitives/primitive.hpp"
#include "zemax/model/rendering/ray.hpp"
#include <optional>

namespace zemax {
namespace model {

class HexPrism : public Primitive {
  public:
    HexPrism( const Material& material,
              const Vector3f& center,
              float           radius, // ra — радиус описанной окружности шестиугольника
              float           height );         // he — половина высоты (от центра вверх/вниз)

    std::optional<Primitive::IntersectionInfo>
    calcRayIntersection( const Ray& ray ) const override final;

    Vector3f
    calcNormal( const Vector3f& point, bool inside_object ) const override final;

    std::unique_ptr<Primitive>
    clone() const override
    {
        return std::make_unique<HexPrism>( *this );
    }

    const char*
    getName() override final
    {
        return "HexPrism";
    }

    std::array<Vector3f, 8>
    getCircumscribedAABB() const override final;

  private:
    float radius_; // ra
    float height_; // he
};

} // namespace model
} // namespace zemax
