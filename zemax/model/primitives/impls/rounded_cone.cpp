/*
    Rounded cone (Inigo Quilez iRoundedCone)
*/

#include "zemax/model/primitives/impls/rounded_cone.hpp"
#include "zemax/model/rendering/vector3.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace zemax {
namespace model {

RoundedCone::RoundedCone( const Material& material,
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

// Возвращает t и нормаль (локальная), либо t=max() если нет хита.
float
iRoundedCone( const Vector3f& ro,
              const Vector3f& rd,
              const Vector3f& pa,
              const Vector3f& pb,
              float           ra,
              float           rb,
              Vector3f&       outNormal )
{
    Vector3f ba = pb - pa;
    Vector3f oa = ro - pa;
    Vector3f ob = ro - pb;

    float rr = ra - rb;

    float m0 = scalarMul( ba, ba );
    float m1 = scalarMul( ba, oa );
    float m2 = scalarMul( ba, rd );
    float m3 = scalarMul( rd, oa );
    float m5 = scalarMul( oa, oa );
    float m6 = scalarMul( ob, rd );
    float m7 = scalarMul( ob, ob );

    // body
    float d2 = m0 - rr * rr;
    float k2 = d2 - m2 * m2;
    float k1 = d2 * m3 - m1 * m2 + m2 * rr * ra;
    float k0 = d2 * m5 - m1 * m1 + m1 * rr * ra * 2.0f - m0 * ra * ra;
    float h  = k1 * k1 - k0 * k2;

    float t = std::numeric_limits<float>::max();

    if ( h >= 0.0f )
    {
        h            = std::sqrt( h );
        float t_body = ( -h - k1 ) / k2;
        float y      = m1 - ra * rr + t_body * m2;
        if ( t_body > 0.0f && y > 0.0f && y < d2 )
        {
            t          = t_body;
            Vector3f n = d2 * ( oa + rd * t ) - ba * y;
            outNormal  = n.normalize();
            return t;
        }
    }

    // caps
    float h1 = m3 * m3 - m5 + ra * ra;
    float h2 = m6 * m6 - m7 + rb * rb;
    if ( std::max( h1, h2 ) < 0.0f )
        return std::numeric_limits<float>::max();

    t = std::numeric_limits<float>::max();
    Vector3f n_best;

    if ( h1 > 0.0f )
    {
        float s  = std::sqrt( h1 );
        float tc = -m3 - s;
        if ( tc > 0.0f )
        {
            t          = tc;
            Vector3f p = oa + rd * t;
            n_best     = ( p / ra ).normalize();
        }
    }

    if ( h2 > 0.0f )
    {
        float s  = std::sqrt( h2 );
        float tc = -m6 - s;
        if ( tc > 0.0f && tc < t )
        {
            t          = tc;
            Vector3f p = ob + rd * t;
            n_best     = ( p / rb ).normalize();
        }
    }

    if ( t == std::numeric_limits<float>::max() )
        return t;

    outNormal = n_best;
    return t;
}

} // anonymous namespace

std::optional<Primitive::IntersectionInfo>
RoundedCone::calcRayIntersection( const Ray& ray ) const
{
    Vector3f ro = worldToLocalPoint( ray.getBasePoint() );
    Vector3f rd = worldToLocalDir( ray.getDir() );

    Vector3f pa = pa_local_;
    Vector3f pb = pb_local_;

    Vector3f n_local;
    float    t = iRoundedCone( ro, rd, pa, pb, ra_, rb_, n_local );

    if ( t >= std::numeric_limits<float>::max() || t < 0.0f )
        return std::nullopt;

    Primitive::IntersectionInfo info;
    info.close_distance = t;
    info.far_distance   = t;
    info.inside_object  = false;
    info.normal         = localToWorldNormal( n_local ); // даём движку готовую нормаль

    return info;
}

Vector3f
RoundedCone::calcNormal( const Vector3f& point, bool inside_object ) const
{
    // Грубая аппроксимация на случай, если нормаль не была сохранена
    // (по факту движок возьмёт нормаль из IntersectionInfo).
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
    float    s  = y / baba;

    // ограничим на отрезке
    s = std::clamp( s, 0.0f, 1.0f );

    Vector3f c = pa + ba * s;
    Vector3f v = ( p_local - c ).normalize();

    Vector3f n = v;
    if ( inside_object )
        n = -n;
    return localToWorldNormal( n );
}

std::array<Vector3f, 8>
RoundedCone::getCircumscribedAABB() const
{
    // Очень грубый AABB: берём концы + оба радиуса
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
} // namespace zemax
