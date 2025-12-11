/*
    Ray–Torus intersection (robust numeric version)

    Имплицитная поверхность тора (ось Z, центр в начале координат):
    F(x,y,z) = (x^2 + y^2 + z^2 + Ra^2 - ra^2)^2 - 4*Ra^2*(x^2 + y^2) = 0

    Мы НЕ решаем квартетик аналитически (это и даёт артефакты),
    а ищем корень F(ro + t*rd) = 0 вдоль луча:
    1) пересекаем луч с bounding-сферой радиуса (Ra+ra),
    2) по отрезку [tEnter, tExit] ищем первый sign-change F(t_prev)*F(t_cur) <= 0
    3) внутри найденного интервала гоняем бисекцию.
*/

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

// Имплицитная функция тора (ось Z).
// F(p) > 0 — снаружи, F(p) < 0 — внутри (по сути не важно, нам важен лишь знак и ноль).
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

// Численное пересечение луча с тором.
// Возвращает t > 0 либо max() если хита нет.
float
torusIntersectNumeric( const Vector3f& ro, const Vector3f& rd, float Ra, float ra )
{
    constexpr float INF      = std::numeric_limits<float>::max();
    constexpr float EPS_T    = 1e-4f;
    constexpr int   STEPS    = 128; // сколько шагов по bounding-сфере
    constexpr int   BISECT_N = 16;  // итераций бисекции внутри одного интервала

    // 1) Пересечение с bounding-сферой радиуса (Ra+ra)
    float Rb   = Ra + ra;
    float Rb2  = Rb * Rb;
    float b    = scalarMul( ro, rd );       // n
    float c    = scalarMul( ro, ro ) - Rb2; // m - Rb^2
    float disc = b * b - c;
    if ( disc < 0.0f )
    {
        return INF; // луч сферы не касается -> точно мимо тора
    }

    float sqrt_disc = std::sqrt( disc );
    float tEnter    = -b - sqrt_disc;
    float tExit     = -b + sqrt_disc;

    if ( tExit < 0.0f )
    {
        return INF; // всё за камерой
    }
    if ( tEnter < 0.0f )
    {
        tEnter = 0.0f;
    }

    // 2) По отрезку [tEnter, tExit] делаем дискретные шаги и ищем первый sign-change
    auto F = [&]( float t ) -> float {
        Vector3f p = ro + rd * t;
        return torusImplicit( p, Ra, ra );
    };

    float tPrev = tEnter;
    float fPrev = F( tPrev );

    // размер шага — либо делим интервал, либо ориентируемся на толщину тора
    float totalLen = tExit - tEnter;
    if ( totalLen <= 0.0f )
    {
        return INF;
    }
    float step = totalLen / static_cast<float>( STEPS );
    // чтобы не проскочить слишком толстый тор, ограничим шаг сверху
    step = std::min( step, ra * 0.25f );

    for ( int i = 0; i < STEPS && tPrev < tExit; ++i )
    {
        float tCur = tPrev + step;
        if ( tCur > tExit )
            tCur = tExit;

        float fCur = F( tCur );

        // Ищем переход через ноль: F меняет знак или один из концов почти ноль
        bool sign_change =
            ( fPrev == 0.0f ) || ( fCur == 0.0f ) || ( ( fPrev > 0.0f ) != ( fCur > 0.0f ) );

        if ( sign_change )
        {
            // 3) Бисекция на [tPrev, tCur]
            float a  = tPrev;
            float b2 = tCur;
            float fa = fPrev;
            float fb = fCur;

            for ( int it = 0; it < BISECT_N; ++it )
            {
                float mid = 0.5f * ( a + b2 );
                float fm  = F( mid );

                // если один конец почти попал ровно в поверхность — сдвигаем интервал
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
                return tHit; // первый найденный хит — и есть ближняя поверхность тора
            }
            // если tHit совсем близко к нулю — мы почти стартуем с поверхности;
            // продолжаем поиск дальше, чтобы не ловить самих себя
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
    // локальное пространство тора (центр в getOrigin(), ось вокруг Z)
    Vector3f ro = worldToLocalPoint( ray.getBasePoint() );
    Vector3f rd = worldToLocalDir( ray.getDir() ); // Ray уже нормализует dir_ в конструкторе

    float t = torusIntersectNumeric( ro, rd, major_radius_, minor_radius_ );

    if ( t >= std::numeric_limits<float>::max() || t < 0.0f )
    {
        return std::nullopt;
    }

    Primitive::IntersectionInfo info;
    info.close_distance = t;
    info.far_distance   = t;
    info.inside_object  = false;
    info.normal         = std::nullopt; // нормаль посчитаем через calcNormal()
    return info;
}

Vector3f
Torus::calcNormal( const Vector3f& point, bool /*inside_object*/ ) const
{
    // Имплицитная поверхность тора (ось Z):
    // F(x,y,z) = (x^2 + y^2 + z^2 + Ra^2 - ra^2)^2 - 4*Ra^2*(x^2 + y^2) = 0
    // ∇F ~ ( x*(s - 2*Ra^2), y*(s - 2*Ra^2), z*s ), где s = x^2 + y^2 + z^2 + Ra^2 - ra^2

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
