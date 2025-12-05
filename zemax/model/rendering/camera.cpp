#include "zemax/model/rendering/camera.hpp"
#include "zemax/config.hpp"
#include "zemax/model/rendering/vector2.hpp"
#include <optional>

namespace zemax {
namespace model {

Camera::Camera( const Vector3f& pos, float screen_width, float screen_height, float fov )
    : pos_( pos ),
      screen_width_( screen_width ),
      screen_height_( screen_height ),
      aspect_ratio_( screen_height / screen_width ),
      hor_ort_( 1, 0, 0 ),
      ver_ort_( 0, 1, 0 ),
      fwd_ort_( 0, 0, -1 ),
      fov_( fov )
{
}

void
Camera::scale( float scale_factor )
{
    fov_ += fov_ * scale_factor;
}

Ray
Camera::emitRay( uint pixel_x, uint pixel_y ) const
{
    float x3d = ( 2 * ( ( float( pixel_x ) + 0.5 ) / screen_width_ ) - 1 ) * fov_;
    float y3d = ( -2 * ( ( float( pixel_y ) + 0.5 ) / screen_height_ ) + 1 ) * aspect_ratio_ * fov_;

    Vector3f ray_dir = x3d * hor_ort_ + y3d * ver_ort_ + fwd_ort_;

    return Ray( ray_dir, pos_ );
}

std::optional<Vector2f>
Camera::projectToScreen( const Vector3f& world_pos ) const
{
    Vector3f bt_ray = world_pos - pos_;

    float x = scalarMul( bt_ray, hor_ort_ );
    float y = scalarMul( bt_ray, ver_ort_ );
    float z = scalarMul( bt_ray, fwd_ort_ );

    if ( std::abs( z ) < 1e-6f )
    {
        return std::nullopt;
    }

    float x_n = x / z;
    float y_n = y / z;

    float px = ( ( x_n / fov_ + 1.0f ) / 2 ) * screen_width_ - 0.5f;
    float py = ( ( ( y_n / fov_ ) / aspect_ratio_ - 1.0f ) / -2 ) * screen_height_ - 0.5f;

    return Vector2f( px, py );
}

void
Camera::move( const Vector3f& delta )
{
    pos_ += ( delta.x * hor_ort_ + delta.y * ver_ort_ - delta.z * fwd_ort_ );
}

void
Camera::rotate( const Vector2f& delta )
{
    rotate( delta.x, hor_ort_ );
    rotate( delta.y, ver_ort_ );
}

void
Camera::rotate( float angle, Vector3f& ort )
{
    float cos_angle = std::cos( angle );
    float sin_angle = std::sin( angle );

    Vector3f old_fwd_ort = fwd_ort_;
    Vector3f old_ort     = ort;

    fwd_ort_ = old_fwd_ort * cos_angle - old_ort * sin_angle;
    ort      = old_fwd_ort * sin_angle + old_ort * cos_angle;
}

} // namespace model
} // namespace zemax
