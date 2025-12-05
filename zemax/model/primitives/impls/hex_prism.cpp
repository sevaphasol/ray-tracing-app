/*

Spizheno s https://iquilezles.org/articles/intersectors/

*/

#include "zemax/model/primitives/impls/hex_prism.hpp"
#include "zemax/model/rendering/vector3.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace zemax {
namespace model {

HexPrism::HexPrism( const Material& material, const Vector3f& center, float radius, float height )
    : Primitive( material, center ), radius_( radius ), height_( height )
{
}

std::optional<Primitive::IntersectionInfo>
HexPrism::calcRayIntersection( const Ray& ray ) const
{
    const float ks3 = 0.8660254037844386f; // sqrt(3)/2

    Vector3f n1( 1.0f, 0.0f, 0.0f );
    Vector3f n2( 0.5f, 0.0f, ks3 );
    Vector3f n3( -0.5f, 0.0f, ks3 );
    Vector3f n4( 0.0f, 1.0f, 0.0f );

    Vector3f ro = ray.getBasePoint() - getOrigin();
    Vector3f rd = ray.getDir();

    auto slabIntersect = []( float           d,
                             const Vector3f& n,
                             const Vector3f& ro,
                             const Vector3f& rd ) -> std::pair<float, float> {
        float denom = scalarMul( rd, n );
        if ( std::abs( denom ) < 1e-6f )
        {
            return { -std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
        }
        float t1 = ( d - scalarMul( ro, n ) ) / denom;
        float t2 = ( -d - scalarMul( ro, n ) ) / denom;
        if ( t1 > t2 )
            std::swap( t1, t2 );
        return { t1, t2 };
    };

    auto [t1x, t1y] = slabIntersect( radius_, n1, ro, rd );
    auto [t2x, t2y] = slabIntersect( radius_, n2, ro, rd );
    auto [t3x, t3y] = slabIntersect( radius_, n3, ro, rd );
    auto [t4x, t4y] = slabIntersect( height_, n4, ro, rd );

    float tNear = std::max( { t1x, t2x, t3x, t4x } );
    float tFar  = std::min( { t1y, t2y, t3y, t4y } );

    if ( tNear > tFar || tFar < 0.0f )
    {
        return std::nullopt;
    }

    // Определяем, какая грань пересечена первой
    Vector3f normal;
    float    t = tNear;

    if ( t == t1x )
        normal = n1;
    else if ( t == t2x )
        normal = n2;
    else if ( t == t3x )
        normal = n3;
    else if ( t == t4x )
        normal = n4;
    else
    {
        // fallback: проверим все грани
        float eps = 1e-4f;
        if ( std::abs( t - t1x ) < eps )
            normal = n1;
        else if ( std::abs( t - t2x ) < eps )
            normal = n2;
        else if ( std::abs( t - t3x ) < eps )
            normal = n3;
        else
            normal = n4;
    }

    Primitive::IntersectionInfo info;
    info.close_distance = t;
    info.far_distance   = tFar;
    info.inside_object  = ( tNear <= 0.0f );
    info.normal         = normal;
    return info;
}

Vector3f
HexPrism::calcNormal( const Vector3f& point, bool /*inside_object*/ ) const
{
    // Для простоты используем нормаль из пересечения
    // (в реальности можно пересчитать, но в вашем движке normal берётся из IntersectionInfo)
    // Поэтому этот метод редко вызывается — оставим заглушку
    Vector3f    local = point - getOrigin();
    const float ks3   = 0.8660254037844386f;

    // Нормали граней
    struct
    {
        Vector3f n;
        float    d;
    } planes[] = { { Vector3f( 1.0f, 0.0f, 0.0f ), radius_ },
                   { Vector3f( 0.5f, 0.0f, ks3 ), radius_ },
                   { Vector3f( -0.5f, 0.0f, ks3 ), radius_ },
                   { Vector3f( -1.0f, 0.0f, 0.0f ), radius_ },
                   { Vector3f( -0.5f, 0.0f, -ks3 ), radius_ },
                   { Vector3f( 0.5f, 0.0f, -ks3 ), radius_ },
                   { Vector3f( 0.0f, 1.0f, 0.0f ), height_ },
                   { Vector3f( 0.0f, -1.0f, 0.0f ), height_ } };

    float    maxDist = -std::numeric_limits<float>::max();
    Vector3f bestNormal;

    for ( const auto& p : planes )
    {
        float dist = scalarMul( local, p.n ) - p.d;
        if ( dist > maxDist )
        {
            maxDist    = dist;
            bestNormal = p.n;
        }
    }

    return bestNormal;
}

std::array<Vector3f, 8>
HexPrism::getCircumscribedAABB() const
{
    auto  c  = getOrigin();
    float dx = radius_; // ra
    float dy = height_; // he
    float dz = radius_; // ra

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
