/*
    Capsule (Inigo Quilez capIntersect)
*/

#include "rta/model/primitives/impls/capsule.hpp"
#include "rta/model/rendering/vector3.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace rta {
namespace model {

Capsule::Capsule( const Material& material,
                  const Vector3f& center,
                  const Vector3f& pa_local,
                  const Vector3f& pb_local,
                  float           radius )
    : Primitive( material, center ), pa_local_( pa_local ), pb_local_( pb_local ), radius_( radius )
{
}

namespace {

float
capIntersect( const Vector3f& ro,
              const Vector3f& rd,
              const Vector3f& pa,
              const Vector3f& pb,
              float           ra )
{
    Vector3f ba   = pb - pa;
    Vector3f oa   = ro - pa;
    float    baba = scalarMul( ba, ba );
    float    bard = scalarMul( ba, rd );
    float    baoa = scalarMul( ba, oa );
    float    rdoa = scalarMul( rd, oa );
    float    oaoa = scalarMul( oa, oa );

    float a = baba - bard * bard;
    float b = baba * rdoa - baoa * bard;
    float c = baba * oaoa - baoa * baoa - ra * ra * baba;
    float h = b * b - a * c;
    if ( h >= 0.0f )
    {
        h       = std::sqrt( h );
        float t = ( -b - h ) / a;
        float y = baoa + t * bard;
        if ( y > 0.0f && y < baba )
            return t;
        Vector3f oc = ( y <= 0.0f ) ? oa : ( ro - pb );
        b           = scalarMul( rd, oc );
        c           = scalarMul( oc, oc ) - ra * ra;
        h           = b * b - c;
        if ( h > 0.0f )
        {
            float t2 = -b - std::sqrt( h );
            if ( t2 > 0.0f )
                return t2;
        }
    }
    return std::numeric_limits<float>::max();
}

} // anonymous namespace

std::optional<Primitive::IntersectionInfo>
Capsule::calcRayIntersection( const Ray& ray ) const
{
    Vector3f ro = worldToLocalPoint( ray.getBasePoint() );
    Vector3f rd = worldToLocalDir( ray.getDir() );

    float t = capIntersect( ro, rd, pa_local_, pb_local_, radius_ );

    if ( t >= std::numeric_limits<float>::max() || t < 0.0f )
        return std::nullopt;

    Primitive::IntersectionInfo info;
    info.close_distance = t;
    info.far_distance   = t;
    info.inside_object  = false;
    info.normal         = std::nullopt;

    return info;
}

Vector3f
Capsule::calcNormal( const Vector3f& point, bool inside_object ) const
{
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

    Vector3f n_local;

    if ( s < 0.0f )
    {
        n_local = ( p_local - pa ).normalize();
    } else if ( s > 1.0f )
    {
        n_local = ( p_local - pb ).normalize();
    } else
    {
        Vector3f c = pa + ba * s;
        n_local    = ( p_local - c ).normalize();
    }

    if ( inside_object )
        n_local = -n_local;

    return localToWorldNormal( n_local );
}

std::array<Vector3f, 8>
Capsule::getCircumscribedAABB() const
{
    Vector3f pa = pa_local_;
    Vector3f pb = pb_local_;

    Vector3f min_local{ std::min( pa.x, pb.x ) - radius_,
                        std::min( pa.y, pb.y ) - radius_,
                        std::min( pa.z, pb.z ) - radius_ };

    Vector3f max_local{ std::max( pa.x, pb.x ) + radius_,
                        std::max( pa.y, pb.y ) + radius_,
                        std::max( pa.z, pb.z ) + radius_ };

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
