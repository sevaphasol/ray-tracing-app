#include "zemax/model/primitives/impls/plane.hpp"
#include "zemax/model/rendering/vector3.hpp"

namespace zemax {
namespace model {

Plane::Plane( const Material& material, const Vector3f& base_point, const Vector3f& normal )
    : Primitive( material, base_point ), normal_( normal )
{
    normal_.normalize();
}

std::optional<Primitive::IntersectionInfo>
Plane::calcRayIntersection( const Ray& ray ) const
{
    Vector3f r0_local = worldToLocalPoint( ray.getBasePoint() );
    Vector3f a        = worldToLocalDir( ray.getDir() );
    auto     n_local  = normal_;

    auto a_pr = scalarMul( a, n_local );

    if ( a_pr == 0 )
    {
        return std::nullopt;
    }

    float t = scalarMul( -r0_local, n_local ) / a_pr;

    if ( t < 0.0f )
    {
        return std::nullopt;
    }

    IntersectionInfo info;

    info.close_distance = t;
    info.far_distance   = t;
    info.inside_object  = false;
    info.normal         = localToWorldNormal( n_local );

    return info;
}

Vector3f
Plane::calcNormal( const Vector3f& point, bool inside_object ) const
{
    Vector3f n = normal_;
    return localToWorldNormal( n );
}

std::array<Vector3f, 8>
Plane::getCircumscribedAABB() const
{
    float h = 1.0f;

    std::array<Vector3f, 8> corners_local = { { { -h, -h, -h },
                                                { h, -h, -h },
                                                { -h, h, -h },
                                                { h, h, -h },
                                                { -h, -h, h },
                                                { h, -h, h },
                                                { -h, h, h },
                                                { h, h, h } } };

    for ( auto& c : corners_local )
    {
        c = localToWorldPoint( c );
    }

    return corners_local;
}

} // namespace model
} // namespace zemax
