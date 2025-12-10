/*
    Capped cylinder — Shadertoy cylIntersect + cylNormal.
*/

#include "zemax/model/primitives/impls/capped_cylinder.hpp"
#include "zemax/model/rendering/vector3.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace zemax {
namespace model {

CappedCylinder::CappedCylinder( const Material& material,
                                const Vector3f& center,
                                const Vector3f& a_local,
                                const Vector3f& b_local,
                                float           radius )
    : Primitive( material, center ), a_local_( a_local ), b_local_( b_local ), ra_( radius )
{
}

namespace {

inline float
dot2( const Vector3f& v )
{
    return scalarMul( v, v );
}

inline float
signf( float x )
{
    return ( x > 0.0f ) ? 1.0f : ( x < 0.0f ? -1.0f : 0.0f );
}

float
cylIntersect( const Vector3f& ro,
              const Vector3f& rd,
              const Vector3f& a,
              const Vector3f& b,
              float           ra,
              Vector3f&       outNormal )
{
    constexpr float INF = std::numeric_limits<float>::max();

    Vector3f ba   = b - a;
    Vector3f oc   = ro - a;
    float    baba = scalarMul( ba, ba );
    float    bard = scalarMul( ba, rd );
    float    baoc = scalarMul( ba, oc );

    float k2 = baba - bard * bard;
    float k1 = baba * scalarMul( oc, rd ) - baoc * bard;
    float k0 = baba * scalarMul( oc, oc ) - baoc * baoc - ra * ra * baba;
    float h  = k1 * k1 - k2 * k0;

    if ( h < 0.0f || std::fabs( k2 ) < 1e-6f )
        return INF;

    h        = std::sqrt( h );
    float t  = ( -k1 - h ) / k2;
    float y  = baoc + t * bard;
    float t0 = t;

    // body
    if ( y > 0.0f && y < baba && t > 1e-4f )
    {
        Vector3f n = oc + rd * t - ba * ( y / baba );
        outNormal  = n / ra;
        outNormal.normalize();
        return t;
    }

    // caps
    float denom = bard;
    if ( std::fabs( denom ) < 1e-6f )
        return INF;

    t = ( ( ( y < 0.0f ) ? 0.0f : baba ) - baoc ) / denom;
    if ( t <= 1e-4f )
        return INF;

    // проверка, что точка в диске
    float k1t = k1 + k2 * t;
    if ( std::fabs( k1t ) < h )
    {
        float    s   = signf( y );
        Vector3f n   = ba * s;
        float    len = std::sqrt( baba );
        if ( len > 0.0f )
            n = n * ( 1.0f / len );
        outNormal = n;
        return t;
    }

    return INF;
}

Vector3f
cylNormal( const Vector3f& p, const Vector3f& a, const Vector3f& b, float ra )
{
    Vector3f pa   = p - a;
    Vector3f ba   = b - a;
    float    baba = scalarMul( ba, ba );

    if ( baba < 1e-6f )
        return pa.normalize();

    float    h = scalarMul( pa, ba ) / baba;
    Vector3f n = ( pa - ba * h ) / ra;
    return n.normalize();
}

} // anonymous namespace

std::optional<Primitive::IntersectionInfo>
CappedCylinder::calcRayIntersection( const Ray& ray ) const
{
    Vector3f ro_local = worldToLocalPoint( ray.getBasePoint() );
    Vector3f rd       = worldToLocalDir( ray.getDir() );

    Vector3f n_local;
    float    t = cylIntersect( ro_local, rd, a_local_, b_local_, ra_, n_local );

    if ( t >= std::numeric_limits<float>::max() || t < 0.0f )
        return std::nullopt;

    Primitive::IntersectionInfo info;
    info.close_distance = t;
    info.far_distance   = t;
    info.inside_object  = false;
    info.normal         = localToWorldNormal( n_local );

    return info;
}

Vector3f
CappedCylinder::calcNormal( const Vector3f& point, bool inside_object ) const
{
    Vector3f p_local = worldToLocalPoint( point );
    Vector3f n       = cylNormal( p_local, a_local_, b_local_, ra_ );
    if ( inside_object )
        n = -n;
    return localToWorldNormal( n );
}

std::array<Vector3f, 8>
CappedCylinder::getCircumscribedAABB() const
{
    Vector3f a    = a_local_;
    Vector3f b    = b_local_;
    float    rmax = ra_;

    Vector3f min_local{ std::min( a.x, b.x ) - rmax,
                        std::min( a.y, b.y ) - rmax,
                        std::min( a.z, b.z ) - rmax };

    Vector3f max_local{ std::max( a.x, b.x ) + rmax,
                        std::max( a.y, b.y ) + rmax,
                        std::max( a.z, b.z ) + rmax };

    std::array<Vector3f, 8> corners_local = { { { min_local.x, min_local.y, min_local.z },
                                                { max_local.x, min_local.y, min_local.z },
                                                { min_local.x, max_local.y, min_local.z },
                                                { max_local.x, max_local.y, min_local.z },
                                                { min_local.x, min_local.y, max_local.z },
                                                { max_local.x, min_local.y, max_local.z },
                                                { min_local.x, max_local.y, max_local.z },
                                                { max_local.x, max_local.y, max_local.z } } };

    for ( auto& c : corners_local )
    {
        c = localToWorldPoint( c );
    }

    return corners_local;
}

} // namespace model
} // namespace zemax
