#pragma once

#include <cmath>

namespace zemax {
namespace model {

template<typename T>
class Vector2 {
  public:
    T x{};
    T y{};

    constexpr Vector2() : x( 0 ), y( 0 ) {}

    constexpr Vector2( T X, T Y ) : x( X ), y( Y ) {}

    template<typename U>
    constexpr Vector2( const Vector2<U>& vector )
        : x( static_cast<T>( vector.x ) ), y( static_cast<T>( vector.y ) )
    {
    }

    constexpr friend Vector2<T>
    operator-( const Vector2<T>& right )
    {
        return Vector2<T>( -right.x, -right.y );
    }

    constexpr friend Vector2<T>&
    operator+=( Vector2<T>& left, const Vector2<T>& right )
    {
        left.x += right.x;
        left.y += right.y;

        return left;
    }

    constexpr friend Vector2<T>&
    operator-=( Vector2<T>& left, const Vector2<T>& right )
    {
        left.x -= right.x;
        left.y -= right.y;

        return left;
    }

    constexpr friend Vector2<T>
    operator+( const Vector2<T>& left, const Vector2<T>& right )
    {
        return Vector2<T>( left.x + right.x, left.y + right.y );
    }

    constexpr friend Vector2<T>
    operator-( const Vector2<T>& left, const Vector2<T>& right )
    {
        return Vector2<T>( left.x - right.x, left.y - right.y );
    }

    constexpr friend Vector2<T>
    operator*( const Vector2<T>& left, T right )
    {
        return Vector2<T>( left.x * right, left.y * right );
    }

    constexpr friend Vector2<T>
    operator*( T left, const Vector2<T>& right )
    {
        return Vector2<T>( right.x * left, right.y * left );
    }

    constexpr friend Vector2<T>&
    operator*=( Vector2<T>& left, T right )
    {
        left.x *= right;
        left.y *= right;

        return left;
    }

    constexpr friend Vector2<T>
    operator/( const Vector2<T>& left, const Vector2<T>& right )
    {
        return Vector2<T>( left.x / right.x, left.y / right.y );
    }

    constexpr friend Vector2<T>
    operator/=( Vector2<T>& left, const Vector2<T>& right )
    {
        left.x /= right.x;
        left.y /= right.y;

        return left;
    }

    constexpr friend Vector2<T>
    operator/( const Vector2<T>& left, T right )
    {
        return Vector2<T>( left.x / right, left.y / right );
    }

    constexpr friend Vector2<T>&
    operator/=( Vector2<T>& left, T right )
    {
        left.x /= right;
        left.y /= right;

        return left;
    }

    constexpr friend bool
    operator==( const Vector2<T>& left, const Vector2<T>& right )
    {
        return ( left.x == right.x ) && ( left.y == right.y );
    }

    constexpr friend bool
    operator!=( const Vector2<T>& left, const Vector2<T>& right )
    {
        return ( left.x != right.x ) || ( left.y != right.y );
    }

    constexpr bool
    valid() const
    {
        return !( std::isnan( x ) || std::isnan( y ) );
    }

    constexpr friend T
    scalarMul( const Vector2<T> left, const Vector2<T> right )
    {
        return left.x * right.x + left.y * right.y;
    }

    constexpr friend T
    calcCos( const Vector2<T>& left, const Vector2<T>& right )
    {
        return scalarMul( left, right ) / ( left.getLen() * right.getLen() );
    }

    float
    calcSin( const Vector2<T>& left, const Vector2<T>& right )
    {
        return std::sin( std::acos( calcCos( left, right ) ) );
    }

    float
    vectorMulModule( const Vector2<T>& left, const Vector2<T>& right )
    {
        return left.getLen() * right.getLen() * calcSin( left, right );
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

    constexpr Vector2<T>
    norm()
    {
        float len = getLen();
        x /= len;
        y /= len;

        return *this;
    }
};

using Vector2f = Vector2<float>;
using Vector2i = Vector2<int>;
using Vector2u = Vector2<unsigned int>;

} // namespace model
} // namespace zemax
