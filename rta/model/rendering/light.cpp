#include "rta/model/rendering/light.hpp"
#include "rta/model/primitives/material.hpp"
#include "rta/model/rendering/color.hpp"
#include "rta/model/rendering/vector3.hpp"

#include <iostream>

namespace rta {
namespace model {

Light::Light( const Vector3f& pos,
              float           embedded_intensity,
              float           diffuse_intensity,
              float           glare_intensity )
    : pos_( pos ),
      embedded_intensity_( embedded_intensity ),
      diffuse_intensity_( diffuse_intensity ),
      glare_intensity_( glare_intensity ) {};

void
Light::move( const Vector3f& delta )
{
    pos_ += delta;
}

Color
Light::calcColor( const Vector3f& ray,
                  const Vector3f& point,
                  const Vector3f& normal,
                  const Material& mat ) const
{
    Vector3f light_ray = pos_ - point;

    Color embedded_light = calcEmbeddedLight( mat );
    Color diffuse_light  = calcDiffuseLight( light_ray, normal );
    Color glare_light    = calcGlareLight( light_ray, normal, ray );

    return embedded_light + diffuse_light + glare_light;
}

Color
Light::calcEmbeddedLight( const Material& mat ) const
{
    return ( mat.painted ) ? Color( 32, 32, 32 ) : mat.color * embedded_intensity_;
}

float
Light::calcDiffuseLight( const Vector3f& light_ray, const Vector3f& normal ) const
{
    float intensity = diffuse_intensity_ * calcCos( light_ray, normal );

    return std::max( 0.0f, intensity * 255 );
}

float
Light::calcGlareLight( const Vector3f& light_ray,
                       const Vector3f& normal,
                       const Vector3f& view_ray ) const
{
    Vector3f reflected_ray = light_ray.calcReflected( normal );

    float cos = calcCos( view_ray, reflected_ray );

    float intensity = glare_intensity_ * float( std::pow( cos, 11 ) );

    return std::max( 0.0f, intensity * 255 );
}

} // namespace model
} // namespace rta
