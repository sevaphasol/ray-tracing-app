#include "rta/model/primitives/impls/torus.hpp"
#include "rta/model/rendering/vector3.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace rta {
namespace model {

Torus::Torus( const Material& material,
              const Vector3f& center,
              float           major_radius,
              float           minor_radius )
    : Primitive( material, center ), major_radius_( major_radius ), minor_radius_( minor_radius )
{
}

namespace {

inline float
torusImplicit( const Vector3f& p, float Ra, float ra )
{
    float Ra2  = Ra * Ra;
    float ra2  = ra * ra;
    float len2 = scalarMul( p, p ); // x^2 + y^2 + z^2
    float s    = len2 + Ra2 - ra2;
    float xy2  = p.x * p.x + p.y * p.y;
    return s * s - 4.0f * Ra2 * xy2;
}

float
torusIntersectNumeric( const Vector3f& ro, const Vector3f& rd, float Ra, float ra )
{
    constexpr float INF      = std::numeric_limits<float>::max();
    constexpr float EPS_T    = 1e-4f;
    constexpr int   STEPS    = 128;
    constexpr int   BISECT_N = 16;

    float Rb   = Ra + ra;
    float Rb2  = Rb * Rb;
    float b    = scalarMul( ro, rd );
    float c    = scalarMul( ro, ro ) - Rb2;
    float disc = b * b - c;
    if ( disc < 0.0f )
    {
        return INF;
    }

    float sqrt_disc = std::sqrt( disc );
    float tEnter    = -b - sqrt_disc;
    float tExit     = -b + sqrt_disc;

    if ( tExit < 0.0f )
    {
        return INF;
    }
    if ( tEnter < 0.0f )
    {
        tEnter = 0.0f;
    }

    auto F = [&]( float t ) -> float {
        Vector3f p = ro + rd * t;
        return torusImplicit( p, Ra, ra );
    };

    float tPrev = tEnter;
    float fPrev = F( tPrev );

    float totalLen = tExit - tEnter;
    if ( totalLen <= 0.0f )
    {
        return INF;
    }
    float step = totalLen / static_cast<float>( STEPS );
    step       = std::min( step, ra * 0.25f );

    for ( int i = 0; i < STEPS && tPrev < tExit; ++i )
    {
        float tCur = tPrev + step;
        if ( tCur > tExit )
            tCur = tExit;

        float fCur = F( tCur );

        bool sign_change =
            ( fPrev == 0.0f ) || ( fCur == 0.0f ) || ( ( fPrev > 0.0f ) != ( fCur > 0.0f ) );

        if ( sign_change )
        {
            float a  = tPrev;
            float b2 = tCur;
            float fa = fPrev;
            float fb = fCur;

            for ( int it = 0; it < BISECT_N; ++it )
            {
                float mid = 0.5f * ( a + b2 );
                float fm  = F( mid );

                if ( std::fabs( fm ) < 1e-6f )
                {
                    a  = mid;
                    fa = fm;
                    break;
                }

                bool same_sign = ( fa > 0.0f ) == ( fm > 0.0f );
                if ( same_sign )
                {
                    a  = mid;
                    fa = fm;
                } else
                {
                    b2 = mid;
                    fb = fm;
                }
            }

            float tHit = 0.5f * ( a + b2 );

            if ( tHit > EPS_T )
            {
                return tHit;
            }
        }

        tPrev = tCur;
        fPrev = fCur;
    }

    return INF;
}

} // anonymous namespace

std::optional<Primitive::IntersectionInfo>
Torus::calcRayIntersection( const Ray& ray ) const
{
    Vector3f ro = worldToLocalPoint( ray.getBasePoint() );
    Vector3f rd = worldToLocalDir( ray.getDir() );

    float t = torusIntersectNumeric( ro, rd, major_radius_, minor_radius_ );

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
    Vector3f p   = worldToLocalPoint( point );
    float    Ra2 = major_radius_ * major_radius_;
    float    ra2 = minor_radius_ * minor_radius_;

    float len2 = scalarMul( p, p );
    float s    = len2 + Ra2 - ra2;

    Vector3f grad( p.x * ( s - 2.0f * Ra2 ), p.y * ( s - 2.0f * Ra2 ), p.z * s );

    grad.normalize();
    return localToWorldNormal( grad );
}

std::array<Vector3f, 8>
Torus::getCircumscribedAABB() const
{
    float dx = major_radius_ + minor_radius_;
    float dy = major_radius_ + minor_radius_;
    float dz = minor_radius_;

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
