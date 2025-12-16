#include "rta/model/primitives/impls/goursat.hpp"
#include "rta/model/rendering/vector3.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace rta {
namespace model {

Goursat::Goursat( const Material& material, const Vector3f& center, float ka, float kb )
    : Primitive( material, center ), ka_( ka ), kb_( kb )
{
}

namespace {

struct Vec3d
{
    double x;
    double y;
    double z;
};

inline Vec3d
makeVec3d( const Vector3f& v )
{
    return Vec3d{ static_cast<double>( v.x ),
                  static_cast<double>( v.y ),
                  static_cast<double>( v.z ) };
}

inline Vec3d
operator*( const Vec3d& a, const Vec3d& b )
{
    return Vec3d{ a.x * b.x, a.y * b.y, a.z * b.z };
}

inline Vec3d
operator*( const Vec3d& a, double s )
{
    return Vec3d{ a.x * s, a.y * s, a.z * s };
}

inline Vec3d
operator*( double s, const Vec3d& a )
{
    return a * s;
}

inline Vec3d
operator+( const Vec3d& a, const Vec3d& b )
{
    return Vec3d{ a.x + b.x, a.y + b.y, a.z + b.z };
}

inline double
dot( const Vec3d& a, const Vec3d& b )
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline double
sign_glsl( double x )
{
    return ( x > 0.0 ) ? 1.0 : ( x < 0.0 ? -1.0 : 0.0 );
}

static double
gouIntersectInternal( const Vec3d& ro, const Vec3d& rd, double ka, double kb )
{
    double po = 1.0;

    Vec3d rd2 = rd * rd;
    Vec3d rd3 = rd2 * rd;

    Vec3d ro2 = ro * ro;
    Vec3d ro3 = ro2 * ro;

    double k4 = dot( rd2, rd2 );
    if ( std::fabs( k4 ) < 1e-12 )
        return -1.0;

    double k3 = dot( ro, rd3 );
    double k2 = dot( ro2, rd2 ) - kb / 6.0;
    double k1 = dot( ro3, rd ) - kb * dot( rd, ro ) / 2.0;
    double k0 = dot( ro2, ro2 ) + ka - kb * dot( ro, ro );

    k3 /= k4;
    k2 /= k4;
    k1 /= k4;
    k0 /= k4;

    double c2 = k2 - k3 * k3;
    double c1 = k1 + k3 * ( 2.0 * k3 * k3 - 3.0 * k2 );
    double c0 = k0 + k3 * ( k3 * ( c2 + k2 ) * 3.0 - 4.0 * k1 );

    if ( std::fabs( c1 ) < 0.1 * std::fabs( c2 ) )
    {
        po       = -1.0;
        double t = k1;
        k1       = k3;
        k3       = t;

        if ( std::fabs( k0 ) < 1e-12 )
            return -1.0;

        k0 = 1.0 / k0;
        k1 = k1 * k0;
        k2 = k2 * k0;
        k3 = k3 * k0;

        c2 = k2 - k3 * k3;
        c1 = k1 + k3 * ( 2.0 * k3 * k3 - 3.0 * k2 );
        c0 = k0 + k3 * ( k3 * ( c2 + k2 ) * 3.0 - 4.0 * k1 );
    }

    c0 /= 3.0;

    double Q = c2 * c2 + c0;
    double R = c2 * c2 * c2 - 3.0 * c0 * c2 + c1 * c1;
    double h = R * R - Q * Q * Q;

    if ( h > 0.0 )
    {
        h = std::sqrt( h );

        double Rp = R + h;
        double Rm = R - h;

        double s = sign_glsl( Rp ) * std::pow( std::fabs( Rp ), 1.0 / 3.0 );
        double u = sign_glsl( Rm ) * std::pow( std::fabs( Rm ), 1.0 / 3.0 );

        double x  = s + u + 4.0 * c2;
        double y  = s - u;
        double ks = x * x + 3.0 * y * y;
        if ( ks <= 0.0 )
            return -1.0;

        double k     = std::sqrt( ks );
        double denom = ks + x * k;
        if ( denom == 0.0 )
            return -1.0;

        double t = -0.5 * po * std::fabs( y ) * std::sqrt( 6.0 / ( k + x ) ) -
                   2.0 * c1 * ( k + x ) / denom - k3;

        return ( po < 0.0 ) ? ( 1.0 / t ) : t;
    }

    if ( Q <= 0.0 )
        return -1.0;

    double sQ  = std::sqrt( Q );
    double arg = -R / ( sQ * Q );
    arg        = std::clamp( arg, -1.0, 1.0 );
    double w   = sQ * std::cos( std::acos( arg ) / 3.0 );
    double d2  = -w - c2;
    if ( d2 < 0.0 )
        return -1.0;

    double d1 = std::sqrt( d2 );

    double h1_arg = w - 2.0 * c2 + c1 / d1;
    double h2_arg = w - 2.0 * c2 - c1 / d1;

    if ( h1_arg < 0.0 || h2_arg < 0.0 )
        return -1.0;

    double h1 = std::sqrt( h1_arg );
    double h2 = std::sqrt( h2_arg );

    double t1 = -d1 - h1 - k3;
    double t2 = -d1 + h1 - k3;
    double t3 = d1 - h2 - k3;
    double t4 = d1 + h2 - k3;

    if ( po < 0.0 )
    {
        if ( std::fabs( t1 ) > 1e-6 )
            t1 = 1.0 / t1;
        if ( std::fabs( t2 ) > 1e-6 )
            t2 = 1.0 / t2;
        if ( std::fabs( t3 ) > 1e-6 )
            t3 = 1.0 / t3;
        if ( std::fabs( t4 ) > 1e-6 )
            t4 = 1.0 / t4;
    }

    double t = 1e20;
    if ( t1 > 0.0 )
        t = t1;
    if ( t2 > 0.0 )
        t = std::min( t, t2 );
    if ( t3 > 0.0 )
        t = std::min( t, t3 );
    if ( t4 > 0.0 )
        t = std::min( t, t4 );

    if ( t == 1e20 )
        return -1.0;

    return t;
}

static float
gouIntersect( const Vector3f& ro_f, const Vector3f& rd_f, float ka_f, float kb_f )
{
    Vec3d ro = makeVec3d( ro_f );
    Vec3d rd = makeVec3d( rd_f );

    double ka = static_cast<double>( ka_f );
    double kb = static_cast<double>( kb_f );

    double t = gouIntersectInternal( ro, rd, ka, kb );
    if ( t <= 0.0 || !std::isfinite( t ) || t > 1e19 )
        return std::numeric_limits<float>::max();

    return static_cast<float>( t );
}

static float
computeBoundingRadius( float ka_f, float kb_f )
{
    double ka = static_cast<double>( ka_f );
    double kb = static_cast<double>( kb_f );

    double disc = kb * kb - 4.0 * ka;
    double R    = 1.0;

    if ( disc >= 0.0 )
    {
        double sd   = std::sqrt( disc );
        double y1   = 0.5 * ( kb + sd );
        double y2   = 0.5 * ( kb - sd );
        double yMax = std::max( 0.0, std::max( y1, y2 ) );
        R           = ( yMax > 0.0 ) ? std::sqrt( yMax ) : 1.0;
    } else
    {
        R = std::sqrt( std::fabs( kb ) + std::fabs( ka ) + 1.0 );
    }

    return static_cast<float>( R * 1.05 );
}

} // anonymous namespace

std::optional<Primitive::IntersectionInfo>
Goursat::calcRayIntersection( const Ray& ray ) const
{
    Vector3f ro = worldToLocalPoint( ray.getBasePoint() );
    Vector3f rd = worldToLocalDir( ray.getDir() );

    float t = gouIntersect( ro, rd, ka_, kb_ );

    if ( t >= std::numeric_limits<float>::max() || t < 0.0f )
    {
        fprintf( stderr, "FAILED\n" );

        return std::nullopt;
    }

    fprintf( stderr, "SUCCESS\n" );

    Primitive::IntersectionInfo info;
    info.close_distance = t;
    info.far_distance   = t;
    info.inside_object  = false;
    info.normal         = std::nullopt;

    return info;
}

Vector3f
Goursat::calcNormal( const Vector3f& point, bool /*inside_object*/ ) const
{
    Vector3f p = worldToLocalPoint( point );

    Vector3f p2( p.x * p.x, p.y * p.y, p.z * p.z );
    Vector3f p3( p2.x * p.x, p2.y * p.y, p2.z * p.z );

    Vector3f grad( 4.0f * p3.x - 2.0f * p.x * kb_,
                   4.0f * p3.y - 2.0f * p.y * kb_,
                   4.0f * p3.z - 2.0f * p.z * kb_ );

    grad.normalize();
    return localToWorldNormal( grad );
}

std::array<Vector3f, 8>
Goursat::getCircumscribedAABB() const
{
    float R  = computeBoundingRadius( ka_, kb_ );
    float dx = R;
    float dy = R;
    float dz = R;

    std::array<Vector3f, 8> corners_local = { { { -dx, -dy, -dz },
                                                { dx, -dy, -dz },
                                                { -dx, dy, -dz },
                                                { dx, dy, -dz },
                                                { -dx, -dy, dz },
                                                { dx, -dy, dz },
                                                { -dx, dy, dz },
                                                { dx, dy, dz } } };

    for ( auto& c : corners_local )
    {
        c = localToWorldPoint( c );
    }

    return corners_local;
}

} // namespace model
} // namespace rta
