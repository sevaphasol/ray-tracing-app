#pragma once

#include <cmath>

namespace rta {
namespace model {

template<typename T>
class Vector3 {
  public:
    T x{};
    T y{};
    T z{};

    constexpr Vector3() : x( 0 ), y( 0 ), z( 0 ) {}

    constexpr Vector3( T X, T Y, T Z ) : x( X ), y( Y ), z( Z ) {}

    template<typename U>
    constexpr Vector3( const Vector3<U>& vector )
        : x( static_cast<T>( vector.x ) ),
          y( static_cast<T>( vector.y ) ),
          z( static_cast<T>( vector.z ) )
    {
    }

    constexpr friend Vector3<T>
    operator-( const Vector3<T>& right )
    {
        return Vector3<T>( -right.x, -right.y, -right.z );
    }

    constexpr friend Vector3<T>&
    operator+=( Vector3<T>& left, const Vector3<T>& right )
    {
        left.x += right.x;
        left.y += right.y;
        left.z += right.z;

        return left;
    }

    constexpr friend Vector3<T>&
    operator-=( Vector3<T>& left, const Vector3<T>& right )
    {
        left.x -= right.x;
        left.y -= right.y;
        left.z -= right.z;

        return left;
    }

    constexpr friend Vector3<T>
    operator+( const Vector3<T>& left, const Vector3<T>& right )
    {
        return Vector3<T>( left.x + right.x, left.y + right.y, left.z + right.z );
    }

    constexpr friend Vector3<T>
    operator-( const Vector3<T>& left, const Vector3<T>& right )
    {
        return Vector3<T>( left.x - right.x, left.y - right.y, left.z - right.z );
    }

    constexpr friend Vector3<T>
    operator*( const Vector3<T>& left, T right )
    {
        return Vector3<T>( left.x * right, left.y * right, left.z * right );
    }

    constexpr friend Vector3<T>
    operator*( T left, const Vector3<T>& right )
    {
        return Vector3<T>( right.x * left, right.y * left, right.z * left );
    }

    constexpr friend Vector3<T>
    operator*( const Vector3<T>& left, const Vector3<T>& right )
    {
        return Vector3<T>( left.x * right.x, left.y * right.y, left.z * right.z );
    }

    constexpr friend Vector3<T>&
    operator*=( Vector3<T>& left, T right )
    {
        left.x *= right;
        left.y *= right;
        left.z *= right;

        return left;
    }

    constexpr friend Vector3<T>
    operator/( const Vector3<T>& left, const Vector3<T>& right )
    {
        return Vector3<T>( left.x / right.x, left.y / right.y, left.z / right.z );
    }

    constexpr friend Vector3<T>
    operator/=( Vector3<T>& left, const Vector3<T>& right )
    {
        left.x /= right.x;
        left.y /= right.y;
        left.z /= right.z;

        return left;
    }

    constexpr friend Vector3<T>
    operator/( const Vector3<T>& left, T right )
    {
        return Vector3<T>( left.x / right, left.y / right, left.z / right );
    }

    constexpr friend Vector3<T>&
    operator/=( Vector3<T>& left, T right )
    {
        left.x /= right;
        left.y /= right;
        left.z /= right;

        return left;
    }

    constexpr friend Vector3<T>
    operator/( T left, const Vector3<T>& right )
    {
        return Vector3<T>( left / right.x, left / right.y, left / right.z );
    }

    constexpr friend Vector3<T>&
    operator/=( T left, const Vector3<T>& right )
    {
        return left / right;
    }

    constexpr friend bool
    operator==( const Vector3<T>& left, const Vector3<T>& right )
    {
        return ( left.x == right.x ) && ( left.y == right.y ) && ( left.z == right.z );
    }

    constexpr friend bool
    operator!=( const Vector3<T>& left, const Vector3<T>& right )
    {
        return ( left.x != right.x ) || ( left.y != right.y ) || ( left.z != right.z );
    }

    constexpr T&
    operator[]( size_t idx )
    {
        switch ( idx )
        {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
            default:
                return z;
        }
    }

    constexpr const T&
    operator[]( size_t idx ) const
    {
        switch ( idx )
        {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
            default:
                return z;
        }
    }

    constexpr bool
    valid() const
    {
        return !( std::isnan( x ) || std::isnan( y ) || std::isnan( z ) );
    }

    constexpr friend T
    scalarMul( const Vector3<T> left, const Vector3<T> right )
    {
        return left.x * right.x + left.y * right.y + left.z * right.z;
    }

    constexpr friend Vector3
    cross( const Vector3& a, const Vector3& b )
    {
        return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
    }

    constexpr friend T
    calcCos( const Vector3<T>& left, const Vector3<T>& right )
    {
        return scalarMul( left, right ) / ( left.getLen() * right.getLen() );
    }

    constexpr friend float
    calcSin( const Vector3<T>& left, const Vector3<T>& right )
    {
        return std::sin( std::acos( calcCos( left, right ) ) );
    }

    constexpr friend float
    vectorMulModule( const Vector3<T>& left, const Vector3<T>& right )
    {
        return left.getLen() * right.getLen() * calcSin( left, right );
    }

    constexpr friend Vector3
    abs( const Vector3& vec )
    {
        return { std::abs( vec.x ), std::abs( vec.y ), std::abs( vec.z ) };
    }

    constexpr friend Vector3
    sign( const Vector3& v )
    {
        return { static_cast<T>( v.x > 0.0f ) - static_cast<T>( v.x < 0.0f ),
                 static_cast<T>( v.y > 0.0f ) - static_cast<T>( v.y < 0.0f ),
                 static_cast<T>( v.z > 0.0f ) - static_cast<T>( v.z < 0.0f ) };
    }

    constexpr friend Vector3
    step( const Vector3& edge, const Vector3& x )
    {
        return { ( x.x >= edge.x ) ? 1.0f : 0.0f,
                 ( x.y >= edge.y ) ? 1.0f : 0.0f,
                 ( x.z >= edge.z ) ? 1.0f : 0.0f };
    }

    constexpr float
    getLenSq() const
    {
        return scalarMul( *this, *this );
    }

    constexpr float
    getLen() const
    {
        return std::sqrt( getLenSq() );
    }

    constexpr Vector3<T>
    normalize()
    {
        float len = getLen();
        x /= len;
        y /= len;
        z /= len;

        return *this;
    }

    constexpr Vector3<T>
    calcReflected( const Vector3<T>& normal ) const
    {
        return *this - normal * ( 2.0 * scalarMul( *this, normal ) );
    }

    static const Vector3<T> Nan;
};

template<typename T>
inline const Vector3<T> Vector3<T>::Nan( std::numeric_limits<T>::quiet_NaN(),
                                         std::numeric_limits<T>::quiet_NaN(),
                                         std::numeric_limits<T>::quiet_NaN() );

using Vector3f = Vector3<float>;
using Vector3i = Vector3<int>;
using Vector3u = Vector3<unsigned int>;

} // namespace model
} // namespace rta
