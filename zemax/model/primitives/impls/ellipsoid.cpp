/*
    Ellipsoid (Inigo Quilez intersectors)
*/

#include "zemax/model/primitives/impls/ellipsoid.hpp"
#include "zemax/model/rendering/vector3.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace zemax {
namespace model {

Ellipsoid::Ellipsoid( const Material& material, const Vector3f& center, const Vector3f& radii )
    : Primitive( material, center ), radii_( radii )
{
}

namespace {

std::pair<float, float>
eliIntersect( const Vector3f& ro, const Vector3f& rd, const Vector3f& ra )
{
    // ellipsoid centered at origin with radii ra
    Vector3f ocn = Vector3f{ ro.x / ra.x, ro.y / ra.y, ro.z / ra.z };
    Vector3f rdn = Vector3f{ rd.x / ra.x, rd.y / ra.y, rd.z / ra.z };

    float a = scalarMul( rdn, rdn );
    float b = scalarMul( ocn, rdn );
    float c = scalarMul( ocn, ocn );
    float h = b * b - a * ( c - 1.0f );
    if ( h < 0.0f )
        return { std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };

    h        = std::sqrt( h );
    float t1 = ( -b - h ) / a;
    float t2 = ( -b + h ) / a;
    return { t1, t2 };
}

} // anonymous namespace

std::optional<Primitive::IntersectionInfo>
Ellipsoid::calcRayIntersection( const Ray& ray ) const
{
    Vector3f ro = ray.getBasePoint() - getOrigin();
    Vector3f rd = ray.getDir();

    auto [t1, t2] = eliIntersect( ro, rd, radii_ );
    if ( t1 == std::numeric_limits<float>::max() )
        return std::nullopt;

    if ( t1 > t2 )
        std::swap( t1, t2 );

    if ( t2 < 0.0f )
        return std::nullopt;

    Primitive::IntersectionInfo info;
    info.close_distance = t1;
    info.far_distance   = t2;
    info.inside_object  = ( t1 < 0.0f );
    info.normal         = std::nullopt;
    return info;
}

Vector3f
Ellipsoid::calcNormal( const Vector3f& point, bool inside_object ) const
{
    Vector3f p = point - getOrigin();

    Vector3f n{ p.x / ( radii_.x * radii_.x ),
                p.y / ( radii_.y * radii_.y ),
                p.z / ( radii_.z * radii_.z ) };

    n.normalize();

    if ( inside_object )
        n = -n;

    return n;
}

std::array<Vector3f, 8>
Ellipsoid::getCircumscribedAABB() const
{
    Vector3f c = getOrigin();
    Vector3f r = radii_;

    return { { { c.x - r.x, c.y - r.y, c.z - r.z },
               { c.x + r.x, c.y - r.y, c.z - r.z },
               { c.x - r.x, c.y + r.y, c.z - r.z },
               { c.x + r.x, c.y + r.y, c.z - r.z },
               { c.x - r.x, c.y - r.y, c.z + r.z },
               { c.x + r.x, c.y - r.y, c.z + r.z },
               { c.x - r.x, c.y + r.y, c.z + r.z },
               { c.x + r.x, c.y + r.y, c.z + r.z } } };
}

} // namespace model
} // namespace zemax
