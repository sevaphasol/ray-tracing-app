#pragma once

#include <algorithm>
#include <sys/types.h>

namespace rta {
namespace model {

class Color {
  public:
    unsigned char r, g, b, a;

    constexpr Color() : r( 0 ), g( 0 ), b( 0 ), a( 255 ) {}

    constexpr Color( unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255 )
        : r( r ), g( g ), b( b ), a( a )
    {
    }

    constexpr Color( unsigned char factor ) : r( factor ), g( factor ), b( factor ), a( 255 ) {}

    Color&
    operator=( const Color& that )
    {
        this->r = that.r;
        this->g = that.g;
        this->b = that.b;
        this->a = that.a;

        return *this;
    }

    static const Color White;
    static const Color Black;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Transparent;

    constexpr friend Color
    operator+( const Color& left, const Color& right )
    {
        return Color( std::clamp( left.r + right.r, 0, 255 ),
                      std::clamp( left.g + right.g, 0, 255 ),
                      std::clamp( left.b + right.b, 0, 255 ) );
    }

    constexpr friend Color
    operator*( Color color, unsigned char factor )
    {
        return Color( std::clamp( color.r * factor, 0, 255 ),
                      std::clamp( color.g * factor, 0, 255 ),
                      std::clamp( color.b * factor, 0, 255 ) );
    }

    template<typename T>
    constexpr friend Color
    operator*( T factor, Color color )
    {
        return Color( T( color.r ) * factor, T( color.g ) * factor, T( color.b ) * factor );
    }

    constexpr friend Color&
    operator+=( Color& left, const Color& right )
    {
        left.r = std::clamp( left.r + right.r, 0, 255 );
        left.g = std::clamp( left.g + right.g, 0, 255 );
        left.b = std::clamp( left.b + right.b, 0, 255 );

        return left;
    }

    void
    clamp( unsigned char lo, unsigned char hi )
    {
        r = std::clamp<unsigned char>( r, 0, 255 );
        g = std::clamp<unsigned char>( g, 0, 255 );
        b = std::clamp<unsigned char>( b, 0, 255 );
    }
};

inline constexpr Color Color::White{ 255, 255, 255 };
inline constexpr Color Color::Black{ 0, 0, 0 };
inline constexpr Color Color::Red{ 255, 0, 0 };
inline constexpr Color Color::Green{ 0, 255, 0 };
inline constexpr Color Color::Blue{ 0, 0, 255 };
inline constexpr Color Color::Transparent{ 0, 0, 0, 0 };

} // namespace model
} // namespace rta
