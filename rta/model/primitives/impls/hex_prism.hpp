// rta/model/primitives/impls/hex_prism.hpp
#pragma once
#include "rta/model/primitives/primitive.hpp"
#include "rta/model/rendering/ray.hpp"
#include <optional>

namespace rta {
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
    getName() const override final
    {
        return "HexPrism";
    }

    std::array<Vector3f, 8>
    getCircumscribedAABB() const override final;

    float
    getRadius() const
    {
        return radius_;
    }
    float
    getHeight() const
    {
        return height_;
    }
    void
    setRadius( float r )
    {
        radius_ = r;
    }
    void
    setHeight( float h )
    {
        height_ = h;
    }

  private:
    float radius_; // ra
    float height_; // he
};

} // namespace model
} // namespace rta
