#pragma once

#include "rta/model/rendering/vector3.hpp"

namespace rta {
namespace model {

class Ray {
  public:
    Ray( const Vector3f& dir, const Vector3f& base_point );

    Ray() = default;

    float
    calcDistance( const Vector3f& point ) const;

    Vector3f
    getDir() const
    {
        return dir_;
    }

    Vector3f
    getBasePoint() const
    {
        return base_point_;
    }

  private:
    Vector3f dir_;
    Vector3f base_point_;
};

} // namespace model
} // namespace rta
