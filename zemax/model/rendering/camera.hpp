#pragma once

#include "zemax/model/rendering/ray.hpp"
#include "zemax/model/rendering/vector2.hpp"
#include "zemax/model/rendering/vector3.hpp"
#include <optional>

namespace zemax {
namespace model {

class Camera {
  public:
    Camera( const Vector3f& pos, float screen_width, float screen_height, float fov = 1.0f );

    Vector3f&
    getPos()
    {
        return pos_;
    }

    Ray
    emitRay( uint pixel_x, uint pixel_y ) const;

    std::optional<Vector2f>
    projectToScreen( const Vector3f& world_pos ) const;

    void
    move( const Vector3f& delta );

    void
    rotate( const Vector2f& delta );

    void
    scale( float scale_factor );

  private:
    void
    rotate( float angle, Vector3f& ort );

  private:
    Vector3f pos_;
    Vector3f hor_ort_;
    Vector3f ver_ort_;
    Vector3f fwd_ort_;
    float    screen_width_;
    float    screen_height_;
    float    aspect_ratio_;
    float    fov_;
};

} // namespace model
} // namespace zemax
