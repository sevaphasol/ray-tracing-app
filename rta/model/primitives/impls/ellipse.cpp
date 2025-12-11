/*
    Ellipse intersection
    Shadertoy ref:

    vec3 ellIntersect( vec3 ro, vec3 rd, vec3 c, vec3 u, vec3 v )
    {
        vec3 q = ro - c;
        vec3 n = cross(u,v);
        float t = -dot(n,q)/dot(rd,n);
        float r = dot(u,q + rd*t);
        float s = dot(v,q + rd*t);
        if( r*r+s*s>1.0 ) return vec3(-1.0);
        return vec3(t,s,r);
    }

    vec3 ellNormal( vec3 u, vec3 v )
    {
        return normalize( cross(u,v) );
    }
*/

#include "rta/model/primitives/impls/ellipse.hpp"
#include "rta/model/rendering/vector3.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace rta {
namespace model {

Ellipse::Ellipse( const Material& material,
                  const Vector3f& center,
                  const Vector3f& u,
                  const Vector3f& v )
    : Primitive( material, center ), u_( u ), v_( v )
{
}

namespace {

inline float
safeDot( const Vector3f& a, const Vector3f& b )
{
    return scalarMul( a, b );
}

} // anonymous namespace

std::optional<Primitive::IntersectionInfo>
Ellipse::calcRayIntersection( const Ray& ray ) const
{
    const Vector3f ro = worldToLocalPoint( ray.getBasePoint() );
    const Vector3f rd = worldToLocalDir( ray.getDir() );

    Vector3f q = ro;
    Vector3f n = cross( u_, v_ );

    float denom = safeDot( rd, n );
    if ( std::fabs( denom ) < 1e-6f )
    {
        // луч почти параллелен плоскости эллипса
        return std::nullopt;
    }

    float t = -safeDot( n, q ) / denom;
    if ( t <= 1e-4f )
    {
        return std::nullopt;
    }

    Vector3f hit = q + rd * t;
    float    r   = safeDot( u_, hit );
    float    s   = safeDot( v_, hit );

    if ( r * r + s * s > 1.0f )
    {
        return std::nullopt;
    }

    Primitive::IntersectionInfo info;
    info.close_distance = t;
    info.far_distance   = t;
    info.inside_object  = false;

    Vector3f nrm = n.normalize();
    // если нужно одно-стороннее освещение, можно фейсинг-флипать по rd
    info.normal = localToWorldNormal( nrm );

    return info;
}

Vector3f
Ellipse::calcNormal( const Vector3f& /*point*/, bool inside_object ) const
{
    Vector3f n = cross( u_, v_ ).normalize();
    if ( inside_object )
        n = -n;
    return localToWorldNormal( n );
}

std::array<Vector3f, 8>
Ellipse::getCircumscribedAABB() const
{
    // Грубый AABB: |u| + |v| по компонентам
    Vector3f c   = getOrigin();
    Vector3f ext = { std::fabs( u_.x ) + std::fabs( v_.x ),
                     std::fabs( u_.y ) + std::fabs( v_.y ),
                     std::fabs( u_.z ) + std::fabs( v_.z ) };

    std::array<Vector3f, 8> corners_local = { { { -ext.x, -ext.y, -ext.z },
                                                { ext.x, -ext.y, -ext.z },
                                                { -ext.x, ext.y, -ext.z },
                                                { ext.x, ext.y, -ext.z },
                                                { -ext.x, -ext.y, ext.z },
                                                { ext.x, -ext.y, ext.z },
                                                { -ext.x, ext.y, ext.z },
                                                { ext.x, ext.y, ext.z } } };

    for ( auto& c : corners_local )
    {
        c = localToWorldPoint( c );
    }

    return corners_local;
}

} // namespace model
} // namespace rta
