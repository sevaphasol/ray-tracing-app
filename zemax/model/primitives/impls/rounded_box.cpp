
/*
    Rounded axis-aligned box (Inigo Quilez intersectors)
*/

#include "zemax/model/primitives/impls/rounded_box.hpp"
#include "zemax/model/rendering/vector3.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace zemax {
namespace model {

RoundedBox::RoundedBox( const Material& material,
                        const Vector3f& center,
                        const Vector3f& half_size,
                        float           radius )
    : Primitive( material, center ), half_size_( half_size ), radius_( radius )
{
}

namespace {

float
roundedboxIntersect( const Vector3f& ro, const Vector3f& rd, const Vector3f& size, float rad )
{
    // bounding box (как в шейдере, но с защитой от деления на ноль)
    Vector3f    m;
    const float big = std::numeric_limits<float>::max();
    m.x             = ( std::abs( rd.x ) > 1e-6f ) ? 1.0f / rd.x : big;
    m.y             = ( std::abs( rd.y ) > 1e-6f ) ? 1.0f / rd.y : big;
    m.z             = ( std::abs( rd.z ) > 1e-6f ) ? 1.0f / rd.z : big;

    Vector3f n = { m.x * ro.x, m.y * ro.y, m.z * ro.z };
    Vector3f k = { std::abs( m.x ) * ( size.x + rad ),
                   std::abs( m.y ) * ( size.y + rad ),
                   std::abs( m.z ) * ( size.z + rad ) };

    Vector3f t1 = { -n.x - k.x, -n.y - k.y, -n.z - k.z };
    Vector3f t2 = { -n.x + k.x, -n.y + k.y, -n.z + k.z };

    float tN = std::max( { t1.x, t1.y, t1.z } );
    float tF = std::min( { t2.x, t2.y, t2.z } );

    if ( tN > tF || tF < 0.0f )
        return std::numeric_limits<float>::max();

    float t = tN;

    // convert to first octant
    Vector3f pos = ro + rd * t;
    Vector3f s   = sign( pos );

    Vector3f ro1  = ro * s;
    Vector3f rd1  = rd * s;
    Vector3f pos1 = pos * s;

    // faces
    pos1 -= size;

    // pos = max( pos.xyz, pos.yzx );
    Vector3f p2{ pos1.y, pos1.z, pos1.x };
    pos1.x = std::max( pos1.x, p2.x );
    pos1.y = std::max( pos1.y, p2.y );
    pos1.z = std::max( pos1.z, p2.z );

    if ( std::min( { pos1.x, pos1.y, pos1.z } ) < 0.0f )
        return t;

    // some precomputation
    Vector3f oc  = ro1 - size;
    Vector3f dd  = { rd1.x * rd1.x, rd1.y * rd1.y, rd1.z * rd1.z };
    Vector3f oo  = { oc.x * oc.x, oc.y * oc.y, oc.z * oc.z };
    Vector3f od  = { oc.x * rd1.x, oc.y * rd1.y, oc.z * rd1.z };
    float    ra2 = rad * rad;

    t = std::numeric_limits<float>::max();

    // corner
    {
        float b = od.x + od.y + od.z;
        float c = oo.x + oo.y + oo.z - ra2;
        float h = b * b - c;
        if ( h > 0.0f )
        {
            float t_candidate = -b - std::sqrt( h );
            if ( t_candidate > 0.0f )
                t = t_candidate;
        }
    }

    // edge X
    {
        float a = dd.y + dd.z;
        float b = od.y + od.z;
        float c = oo.y + oo.z - ra2;
        float h = b * b - a * c;
        if ( h > 0.0f )
        {
            h = ( -b - std::sqrt( h ) ) / a;
            if ( h > 0.0f && h < t && std::abs( ro1.x + rd1.x * h ) < size.x )
                t = h;
        }
    }

    // edge Y
    {
        float a = dd.z + dd.x;
        float b = od.z + od.x;
        float c = oo.z + oo.x - ra2;
        float h = b * b - a * c;
        if ( h > 0.0f )
        {
            h = ( -b - std::sqrt( h ) ) / a;
            if ( h > 0.0f && h < t && std::abs( ro1.y + rd1.y * h ) < size.y )
                t = h;
        }
    }

    // edge Z
    {
        float a = dd.x + dd.y;
        float b = od.x + od.y;
        float c = oo.x + oo.y - ra2;
        float h = b * b - a * c;
        if ( h > 0.0f )
        {
            h = ( -b - std::sqrt( h ) ) / a;
            if ( h > 0.0f && h < t && std::abs( ro1.z + rd1.z * h ) < size.z )
                t = h;
        }
    }

    if ( t == std::numeric_limits<float>::max() || t > 1e19f )
        return std::numeric_limits<float>::max();

    return t;
}

} // anonymous namespace

std::optional<Primitive::IntersectionInfo>
RoundedBox::calcRayIntersection( const Ray& ray ) const
{
    Vector3f ro = ray.getBasePoint() - getOrigin();
    Vector3f rd = ray.getDir(); // уже нормализован

    float t = roundedboxIntersect( ro, rd, half_size_, radius_ );

    if ( t >= std::numeric_limits<float>::max() || t < 0.0f )
        return std::nullopt;

    Primitive::IntersectionInfo info;
    info.close_distance = t;
    info.far_distance   = t;
    info.inside_object  = false;
    info.normal         = std::nullopt; // нормаль посчитаем через calcNormal
    return info;
}

Vector3f
RoundedBox::calcNormal( const Vector3f& point, bool inside_object ) const
{
    // Шейдер-труд: sign(pos)*normalize(max(abs(pos)-siz,0.0))
    Vector3f p_local = point - getOrigin();
    Vector3f s       = sign( p_local );
    Vector3f ap      = abs( p_local );

    Vector3f q{ std::max( ap.x - half_size_.x, 0.0f ),
                std::max( ap.y - half_size_.y, 0.0f ),
                std::max( ap.z - half_size_.z, 0.0f ) };

    Vector3f n;

    if ( q.getLenSq() < 1e-8f )
    {
        // На почти плоской части — как у обычного AABB
        if ( ap.x >= ap.y && ap.x >= ap.z )
            n = Vector3f{ s.x, 0.0f, 0.0f };
        else if ( ap.y >= ap.z )
            n = Vector3f{ 0.0f, s.y, 0.0f };
        else
            n = Vector3f{ 0.0f, 0.0f, s.z };
    } else
    {
        q.normalize();
        n = Vector3f{ s.x * q.x, s.y * q.y, s.z * q.z };
    }

    if ( inside_object )
        n = -n;

    return n;
}

std::array<Vector3f, 8>
RoundedBox::getCircumscribedAABB() const
{
    Vector3f c   = getOrigin();
    Vector3f ext = half_size_ + Vector3f{ radius_, radius_, radius_ };

    return { { { c.x - ext.x, c.y - ext.y, c.z - ext.z },
               { c.x + ext.x, c.y - ext.y, c.z - ext.z },
               { c.x - ext.x, c.y + ext.y, c.z - ext.z },
               { c.x + ext.x, c.y + ext.y, c.z - ext.z },
               { c.x - ext.x, c.y - ext.y, c.z + ext.z },
               { c.x + ext.x, c.y - ext.y, c.z + ext.z },
               { c.x - ext.x, c.y + ext.y, c.z + ext.z },
               { c.x + ext.x, c.y + ext.y, c.z + ext.z } } };
}

} // namespace model
} // namespace zemax
