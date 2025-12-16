#pragma once

#include "rta/model/primitives/primitive.hpp"
#include <string>
#include <vector>

namespace rta {
namespace view {

enum class ObjectType {
    Sphere,
    AABB,
    Torus,
    HexPrism,
    Goursat,
    RoundedBox,
    Ellipsoid,
    Capsule,
    RoundedCone,
    CappedCone,
    CappedCylinder,
    Wedge,
    Ellipse,
    Triangle
};

const char*
objectTypeName( ObjectType t );
ObjectType
objectTypeFromName( const std::string& name );
ObjectType
objectTypeFromCode( model::PrimitiveCode code );
model::PrimitiveCode
primitiveCodeFromObjectType( ObjectType t );
const std::vector<ObjectType>&
allObjectTypes();

} // namespace view
} // namespace rta
