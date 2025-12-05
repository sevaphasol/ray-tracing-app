/*

Spizheno s https://iquilezles.org/articles/intersectors/

*/

#include "zemax/model/primitives/impls/torus.hpp"
#include "zemax/model/rendering/vector3.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace zemax {
namespace model {

Torus::Torus( const Material& material,
              const Vector3f& center,
              float           major_radius,
              float           minor_radius )
    : Primitive( material, center ), major_radius_( major_radius ), minor_radius_( minor_radius )
{
}

namespace {

float
solveQuadratic( float a, float b, float c )
{
    if ( std::abs( a ) < 1e-6f )
    {
        if ( std::abs( b ) < 1e-6f )
            return std::numeric_limits<float>::max();
        return ( -c / b > 0.0f ) ? ( -c / b ) : std::numeric_limits<float>::max();
    }
    float disc = b * b - 4.0f * a * c;
    if ( disc < 0.0f )
        return std::numeric_limits<float>::max();
    float sqrt_disc = std::sqrt( disc );
    float t1        = ( -b - sqrt_disc ) / ( 2.0f * a );
    float t2        = ( -b + sqrt_disc ) / ( 2.0f * a );
    float t         = std::numeric_limits<float>::max();
    if ( t1 > 1e-4f )
        t = t1;
    if ( t2 > 1e-4f && t2 < t )
        t = t2;
    return t;
}

float
torIntersect( const Vector3f& ro, const Vector3f& rd, float Ra, float ra )
{
    const float Ra2 = Ra * Ra;
    const float ra2 = ra * ra;

    const float m = scalarMul( ro, ro );
    const float n = scalarMul( ro, rd );
    const float k = ( m + Ra2 - ra2 ) * 0.5f;

    float k3 = n;
    float k2 = n * n - Ra2 * ( rd.x * rd.x + rd.y * rd.y ) + k;
    float k1 = n * k - Ra2 * ( rd.x * ro.x + rd.y * ro.y );
    float k0 = k * k - Ra2 * ( ro.x * ro.x + ro.y * ro.y );

    float po = 1.0f;
    float t0 = std::numeric_limits<float>::max();

    if ( std::abs( k3 * ( k3 * k3 - k2 ) + k1 ) < 0.01f )
    {
        po = -1.0f;
        std::swap( k1, k3 );
        if ( std::abs( k0 ) < 1e-6f )
            return t0;
        float inv_k0 = 1.0f / k0;
        k1 *= inv_k0;
        k2 *= inv_k0;
        k3 *= inv_k0;
        k0 = 1.0f;
    }

    const float c2 = ( 2.0f * k2 - 3.0f * k3 * k3 ) / 3.0f;
    const float c1 = 2.0f * ( k3 * ( k3 * k3 - k2 ) + k1 );
    const float c0 = ( k3 * ( k3 * ( c2 * 3.0f + 2.0f * k2 ) - 8.0f * k1 ) + 4.0f * k0 ) / 3.0f;

    const float Q = c2 * c2 + c0;
    const float R = c2 * c2 * c2 - 3.0f * c2 * c0 + c1 * c1 * 0.25f;
    const float h = R * R - Q * Q * Q;

    if ( h >= 0.0f )
    {
        const float sqrt_h = std::sqrt( h );
        auto        cbrt   = []( float x ) -> float {
            return ( x > 0.0f ) ? std::cbrt( x ) : -std::cbrt( -x );
        };
        const float v = cbrt( R + sqrt_h );
        const float u = cbrt( R - sqrt_h );
        const float y =
            std::sqrt( 0.5f * ( std::sqrt( ( v + u + 4.0f * c2 ) * ( v + u + 4.0f * c2 ) +
                                           3.0f * ( v - u ) * ( v - u ) ) +
                                ( v + u + 4.0f * c2 ) ) );
        const float x     = ( y > 1e-6f ) ? ( 0.5f * std::sqrt( 3.0f ) * ( v - u ) / y ) : 0.0f;
        const float r_val = ( x * x + y * y > 1e-6f ) ? ( 2.0f * c1 / ( x * x + y * y ) ) : 0.0f;

        float t1 = x - r_val - k3;
        float t2 = -x - r_val - k3;
        if ( po < 0.0f )
        {
            if ( std::abs( t1 ) > 1e-6f )
                t1 = 2.0f / t1;
            if ( std::abs( t2 ) > 1e-6f )
                t2 = 2.0f / t2;
        }
        if ( t1 > 1e-4f )
            t0 = t1;
        if ( t2 > 1e-4f && t2 < t0 )
            t0 = t2;
        return t0;
    }

    if ( Q < 0.0f )
        return t0;
    const float sQ    = std::sqrt( Q );
    const float theta = std::acos( std::clamp( -R / ( sQ * Q ), -1.0f, 1.0f ) ) / 3.0f;
    const float w     = sQ * std::cos( theta );
    const float d2    = -( w + c2 );
    if ( d2 < 0.0f )
        return t0;
    const float d1 = std::sqrt( d2 );
    const float h1 =
        ( w - 2.0f * c2 + c1 / d1 >= 0.0f ) ? std::sqrt( w - 2.0f * c2 + c1 / d1 ) : 0.0f;
    const float h2 =
        ( w - 2.0f * c2 - c1 / d1 >= 0.0f ) ? std::sqrt( w - 2.0f * c2 - c1 / d1 ) : 0.0f;

    float t1 = -d1 - h1 - k3;
    float t2 = -d1 + h1 - k3;
    float t3 = d1 - h2 - k3;
    float t4 = d1 + h2 - k3;

    if ( po < 0.0f )
    {
        if ( std::abs( t1 ) > 1e-6f )
            t1 = 2.0f / t1;
        if ( std::abs( t2 ) > 1e-6f )
            t2 = 2.0f / t2;
        if ( std::abs( t3 ) > 1e-6f )
            t3 = 2.0f / t3;
        if ( std::abs( t4 ) > 1e-6f )
            t4 = 2.0f / t4;
    }

    float t = std::numeric_limits<float>::max();
    if ( t1 > 1e-4f )
        t = t1;
    if ( t2 > 1e-4f && t2 < t )
        t = t2;
    if ( t3 > 1e-4f && t3 < t )
        t = t3;
    if ( t4 > 1e-4f && t4 < t )
        t = t4;
    return t;
}

} // anonymous namespace

std::optional<Primitive::IntersectionInfo>
Torus::calcRayIntersection( const Ray& ray ) const
{
    Vector3f ro = ray.getBasePoint() - getOrigin();
    Vector3f rd = ray.getDir();

    float t = torIntersect( ro, rd, major_radius_, minor_radius_ );

    if ( t >= std::numeric_limits<float>::max() || t < 0.0f )
    {
        return std::nullopt;
    }

    Primitive::IntersectionInfo info;
    info.close_distance = t;
    info.far_distance   = t;
    info.inside_object  = false;
    info.normal         = std::nullopt;
    return info;
}

Vector3f
Torus::calcNormal( const Vector3f& point, bool /*inside_object*/ ) const
{
    Vector3f p    = point - getOrigin();
    float    Ra2  = major_radius_ * major_radius_;
    float    ra2  = minor_radius_ * minor_radius_;
    float    len2 = scalarMul( p, p );
    Vector3f grad = p * ( len2 - ra2 - Ra2 ) + Vector3f( -Ra2 * p.x, -Ra2 * p.y, Ra2 * p.z );
    return grad.normalize();
}

std::array<Vector3f, 8>
Torus::getCircumscribedAABB() const
{
    auto c = getOrigin();

    float dx = major_radius_ + minor_radius_;
    float dy = major_radius_ + minor_radius_;
    float dz = minor_radius_;

    return { { { c.x - dx, c.y - dy, c.z - dz },
               { c.x + dx, c.y - dy, c.z - dz },
               { c.x - dx, c.y + dy, c.z - dz },
               { c.x + dx, c.y + dy, c.z - dz },
               { c.x - dx, c.y - dy, c.z + dz },
               { c.x + dx, c.y - dy, c.z + dz },
               { c.x - dx, c.y + dy, c.z + dz },
               { c.x + dx, c.y + dy, c.z + dz } } };
}

} // namespace model
} // namespace zemax
