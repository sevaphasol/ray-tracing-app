#pragma once

#include "rta/model/rendering/color.hpp"

namespace rta {
namespace model {

struct Material
{
    Material( const Color& color_ ) : color( color_ ) {}
    Material( const Color& color_, float reflection_factor_ )
        : color( color_ ), reflection_factor( reflection_factor_ )
    {
    }
    Material( const Color& color_,
              float        reflection_factor_,
              float        refraction_factor_,
              float        refraction_eta_ )
        : color( color_ ),
          reflection_factor( reflection_factor_ ),
          refraction_factor( refraction_factor_ ),
          refraction_eta( refraction_eta_ )
    {
    }

    Color color             = { 0, 0, 0, 0 };
    float refraction_factor = 0.0F;
    float refraction_eta    = 0.0F;
    float reflection_factor = 0.0F;
    bool  painted           = false;
};

} // namespace model
} // namespace rta
