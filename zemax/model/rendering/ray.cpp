#include "zemax/model/rendering/ray.hpp"
#include "zemax/model/rendering/vector3.hpp"

namespace zemax {
namespace model {

Ray::Ray( const Vector3f& dir, const Vector3f& base_point ) : dir_( dir ), base_point_( base_point )
{
    dir_.normalize();
}

float
Ray::calcDistance( const Vector3f& point ) const
{
    return vectorMulModule( ( point - base_point_ ), dir_ );
}

} // namespace model
} // namespace zemax
