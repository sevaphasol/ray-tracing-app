#include "rta/view/object_editor/types.hpp"

namespace rta {
namespace view {

namespace {
struct TypeName
{
    ObjectType  type;
    const char* name;
};

const TypeName&
tableEntry( ObjectType t )
{
    static const TypeName Table[] = {
        { ObjectType::Sphere, "Sphere" },
        { ObjectType::AABB, "AABB" },
        { ObjectType::Torus, "Torus" },
        { ObjectType::HexPrism, "HexPrism" },
        { ObjectType::Goursat, "Goursat" },
        { ObjectType::RoundedBox, "RoundedBox" },
        { ObjectType::Ellipsoid, "Ellipsoid" },
        { ObjectType::Capsule, "Capsule" },
        { ObjectType::RoundedCone, "RoundedCone" },
        { ObjectType::CappedCone, "CappedCone" },
        { ObjectType::CappedCylinder, "CappedCylinder" },
        { ObjectType::Wedge, "Wedge" },
        { ObjectType::Ellipse, "Ellipse" },
        { ObjectType::Triangle, "Triangle" },
    };
    for ( const auto& e : Table )
    {
        if ( e.type == t )
        {
            return e;
        }
    }
    return Table[0];
}

} // namespace

const char*
objectTypeName( ObjectType t )
{
    return tableEntry( t ).name;
}

ObjectType
objectTypeFromName( const std::string& name )
{
    for ( ObjectType t : allObjectTypes() )
    {
        if ( name == objectTypeName( t ) )
        {
            return t;
        }
    }
    return ObjectType::Sphere;
}

ObjectType
objectTypeFromCode( model::PrimitiveCode code )
{
    switch ( code )
    {
        case model::PrimitiveCode::Sphere:
            return ObjectType::Sphere;
        case model::PrimitiveCode::AABB:
            return ObjectType::AABB;
        case model::PrimitiveCode::Torus:
            return ObjectType::Torus;
        case model::PrimitiveCode::HexPrism:
            return ObjectType::HexPrism;
        case model::PrimitiveCode::Goursat:
            return ObjectType::Goursat;
        case model::PrimitiveCode::RoundedBox:
            return ObjectType::RoundedBox;
        case model::PrimitiveCode::Ellipsoid:
            return ObjectType::Ellipsoid;
        case model::PrimitiveCode::Capsule:
            return ObjectType::Capsule;
        case model::PrimitiveCode::RoundedCone:
            return ObjectType::RoundedCone;
        case model::PrimitiveCode::CappedCone:
            return ObjectType::CappedCone;
        case model::PrimitiveCode::CappedCylinder:
            return ObjectType::CappedCylinder;
        case model::PrimitiveCode::Wedge:
            return ObjectType::Wedge;
        case model::PrimitiveCode::Ellipse:
            return ObjectType::Ellipse;
        case model::PrimitiveCode::Triangle:
            return ObjectType::Triangle;
        case model::PrimitiveCode::Plane:
        default:
            return ObjectType::Sphere;
    }
}

model::PrimitiveCode
primitiveCodeFromObjectType( ObjectType t )
{
    switch ( t )
    {
        case ObjectType::Sphere:
            return model::PrimitiveCode::Sphere;
        case ObjectType::AABB:
            return model::PrimitiveCode::AABB;
        case ObjectType::Torus:
            return model::PrimitiveCode::Torus;
        case ObjectType::HexPrism:
            return model::PrimitiveCode::HexPrism;
        case ObjectType::Goursat:
            return model::PrimitiveCode::Goursat;
        case ObjectType::RoundedBox:
            return model::PrimitiveCode::RoundedBox;
        case ObjectType::Ellipsoid:
            return model::PrimitiveCode::Ellipsoid;
        case ObjectType::Capsule:
            return model::PrimitiveCode::Capsule;
        case ObjectType::RoundedCone:
            return model::PrimitiveCode::RoundedCone;
        case ObjectType::CappedCone:
            return model::PrimitiveCode::CappedCone;
        case ObjectType::CappedCylinder:
            return model::PrimitiveCode::CappedCylinder;
        case ObjectType::Wedge:
            return model::PrimitiveCode::Wedge;
        case ObjectType::Ellipse:
            return model::PrimitiveCode::Ellipse;
        case ObjectType::Triangle:
            return model::PrimitiveCode::Triangle;
    }

    return model::PrimitiveCode::Sphere;
}

const std::vector<ObjectType>&
allObjectTypes()
{
    static const std::vector<ObjectType> All = { ObjectType::Sphere,
                                                 ObjectType::AABB,
                                                 ObjectType::Torus,
                                                 ObjectType::HexPrism,
                                                 ObjectType::Goursat,
                                                 ObjectType::RoundedBox,
                                                 ObjectType::Ellipsoid,
                                                 ObjectType::Capsule,
                                                 ObjectType::RoundedCone,
                                                 ObjectType::CappedCone,
                                                 ObjectType::CappedCylinder,
                                                 ObjectType::Wedge,
                                                 ObjectType::Ellipse,
                                                 ObjectType::Triangle };
    return All;
}

} // namespace view
} // namespace rta
