#include "rta/model/rendering/scene_manager.hpp"
#include "rta/model/primitives/impls/aabb.hpp"
#include "rta/model/primitives/impls/capped_cone.hpp"
#include "rta/model/primitives/impls/capped_cylinder.hpp"
#include "rta/model/primitives/impls/capsule.hpp"
#include "rta/model/primitives/impls/ellipse.hpp"
#include "rta/model/primitives/impls/ellipsoid.hpp"
#include "rta/model/primitives/impls/goursat.hpp"
#include "rta/model/primitives/impls/hex_prism.hpp"
#include "rta/model/primitives/impls/plane.hpp"
#include "rta/model/primitives/impls/rounded_box.hpp"
#include "rta/model/primitives/impls/rounded_cone.hpp"
#include "rta/model/primitives/impls/sphere.hpp"
#include "rta/model/primitives/impls/torus.hpp"
#include "rta/model/primitives/impls/triangle.hpp"
#include "rta/model/primitives/impls/wedge.hpp"
#include "rta/model/primitives/material.hpp"
#include "rta/model/primitives/primitive.hpp"
#include "rta/model/rendering/camera.hpp"
#include "rta/model/rendering/color.hpp"
#include "rta/model/rendering/vector3.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <sstream>

namespace rta {
namespace model {

SceneManager::SceneManager( const Vector3f& camera_pos, float screen_width, float screen_height )
    : camera_( camera_pos, screen_width, screen_height )
{
}

void
SceneManager::addLight( Vector3f pos,
                        float    embedded_intensity,
                        float    diffuse_intensity,
                        float    glare_intensity )
{
    lights_.push_back( Light( pos, embedded_intensity, diffuse_intensity, glare_intensity ) );
}

void
SceneManager::addSphere( const Material& material, const Vector3f& center, float radius )
{
    objects_.push_back( std::make_unique<Sphere>( material, center, radius ) );
}

void
SceneManager::addPlane( const Material& material,
                        const Vector3f& base_point,
                        const Vector3f& normal )
{
    objects_.push_back( std::make_unique<Plane>( material, base_point, normal ) );
}

void
SceneManager::addAABB( const Material& material, const Vector3f& center, const Vector3f& bounds )
{
    objects_.push_back( std::make_unique<AABB>( material, center, bounds ) );
}

void
SceneManager::addHexPrism( const Material& material,
                           const Vector3f& center,
                           float           radius,
                           float           height )
{
    objects_.push_back( std::make_unique<HexPrism>( material, center, radius, height ) );
}

void
SceneManager::addTorus( const Material& material,
                        const Vector3f& center,
                        float           minor_radius,
                        float           major_radius )
{
    objects_.push_back( std::make_unique<Torus>( material, center, major_radius, minor_radius ) );
}

void
SceneManager::addGoursat( const Material& material, const Vector3f& center, float ka, float kb )
{
    objects_.push_back( std::make_unique<Goursat>( material, center, ka, kb ) );
}

void
SceneManager::addRoundedBox( const Material& material,
                             const Vector3f& center,
                             const Vector3f& half_size,
                             float           radius )
{
    objects_.push_back( std::make_unique<RoundedBox>( material, center, half_size, radius ) );
}

void
SceneManager::addEllipsoid( const Material& material,
                            const Vector3f& center,
                            const Vector3f& radii )
{
    objects_.push_back( std::make_unique<Ellipsoid>( material, center, radii ) );
}

void
SceneManager::addCapsule( const Material& material,
                          const Vector3f& center,
                          float           height,
                          float           radius )
{
    Vector3f pa_local{ 0.0f, -0.5f * height, 0.0f };
    Vector3f pb_local{ 0.0f, 0.5f * height, 0.0f };
    objects_.push_back( std::make_unique<Capsule>( material, center, pa_local, pb_local, radius ) );
}

void
SceneManager::addRoundedCone( const Material& material,
                              const Vector3f& center,
                              float           height,
                              float           ra,
                              float           rb )
{
    Vector3f pa_local{ 0.0f, -0.5f * height, 0.0f };
    Vector3f pb_local{ 0.0f, 0.5f * height, 0.0f };
    objects_.push_back(
        std::make_unique<RoundedCone>( material, center, pa_local, pb_local, ra, rb ) );
}

void
SceneManager::addEllipse( const Material& material,
                          const Vector3f& center,
                          const Vector3f& u,
                          const Vector3f& v )
{
    objects_.push_back( std::make_unique<Ellipse>( material, center, u, v ) );
}

void
SceneManager::addTriangle( const Material& material,
                           const Vector3f& v0,
                           const Vector3f& v1,
                           const Vector3f& v2 )
{
    objects_.push_back( std::make_unique<Triangle>( material, v0, v1, v2 ) );
}

void
SceneManager::addCappedCone( const Material& material,
                             const Vector3f& center,
                             float           height,
                             float           ra,
                             float           rb )
{
    // Ось по Y, локальные концы относительно center
    Vector3f pa_local{ 0.0f, -0.5f * height, 0.0f };
    Vector3f pb_local{ 0.0f, 0.5f * height, 0.0f };

    objects_.push_back(
        std::make_unique<CappedCone>( material, center, pa_local, pb_local, ra, rb ) );
}

void
SceneManager::addCappedCylinder( const Material& material,
                                 const Vector3f& center,
                                 float           height,
                                 float           radius )
{
    // Ось по Y, локальные концы относительно center
    Vector3f a_local{ 0.0f, -0.5f * height, 0.0f };
    Vector3f b_local{ 0.0f, 0.5f * height, 0.0f };

    objects_.push_back(
        std::make_unique<CappedCylinder>( material, center, a_local, b_local, radius ) );
}

void
SceneManager::addWedge( const Material& material,
                        const Vector3f& center,
                        const Vector3f& half_size )
{
    objects_.push_back( std::make_unique<Wedge>( material, center, half_size ) );
}

bool
SceneManager::saveToFile( const std::string& path ) const
{
    nlohmann::json j;

    // Lights
    j["lights"] = nlohmann::json::array();
    for ( const auto& l : lights_ )
    {
        auto p = l.getPos();
        j["lights"].push_back( { { "position", { { "x", p.x }, { "y", p.y }, { "z", p.z } } },
                                 { "embedded", l.getEmbeddedIntensity() },
                                 { "diffuse", l.getDiffuseIntensity() },
                                 { "glare", l.getGlareIntensity() } } );
    }

    // Objects
    j["objects"] = nlohmann::json::array();
    for ( size_t idx = 0; idx < objects_.size(); ++idx )
    {
        const auto& obj_ptr = objects_[idx];
        auto*       obj     = obj_ptr.get();
        auto        info    = getObjectInfo( idx );
        auto        mat     = obj->getMaterial();

        nlohmann::json o;
        o["type"]     = info.type_name;
        o["name"]     = info.display_name;
        o["material"] = nlohmann::json{ { "color",
                                          { { "r", mat.color.r },
                                            { "g", mat.color.g },
                                            { "b", mat.color.b },
                                            { "a", mat.color.a } } },
                                        { "reflection", mat.reflection_factor },
                                        { "refraction", mat.refraction_factor },
                                        { "eta", mat.refraction_eta } };
        o["position"] = { { "x", info.pos.x }, { "y", info.pos.y }, { "z", info.pos.z } };

        auto push_size = [&]( const std::string& key, float v ) { o["size"][key] = v; };

        if ( auto* s = dynamic_cast<Sphere*>( obj ) )
        {
            push_size( "radius", s->getRadius() );
        } else if ( auto* a = dynamic_cast<AABB*>( obj ) )
        {
            auto hs = a->getHalfSize();
            push_size( "half_x", hs.x );
            push_size( "half_y", hs.y );
            push_size( "half_z", hs.z );
        } else if ( auto* t = dynamic_cast<Torus*>( obj ) )
        {
            push_size( "major_radius", t->getMajorRadius() );
            push_size( "minor_radius", t->getMinorRadius() );
        } else if ( auto* h = dynamic_cast<HexPrism*>( obj ) )
        {
            push_size( "radius", h->getRadius() );
            push_size( "height", h->getHeight() );
        } else if ( auto* g = dynamic_cast<Goursat*>( obj ) )
        {
            push_size( "ka", g->getKa() );
            push_size( "kb", g->getKb() );
        } else if ( auto* rb = dynamic_cast<RoundedBox*>( obj ) )
        {
            auto hs = rb->getHalfSize();
            push_size( "half_x", hs.x );
            push_size( "half_y", hs.y );
            push_size( "half_z", hs.z );
            push_size( "radius", rb->getRadius() );
        } else if ( auto* e = dynamic_cast<Ellipsoid*>( obj ) )
        {
            auto r = e->getRadii();
            push_size( "rad_x", r.x );
            push_size( "rad_y", r.y );
            push_size( "rad_z", r.z );
        } else if ( auto* c = dynamic_cast<Capsule*>( obj ) )
        {
            auto  pa = c->getPaLocal();
            auto  pb = c->getPbLocal();
            float h  = std::abs( ( pb - pa ).y );
            push_size( "height", h );
            push_size( "radius", c->getRadius() );
        } else if ( auto* rc = dynamic_cast<RoundedCone*>( obj ) )
        {
            auto  pa = rc->getPaLocal();
            auto  pb = rc->getPbLocal();
            float h  = std::abs( ( pb - pa ).y );
            push_size( "height", h );
            push_size( "radius_a", rc->getRadiusA() );
            push_size( "radius_b", rc->getRadiusB() );
        } else if ( auto* cc = dynamic_cast<CappedCone*>( obj ) )
        {
            auto  pa = cc->getPaLocal();
            auto  pb = cc->getPbLocal();
            float h  = std::abs( ( pb - pa ).y );
            push_size( "height", h );
            push_size( "radius_a", cc->getRadiusA() );
            push_size( "radius_b", cc->getRadiusB() );
        } else if ( auto* cyl = dynamic_cast<CappedCylinder*>( obj ) )
        {
            auto  a = cyl->getALocal();
            auto  b = cyl->getBLocal();
            float h = std::abs( ( b - a ).y );
            push_size( "height", h );
            push_size( "radius", cyl->getRadius() );
        } else if ( auto* w = dynamic_cast<Wedge*>( obj ) )
        {
            auto hs = w->getS();
            push_size( "half_x", hs.x );
            push_size( "half_y", hs.y );
            push_size( "half_z", hs.z );
        } else if ( auto* el = dynamic_cast<Ellipse*>( obj ) )
        {
            auto u = el->getU();
            auto v = el->getV();
            o["u"] = { { "x", u.x }, { "y", u.y }, { "z", u.z } };
            o["v"] = { { "x", v.x }, { "y", v.y }, { "z", v.z } };
        } else if ( auto* tri = dynamic_cast<Triangle*>( obj ) )
        {
            auto v0 = tri->getV0();
            auto v1 = tri->getV1();
            auto v2 = tri->getV2();
            o["v0"] = { { "x", v0.x }, { "y", v0.y }, { "z", v0.z } };
            o["v1"] = { { "x", v1.x }, { "y", v1.y }, { "z", v1.z } };
            o["v2"] = { { "x", v2.x }, { "y", v2.y }, { "z", v2.z } };
        }

        j["objects"].push_back( o );
    }

    std::ofstream out( path );
    if ( !out.is_open() )
    {
        return false;
    }
    out << j.dump( 4 );
    return true;
}

bool
SceneManager::loadFromFile( const std::string& path )
{
    std::ifstream in( path );
    if ( !in.is_open() )
    {
        return false;
    }

    nlohmann::json j;
    try
    {
        in >> j;
    } catch ( const std::exception& )
    {
        return false;
    }

    clear();

    auto get_obj_array = [&]() -> std::vector<nlohmann::json> {
        if ( j.contains( "objects" ) && j["objects"].is_array() )
            return j["objects"].get<std::vector<nlohmann::json>>();
        // support map with numeric keys
        std::vector<nlohmann::json> res;
        if ( j.is_object() )
        {
            for ( auto& [k, v] : j.items() )
            {
                if ( v.is_object() )
                    res.push_back( v );
            }
        }
        return res;
    };

    auto lights_arr = j.value( "lights", nlohmann::json::array() );
    if ( lights_arr.is_array() )
    {
        for ( auto& l : lights_arr )
        {
            auto pos = l.value( "position", nlohmann::json::object() );
            addLight( { pos.value( "x", 0.0f ), pos.value( "y", 0.0f ), pos.value( "z", 0.0f ) },
                      l.value( "embedded", 1.0f ),
                      l.value( "diffuse", 1.0f ),
                      l.value( "glare", 1.0f ) );
        }
    }

    auto objs = get_obj_array();
    for ( auto& o : objs )
    {
        auto     type = o.value( "type", std::string() );
        auto     name = o.value( "name", type );
        auto     pos  = o.value( "position", nlohmann::json::object() );
        auto     matj = o.value( "material", nlohmann::json::object() );
        auto     colj = matj.value( "color", nlohmann::json::object() );
        Color    color( static_cast<uint8_t>( colj.value( "r", 118 ) ),
                     static_cast<uint8_t>( colj.value( "g", 185 ) ),
                     static_cast<uint8_t>( colj.value( "b", 0 ) ),
                     static_cast<uint8_t>( colj.value( "a", 255 ) ) );
        float    refl = matj.value( "reflection", 0.0f );
        float    refr = matj.value( "refraction", matj.value( "refracion", 0.0f ) ); // typo support
        float    eta  = matj.value( "eta", 1.0f );
        Material material( color, refl, refr, eta );
        Vector3f origin{ pos.value( "x", 0.0f ), pos.value( "y", 0.0f ), pos.value( "z", 0.0f ) };
        auto     size = o.value( "size", nlohmann::json::object() );

        auto add_with_name = [&]( auto obj_builder ) {
            obj_builder();
            if ( !objects_.empty() )
                objects_.back()->setDisplayName( name );
        };

        if ( type == "Sphere" )
        {
            add_with_name( [&]() { addSphere( material, origin, size.value( "radius", 1.0f ) ); } );
        } else if ( type == "AABB" )
        {
            add_with_name( [&]() {
                addAABB( material,
                         origin,
                         { size.value( "half_x", 1.0f ),
                           size.value( "half_y", 1.0f ),
                           size.value( "half_z", 1.0f ) } );
            } );
        } else if ( type == "Torus" )
        {
            add_with_name( [&]() {
                addTorus( material,
                          origin,
                          size.value( "minor_radius", 0.5f ),
                          size.value( "major_radius", 1.0f ) );
            } );
        } else if ( type == "HexPrism" )
        {
            add_with_name( [&]() {
                addHexPrism( material,
                             origin,
                             size.value( "radius", 1.0f ),
                             size.value( "height", 1.0f ) );
            } );
        } else if ( type == "Goursat" )
        {
            add_with_name( [&]() {
                addGoursat( material, origin, size.value( "ka", 1.0f ), size.value( "kb", 1.0f ) );
            } );
        } else if ( type == "RoundedBox" )
        {
            add_with_name( [&]() {
                addRoundedBox( material,
                               origin,
                               { size.value( "half_x", 1.0f ),
                                 size.value( "half_y", 1.0f ),
                                 size.value( "half_z", 1.0f ) },
                               size.value( "radius", 0.2f ) );
            } );
        } else if ( type == "Ellipsoid" )
        {
            add_with_name( [&]() {
                addEllipsoid( material,
                              origin,
                              { size.value( "rad_x", 1.0f ),
                                size.value( "rad_y", 1.0f ),
                                size.value( "rad_z", 1.0f ) } );
            } );
        } else if ( type == "Capsule" )
        {
            add_with_name( [&]() {
                addCapsule( material,
                            origin,
                            size.value( "height", 1.0f ),
                            size.value( "radius", 0.5f ) );
            } );
        } else if ( type == "RoundedCone" )
        {
            add_with_name( [&]() {
                addRoundedCone( material,
                                origin,
                                size.value( "height", 1.0f ),
                                size.value( "radius_a", 1.0f ),
                                size.value( "radius_b", 0.5f ) );
            } );
        } else if ( type == "CappedCone" )
        {
            add_with_name( [&]() {
                addCappedCone( material,
                               origin,
                               size.value( "height", 1.0f ),
                               size.value( "radius_a", 1.0f ),
                               size.value( "radius_b", 0.5f ) );
            } );
        } else if ( type == "CappedCylinder" )
        {
            add_with_name( [&]() {
                addCappedCylinder( material,
                                   origin,
                                   size.value( "height", 1.0f ),
                                   size.value( "radius", 0.5f ) );
            } );
        } else if ( type == "Wedge" )
        {
            add_with_name( [&]() {
                addWedge( material,
                          origin,
                          { size.value( "half_x", 1.0f ),
                            size.value( "half_y", 1.0f ),
                            size.value( "half_z", 1.0f ) } );
            } );
        } else if ( type == "Ellipse" )
        {
            auto u = o.value( "u", nlohmann::json::object() );
            auto v = o.value( "v", nlohmann::json::object() );
            add_with_name( [&]() {
                addEllipse( material,
                            origin,
                            { u.value( "x", 1.0f ), u.value( "y", 0.0f ), u.value( "z", 0.0f ) },
                            { v.value( "x", 0.0f ), v.value( "y", 1.0f ), v.value( "z", 0.0f ) } );
            } );
        } else if ( type == "Triangle" )
        {
            auto v0 = o.value( "v0", nlohmann::json::object() );
            auto v1 = o.value( "v1", nlohmann::json::object() );
            auto v2 = o.value( "v2", nlohmann::json::object() );
            add_with_name( [&]() {
                addTriangle(
                    material,
                    { v0.value( "x", 0.0f ), v0.value( "y", 0.0f ), v0.value( "z", 0.0f ) },
                    { v1.value( "x", 1.0f ), v1.value( "y", 0.0f ), v1.value( "z", 0.0f ) },
                    { v2.value( "x", 0.0f ), v2.value( "y", 1.0f ), v2.value( "z", 0.0f ) } );
            } );
        }
    }

    need_update_ = true;
    target_obj_  = nullptr;
    return true;
}

void
SceneManager::moveLights( const Vector3f& delta )
{
    for ( auto& light : lights_ )
    {
        light.move( delta );
    }
}

bool
SceneManager::findClosestIntersection( IntersectionContext& ctx )
{
    bool  hit          = false;
    float min_distance = std::numeric_limits<float>::max();

    for ( auto& object : objects_ )
    {
        auto info = object->calcRayIntersection( ctx.view_ray );

        if ( !info.has_value() )
        {
            continue;
        }

        auto ro = ctx.view_ray.getBasePoint();
        auto rd = ctx.view_ray.getDir();

        if ( info->inside_object )
        {
            continue;
        }

        const Vector3f point = ro + info->close_distance * rd;

        float distance = ( point - ctx.view_ray.getBasePoint() ).getLen();
        if ( distance > 0 && distance < min_distance )
        {
            min_distance           = distance;
            ctx.closest_object     = object.get();
            ctx.intersection_point = point;
            ctx.normal =
                ( info->normal.has_value() ? info->normal.value()
                                           : object->calcNormal( point, info->inside_object ) );
            hit = true;
        }
    }

    return hit;
}

Color
SceneManager::calcLightsColor( IntersectionContext& ctx )
{
    Color sum_light;

    for ( const auto& light : lights_ )
    {
        sum_light += light.calcColor( ctx.view_ray.getDir(),
                                      ctx.intersection_point,
                                      ctx.normal,
                                      ctx.closest_object->getMaterial() );
    }

    sum_light.clamp( 0, 255 );

    return sum_light;
}

Color
SceneManager::calcRefractedColor( IntersectionContext& ctx )
{
    Vector3f i = ctx.view_ray.getDir().normalize();
    Vector3f n = ctx.normal.normalize();

    float cosi = scalarMul( i, n );

    float etai = 1.0f;
    float etat = ctx.closest_object->getMaterial().refraction_eta;

    if ( cosi > 0.0f )
    {
        std::swap( etai, etat );
        n = -n;
    }

    float eta = etai / etat;
    float k   = 1.0f - eta * eta * ( 1.0f - cosi * cosi );

    if ( k < 0.0f )
    {
        return calcReflectedColor( ctx );
    }

    Vector3f refract_dir = ( eta * i ) + ( eta * cosi - std::sqrt( k ) ) * n;
    refract_dir          = refract_dir.normalize();

    Ray refracted_ray( refract_dir, ctx.intersection_point + refract_dir * 1e-4f );

    auto old_ctx = ctx;

    ctx.view_ray = refracted_ray;
    ctx.depth++;
    Color color = calcRayColor( ctx );

    ctx = old_ctx;

    return color;
}

Color
SceneManager::calcReflectedColor( IntersectionContext& ctx )
{
    Vector3f reflect_dir = ctx.view_ray.getDir().calcReflected( ctx.normal );

    Ray reflected_ray( reflect_dir, ctx.intersection_point + 1e-4f * ctx.normal );

    auto old_ctx = ctx;

    ctx.view_ray = reflected_ray;
    ctx.depth++;
    Color color = calcRayColor( ctx );

    ctx = old_ctx;

    return color;
}

Color
SceneManager::calcColor( IntersectionContext& ctx )
{
    Color light_color     = calcLightsColor( ctx );
    Color reflected_color = { 0, 0, 0 };
    Color refracted_color = { 0, 0, 0 };

    if ( ctx.closest_object->getMaterial().refraction_factor > 0.0f )
    {
        refracted_color = calcRefractedColor( ctx );
    } else if ( ctx.closest_object->getMaterial().reflection_factor > 0.0f )
    {
        reflected_color = calcReflectedColor( ctx );
    }

    float reflection_factor = ctx.closest_object->getMaterial().reflection_factor;
    float refraction_factor = ctx.closest_object->getMaterial().refraction_factor;

    return ( 1 - reflection_factor - refraction_factor ) * light_color +
           reflection_factor * reflected_color + refraction_factor * refracted_color;
}

Color
SceneManager::calcRayColor( IntersectionContext& ctx )
{
    if ( ctx.depth >= IntersectionContext::MaxDepth )
    {
        return ctx.background_color;
    }

    if ( findClosestIntersection( ctx ) )
    {
        return calcColor( ctx );
    }

    return ctx.background_color;
}

Color
SceneManager::calcPixelColor( uint row, uint col, const Color& background_color )
{
    const Ray view_ray = camera_.emitRay( col, row );

    IntersectionContext ctx( view_ray, background_color );

    Color ray_color = calcRayColor( ctx );

    return calcRayColor( ctx );
}
} // namespace model
} // namespace rta
