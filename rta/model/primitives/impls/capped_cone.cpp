/*
    Capped cone (not rounded) — Shadertoy coneIntersect.

    vec4 coneIntersect( vec3 ro, vec3 rd, vec3 pa, vec3 pb, float ra, float rb )
*/

#include "rta/model/primitives/impls/capped_cone.hpp"
#include "rta/model/rendering/vector3.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace rta {
namespace model {

CappedCone::CappedCone( const Material& material,
                        const Vector3f& center,
                        const Vector3f& pa_local,
                        const Vector3f& pb_local,
                        float           ra,
                        float           rb )
    : Primitive( material, center ),
      pa_local_( pa_local ),
      pb_local_( pb_local ),
      ra_( ra ),
      rb_( rb )
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

// Возвращает t и нормаль в ЛОКАЛЬНЫХ координатах, либо t=max() если нет пересечения.
float
coneIntersect( const Vector3f& ro,
               const Vector3f& rd,
               const Vector3f& pa,
               const Vector3f& pb,
               float           ra,
               float           rb,
               Vector3f&       outNormal )
{
    constexpr float INF = std::numeric_limits<float>::max();

    Vector3f ba = pb - pa;
    Vector3f oa = ro - pa;
    Vector3f ob = ro - pb;

    float m0 = scalarMul( ba, ba );
    float m1 = scalarMul( oa, ba );
    float m2 = scalarMul( rd, ba );
    float m3 = scalarMul( rd, oa );
    float m5 = scalarMul( oa, oa );
    float m9 = scalarMul( ob, ba );

    // caps
    if ( m1 < 0.0f )
    {
        Vector3f tmp = oa * m2 - rd * m1;
        if ( dot2( tmp ) < ( ra * ra * m2 * m2 ) ) // delayed division
        {
            float denom = m2;
            if ( std::fabs( denom ) < 1e-6f )
                return INF;
            float    t   = -m1 / denom;
            Vector3f n   = -ba;
            float    len = std::sqrt( m0 );
            if ( len > 0.0f )
                n = n * ( 1.0f / len );
            outNormal = n;
            return ( t > 1e-4f ) ? t : INF;
        }
    } else if ( m9 > 0.0f )
    {
        float denom = m2;
        if ( std::fabs( denom ) >= 1e-6f )
        {
            float    t = -m9 / denom; // NOT delayed division
            Vector3f p = ob + rd * t;
            if ( dot2( p ) < rb * rb )
            {
                Vector3f n   = ba;
                float    len = std::sqrt( m0 );
                if ( len > 0.0f )
                    n = n * ( 1.0f / len );
                outNormal = n;
                return ( t > 1e-4f ) ? t : INF;
            }
        }
    }

    // body
    float rr = ra - rb;
    float hy = m0 + rr * rr;
    float k2 = m0 * m0 - m2 * m2 * hy;
    float k1 = m0 * m0 * m3 - m1 * m2 * hy + m0 * ra * ( rr * m2 );
    float k0 = m0 * m0 * m5 - m1 * m1 * hy + m0 * ra * ( rr * m1 * 2.0f - m0 * ra );
    float h  = k1 * k1 - k2 * k0;
    if ( h < 0.0f || std::fabs( k2 ) < 1e-6f )
        return INF;

    h       = std::sqrt( h );
    float t = ( -k1 - h ) / k2;
    if ( t <= 1e-4f )
        return INF;

    float y = m1 + t * m2;
    if ( y < 0.0f || y > m0 )
        return INF;

    // нормаль тела
    Vector3f n = m0 * ( m0 * ( oa + rd * t ) + rr * ba * ra ) - ba * hy * y;
    outNormal  = n.normalize();
    return t;
}

} // anonymous namespace

std::optional<Primitive::IntersectionInfo>
CappedCone::calcRayIntersection( const Ray& ray ) const
{
    Vector3f ro_local = worldToLocalPoint( ray.getBasePoint() );
    Vector3f rd       = worldToLocalDir( ray.getDir() );

    Vector3f n_local;
    float    t = coneIntersect( ro_local, rd, pa_local_, pb_local_, ra_, rb_, n_local );

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
CappedCone::calcNormal( const Vector3f& point, bool inside_object ) const
{
    // Грубый пересчёт нормали по локальной геометрии:
    Vector3f p_local = worldToLocalPoint( point );
    Vector3f pa      = pa_local_;
    Vector3f pb      = pb_local_;
    Vector3f ba      = pb - pa;

    float baba = scalarMul( ba, ba );
    if ( baba < 1e-6f )
    {
        Vector3f n = ( p_local - pa ).normalize();
        if ( inside_object )
            n = -n;
        return n;
    }

    Vector3f oa = p_local - pa;
    float    y  = scalarMul( ba, oa );
    float    h  = y / baba;

    Vector3f n;
    if ( h < 0.0f )
    {
        n = -ba.normalize(); // нижняя крышка
    } else if ( h > 1.0f )
    {
        n = ba.normalize();  // верхняя крышка
    } else
    {
        // тело — просто берём градиент от осевой линии, без учёта изменения радиуса
        Vector3f c = pa + ba * h;
        n          = ( p_local - c ).normalize();
    }

    if ( inside_object )
        n = -n;
    return localToWorldNormal( n );
}

std::array<Vector3f, 8>
CappedCone::getCircumscribedAABB() const
{
    Vector3f pa   = pa_local_;
    Vector3f pb   = pb_local_;
    float    rmax = std::max( ra_, rb_ );

    Vector3f min_local{ std::min( pa.x, pb.x ) - rmax,
                        std::min( pa.y, pb.y ) - rmax,
                        std::min( pa.z, pb.z ) - rmax };

    Vector3f max_local{ std::max( pa.x, pb.x ) + rmax,
                        std::max( pa.y, pb.y ) + rmax,
                        std::max( pa.z, pb.z ) + rmax };

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
} // namespace rta
