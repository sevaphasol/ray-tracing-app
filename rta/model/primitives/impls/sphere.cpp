#include "rta/model/primitives/impls/sphere.hpp"
#include "rta/model/rendering/vector2.hpp"
#include "rta/model/rendering/vector3.hpp"
#include <optional>

namespace rta {
namespace model {

Sphere::Sphere( const Material& material, const Vector3f& center, float radius )
    : Primitive( material, center ), radius_( radius ), radius_sq_( radius * radius )
{
}

std::optional<Primitive::IntersectionInfo>
Sphere::calcRayIntersection( const Ray& ray ) const
{
    Vector3f ro_local = worldToLocalPoint( ray.getBasePoint() );
    Vector3f rd_local = worldToLocalDir( ray.getDir() );
    Vector3f oc       = ro_local;

    float b = scalarMul( oc, rd_local );
    float c = scalarMul( oc, oc ) - radius_sq_;

    float h = b * b - c;

    if ( h < 0.0f )
    {
        return std::nullopt;
    }

    h = sqrt( h );

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
    Vector3f normal_local = worldToLocalPoint( point );
    normal_local.normalize();

    if ( inside_object )
    {
        normal_local = -normal_local;
    }

    return localToWorldNormal( normal_local );
}

std::array<Vector3f, 8>
Sphere::getCircumscribedAABB() const
{
    float r = radius_;

    std::array<Vector3f, 8> corners_local = { { { -r, -r, -r },
                                                { r, -r, -r },
                                                { -r, r, -r },
                                                { r, r, -r },
                                                { -r, -r, r },
                                                { r, -r, r },
                                                { -r, r, r },
                                                { r, r, r } } };

    for ( auto& c : corners_local )
    {
        c = localToWorldPoint( c );
    }

    return corners_local;
}

} // namespace model
} // namespace rta
