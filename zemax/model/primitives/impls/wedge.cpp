/*
    Wedge — iWedge from Shadertoy:

    vec4 iWedge( vec3 ro, vec3 rd, vec3 s )
*/

#include "zemax/model/primitives/impls/wedge.hpp"
#include "zemax/model/rendering/vector3.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace zemax {
namespace model {

Wedge::Wedge( const Material& material, const Vector3f& center, const Vector3f& s )
    : Primitive( material, center ), s_( s )
{
}

namespace {

inline float
signf( float x )
{
    return ( x > 0.0f ) ? 1.0f : ( x < 0.0f ? -1.0f : 0.0f );
}

} // anonymous namespace

std::optional<Primitive::IntersectionInfo>
Wedge::calcRayIntersection( const Ray& ray ) const
{
    constexpr float INF = std::numeric_limits<float>::max();

    Vector3f ro = ray.getBasePoint() - getOrigin();
    Vector3f rd = ray.getDir();

    // intersect plane box (axis-aligned box [-s,+s])
    Vector3f    m;
    const float big = std::numeric_limits<float>::max();
    m.x             = ( std::fabs( rd.x ) > 1e-6f ) ? 1.0f / rd.x : big;
    m.y             = ( std::fabs( rd.y ) > 1e-6f ) ? 1.0f / rd.y : big;
    m.z             = ( std::fabs( rd.z ) > 1e-6f ) ? 1.0f / rd.z : big;

    Vector3f z{ rd.x >= 0.0f ? 1.0f : -1.0f,
                rd.y >= 0.0f ? 1.0f : -1.0f,
                rd.z >= 0.0f ? 1.0f : -1.0f };

    Vector3f k = Vector3f{ s_.x * z.x, s_.y * z.y, s_.z * z.z };

    Vector3f t1 = ( Vector3f{ -ro.x - k.x, -ro.y - k.y, -ro.z - k.z } ) * m;
    Vector3f t2 = ( Vector3f{ -ro.x + k.x, -ro.y + k.y, -ro.z + k.z } ) * m;

    float tn = std::max( { t1.x, t1.y, t1.z } );
    float tf = std::min( { t2.x, t2.y, t2.z } );

    if ( tn > tf || tf < 0.0f )
        return std::nullopt;

    // boolean with plane
    float k1 = s_.y * ro.x - s_.x * ro.y;
    float k2 = s_.x * rd.y - s_.y * rd.x;
    if ( std::fabs( k2 ) < 1e-6f )
    {
        // луч почти параллелен разделяющей плоскости
        // остаётся только бокс
        if ( tn <= 1e-4f )
            return std::nullopt;

        // выбираем нормаль грани бокса
        Vector3f n_box( 0.0f, 0.0f, 0.0f );
        if ( tn == t1.x )
            n_box = Vector3f{ -z.x, 0.0f, 0.0f };
        else if ( tn == t1.y )
            n_box = Vector3f{ 0.0f, -z.y, 0.0f };
        else
            n_box = Vector3f{ 0.0f, 0.0f, -z.z };

        Primitive::IntersectionInfo info;
        info.close_distance = tn;
        info.far_distance   = tn;
        info.inside_object  = false;
        info.normal         = n_box;

        return info;
    }

    float tp = k1 / k2;

    // условие, как в шейдере:
    if ( k1 > tn * k2 )
    {
        // box
        if ( tn <= 1e-4f )
            return std::nullopt;

        Vector3f n_box( 0.0f, 0.0f, 0.0f );
        if ( tn == t1.x )
            n_box = Vector3f{ -z.x, 0.0f, 0.0f };
        else if ( tn == t1.y )
            n_box = Vector3f{ 0.0f, -z.y, 0.0f };
        else
            n_box = Vector3f{ 0.0f, 0.0f, -z.z };

        Primitive::IntersectionInfo info;
        info.close_distance = tn;
        info.far_distance   = tn;
        info.inside_object  = false;
        info.normal         = n_box;

        return info;
    }

    if ( tp > tn && tp < tf && tp > 1e-4f )
    {
        // hit плоскости
        Vector3f n_plane( -s_.y, s_.x, 0.0f );
        n_plane.normalize();

        Primitive::IntersectionInfo info;
        info.close_distance = tp;
        info.far_distance   = tp;
        info.inside_object  = false;
        info.normal         = n_plane;

        return info;
    }

    return std::nullopt;
}

Vector3f
Wedge::calcNormal( const Vector3f& point, bool inside_object ) const
{
    // Нормаль уже кладём в IntersectionInfo, так что здесь — fallback:
    // heuristics: если точка ближе к плоскости — возвращаем её нормаль, иначе нормаль бокса.

    Vector3f p_local = point - getOrigin();

    // плоскость клина: s.y * x - s.x * y = 0
    float d_plane    = s_.y * p_local.x - s_.x * p_local.y;
    float dist_plane = std::fabs( d_plane );

    // расстояние до граней бокса
    float dx       = std::max( 0.0f, std::fabs( p_local.x ) - s_.x );
    float dy       = std::max( 0.0f, std::fabs( p_local.y ) - s_.y );
    float dz       = std::max( 0.0f, std::fabs( p_local.z ) - s_.z );
    float dist_box = std::sqrt( dx * dx + dy * dy + dz * dz );

    Vector3f n;
    if ( dist_plane < dist_box )
    {
        n = Vector3f( -s_.y, s_.x, 0.0f ).normalize();
    } else
    {
        // выбираем по максимальному |координата|
        float ax = std::fabs( p_local.x ) - s_.x;
        float ay = std::fabs( p_local.y ) - s_.y;
        float az = std::fabs( p_local.z ) - s_.z;

        if ( ax >= ay && ax >= az )
            n = Vector3f( signf( p_local.x ), 0.0f, 0.0f );
        else if ( ay >= az )
            n = Vector3f( 0.0f, signf( p_local.y ), 0.0f );
        else
            n = Vector3f( 0.0f, 0.0f, signf( p_local.z ) );
    }

    if ( inside_object )
        n = -n;
    return n;
}

std::array<Vector3f, 8>
Wedge::getCircumscribedAABB() const
{
    Vector3f c = getOrigin();
    Vector3f s = s_;

    return { { { c.x - s.x, c.y - s.y, c.z - s.z },
               { c.x + s.x, c.y - s.y, c.z - s.z },
               { c.x - s.x, c.y + s.y, c.z - s.z },
               { c.x + s.x, c.y + s.y, c.z - s.z },
               { c.x - s.x, c.y - s.y, c.z + s.z },
               { c.x + s.x, c.y - s.y, c.z + s.z },
               { c.x - s.x, c.y + s.y, c.z + s.z },
               { c.x + s.x, c.y + s.y, c.z + s.z } } };
}

} // namespace model
} // namespace zemax
