#include "zemax/model/primitives/impls/aabb.hpp"
#include "zemax/model/primitives/primitive.hpp"
#include "zemax/model/rendering/vector3.hpp"
#include <optional>

namespace zemax {
namespace model {

AABB::AABB( const Material& material, const Vector3f& center, const Vector3f& half_size )
    : Primitive( material, center ), half_size_( half_size )
{
}

std::optional<Primitive::IntersectionInfo>
AABB::calcRayIntersection( const Ray& ray ) const
{
    Vector3f ro = worldToLocalPoint( ray.getBasePoint() );
    Vector3f rd = worldToLocalDir( ray.getDir() );

    Vector3f m;
    m.x = ( std::abs( rd.x ) > 1e-6f ) ? 1.0f / rd.x : std::numeric_limits<float>::max();
    m.y = ( std::abs( rd.y ) > 1e-6f ) ? 1.0f / rd.y : std::numeric_limits<float>::max();
    m.z = ( std::abs( rd.z ) > 1e-6f ) ? 1.0f / rd.z : std::numeric_limits<float>::max();

    Vector3f n = { m.x * ro.x, m.y * ro.y, m.z * ro.z };
    Vector3f k = { std::abs( m.x ) * half_size_.x,
                   std::abs( m.y ) * half_size_.y,
                   std::abs( m.z ) * half_size_.z };

    Vector3f t1 = { -n.x - k.x, -n.y - k.y, -n.z - k.z };
    Vector3f t2 = { -n.x + k.x, -n.y + k.y, -n.z + k.z };

    float t_n = std::max( { t1.x, t1.y, t1.z } );
    float t_f = std::min( { t2.x, t2.y, t2.z } );

    if ( t_n > t_f || t_f < 0.0f )
    {
        return std::nullopt;
    }

    Primitive::IntersectionInfo info;
    info.close_distance = t_n;
    info.far_distance   = t_f;
    info.inside_object  = ( t_n <= 0.0f );

    if ( !info.inside_object )
    {
        info.normal = step( Vector3f{ t_n, t_n, t_n }, t1 );
    } else
    {
        info.normal = step( t2, Vector3f{ t_f, t_f, t_f } );
    }

    info.normal = localToWorldNormal( info.normal.value() * sign( rd ) );

    return info;
}

Vector3f
AABB::calcNormal( const Vector3f& point, bool inside_object ) const
{
    Vector3f local     = worldToLocalPoint( point );
    Vector3f abs_local = { std::abs( local.x ), std::abs( local.y ), std::abs( local.z ) };

    if ( abs_local.x >= abs_local.y && abs_local.x >= abs_local.z )
    {
        return localToWorldNormal( { ( local.x >= 0.0f ) ? 1.0f : -1.0f, 0.0f, 0.0f } );
    }
    if ( abs_local.y >= abs_local.z )
    {
        return localToWorldNormal( { 0.0f, ( local.y >= 0.0f ) ? 1.0f : -1.0f, 0.0f } );
    }
    return localToWorldNormal( { 0.0f, 0.0f, ( local.z >= 0.0f ) ? 1.0f : -1.0f } );
}

std::array<Vector3f, 8>
AABB::getCircumscribedAABB() const
{
    Vector3f h = half_size_;

    std::array<Vector3f, 8> corners_local = { { { -h.x, -h.y, -h.z },
                                                { h.x, -h.y, -h.z },
                                                { -h.x, h.y, -h.z },
                                                { h.x, h.y, -h.z },
                                                { -h.x, -h.y, h.z },
                                                { h.x, -h.y, h.z },
                                                { -h.x, h.y, h.z },
                                                { h.x, h.y, h.z } } };

    for ( auto& c : corners_local )
    {
        c = localToWorldPoint( c );
    }

    return corners_local;
}

} // namespace model
} // namespace zemax
