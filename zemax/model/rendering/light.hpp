#pragma once

#include "zemax/model/primitives/material.hpp"
#include "zemax/model/rendering/color.hpp"
#include "zemax/model/rendering/vector3.hpp"

namespace zemax {
namespace model {

class Light {
  public:
    Light( const Vector3f& pos,
           float           embedded_intensity,
           float           diffuse_intensity,
           float           glare_intensity );

    void
    move( const Vector3f& delta );

    Color
    calcColor( const Vector3f& ray,
               const Vector3f& point,
               const Vector3f& normal,
               const Material& color ) const;

    Vector3f
    getPos() const
    {
        return pos_;
    }
    float
    getEmbeddedIntensity() const
    {
        return embedded_intensity_;
    }
    float
    getDiffuseIntensity() const
    {
        return diffuse_intensity_;
    }
    float
    getGlareIntensity() const
    {
        return glare_intensity_;
    }

  private:
    Color
    calcEmbeddedLight( const Material& color ) const;

    float
    calcDiffuseLight( const Vector3f& light_ray, const Vector3f& normal ) const;

    float
    calcGlareLight( const Vector3f& light_ray, const Vector3f& normal, const Vector3f& view ) const;

  private:
    Vector3f pos_;
    float    embedded_intensity_;
    float    diffuse_intensity_;
    float    glare_intensity_;
};

} // namespace model
} // namespace zemax
