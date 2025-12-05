#include "zemax/model/primitives/impls/sphere.hpp"
#include "zemax/model/rendering/vector2.hpp"
#include "zemax/model/rendering/vector3.hpp"
#include <optional>

namespace zemax {
namespace model {

Sphere::Sphere( const Material& material, const Vector3f& center, float radius )
    : Primitive( material, center ), radius_( radius ), radius_sq_( radius * radius )
{
}

std::optional<Primitive::IntersectionInfo>
Sphere::calcRayIntersection( const Ray& ray ) const
{
    Vector3f ro = ray.getBasePoint();
    Vector3f rd = ray.getDir();
    Vector3f ce = getOrigin();
    Vector3f oc = ro - ce;

    float b = scalarMul( oc, rd );
    float c = scalarMul( oc, oc ) - radius_sq_;

    float h = b * b - c;

    if ( h < 0.0f )
    {
        return std::nullopt;
    }

    h = sqrt( h );

    float t = -b - h;

    IntersectionInfo info;

    info.close_distance = -b - h;
    info.far_distance   = -b + h;
    info.inside_object  = info.close_distance < 0.0f;
    info.normal         = std::nullopt;

    return info;
}

Vector3f
Sphere::calcNormal( const Vector3f& point, bool inside_object ) const
{
    Vector3f normal = ( point - getOrigin() ).normalize();

    if ( inside_object )
    {
        normal = -normal;
    }

    return normal;
}

std::array<Vector3f, 8>
Sphere::getCircumscribedAABB() const
{
    Vector3f c = getOrigin();

    float r = radius_;

    return { { { c.x - r, c.y - r, c.z - r },
               { c.x + r, c.y - r, c.z - r },
               { c.x - r, c.y + r, c.z - r },
               { c.x + r, c.y + r, c.z - r },
               { c.x - r, c.y - r, c.z + r },
               { c.x + r, c.y - r, c.z + r },
               { c.x - r, c.y + r, c.z + r },
               { c.x + r, c.y + r, c.z + r } } };
}

} // namespace model
} // namespace zemax
