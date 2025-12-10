/*
    Triangle intersection (barycentric), но теперь вершины в локальных координатах
    относительно origin, а луч в calcRayIntersection переводим в локал:
    ro_local = ro_world - origin.
*/

#include "zemax/model/primitives/impls/triangle.hpp"
#include "zemax/model/rendering/vector3.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace zemax {
namespace model {

Triangle::Triangle( const Material& material,
                    const Vector3f& v0_world,
                    const Vector3f& v1_world,
                    const Vector3f& v2_world )
    // origin = центр масс треугольника
    : Primitive( material, ( v0_world + v1_world + v2_world ) * ( 1.0f / 3.0f ) )
{
    Vector3f o = getOrigin();
    v0_local_  = v0_world - o;
    v1_local_  = v1_world - o;
    v2_local_  = v2_world - o;
}

std::optional<Primitive::IntersectionInfo>
Triangle::calcRayIntersection( const Ray& ray ) const
{
    // Переводим луч в локальное пространство треугольника:
    const Vector3f ro = worldToLocalPoint( ray.getBasePoint() );
    const Vector3f rd = worldToLocalDir( ray.getDir() );

    const Vector3f& v0 = v0_local_;
    const Vector3f& v1 = v1_local_;
    const Vector3f& v2 = v2_local_;

    Vector3f v1v0 = v1 - v0;
    Vector3f v2v0 = v2 - v0;
    Vector3f rov0 = ro - v0;

    Vector3f n     = cross( v1v0, v2v0 );
    float    denom = scalarMul( rd, n );
    if ( std::fabs( denom ) < 1e-6f )
    {
        // Луч почти параллелен плоскости
        return std::nullopt;
    }

    Vector3f q     = cross( rov0, rd );
    float    inv_d = 1.0f / denom;

    float u = inv_d * scalarMul( -q, v2v0 );
    float v = inv_d * scalarMul( q, v1v0 );
    float t = inv_d * scalarMul( -n, rov0 );

    if ( u < 0.0f || v < 0.0f || u + v > 1.0f || t <= 1e-4f )
    {
        return std::nullopt;
    }

    Primitive::IntersectionInfo info;
    info.close_distance = t; // t вдоль rd в МИРОВЫХ = t вдоль rd в локале (только трансляция)
    info.far_distance   = t;
    info.inside_object  = false;

    Vector3f nrm = n.normalize();
    info.normal  = localToWorldNormal( nrm );

    return info;
}

Vector3f
Triangle::calcNormal( const Vector3f& /*point*/, bool inside_object ) const
{
    Vector3f n = cross( v1_local_ - v0_local_, v2_local_ - v0_local_ ).normalize();
    if ( inside_object )
        n = -n;
    return localToWorldNormal( n );
}

std::array<Vector3f, 8>
Triangle::getCircumscribedAABB() const
{
    // AABB считаем в МИРОВЫХ координатах
    Vector3f w0 = localToWorldPoint( v0_local_ );
    Vector3f w1 = localToWorldPoint( v1_local_ );
    Vector3f w2 = localToWorldPoint( v2_local_ );

    float min_x = std::min( { w0.x, w1.x, w2.x } );
    float max_x = std::max( { w0.x, w1.x, w2.x } );
    float min_y = std::min( { w0.y, w1.y, w2.y } );
    float max_y = std::max( { w0.y, w1.y, w2.y } );
    float min_z = std::min( { w0.z, w1.z, w2.z } );
    float max_z = std::max( { w0.z, w1.z, w2.z } );

    return { { { min_x, min_y, min_z },
               { max_x, min_y, min_z },
               { min_x, max_y, min_z },
               { max_x, max_y, min_z },
               { min_x, min_y, max_z },
               { max_x, min_y, max_z },
               { min_x, max_y, max_z },
               { max_x, max_y, max_z } } };
}

} // namespace model
} // namespace zemax
