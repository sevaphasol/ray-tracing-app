#include "zemax/model/rendering/scene_manager.hpp"
#include "zemax/model/primitives/impls/aabb.hpp"
#include "zemax/model/primitives/impls/capped_cone.hpp"
#include "zemax/model/primitives/impls/capped_cylinder.hpp"
#include "zemax/model/primitives/impls/capsule.hpp"
#include "zemax/model/primitives/impls/ellipse.hpp"
#include "zemax/model/primitives/impls/ellipsoid.hpp"
#include "zemax/model/primitives/impls/goursat.hpp"
#include "zemax/model/primitives/impls/hex_prism.hpp"
#include "zemax/model/primitives/impls/plane.hpp"
#include "zemax/model/primitives/impls/rounded_box.hpp"
#include "zemax/model/primitives/impls/rounded_cone.hpp"
#include "zemax/model/primitives/impls/sphere.hpp"
#include "zemax/model/primitives/impls/torus.hpp"
#include "zemax/model/primitives/impls/triangle.hpp"
#include "zemax/model/primitives/impls/wedge.hpp"
#include "zemax/model/primitives/material.hpp"
#include "zemax/model/primitives/primitive.hpp"
#include "zemax/model/rendering/camera.hpp"
#include "zemax/model/rendering/color.hpp"
#include "zemax/model/rendering/vector3.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

namespace zemax {
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
    std::ofstream out( path );
    if ( !out.is_open() )
    {
        return false;
    }

    // Simple text format:
    // LIGHTS N
    // x y z emb diff glare
    // ...
    // OBJECTS N
    // TYPE name x y z p1 p2 p3 p4 r g b refl
    out << "LIGHTS " << lights_.size() << '\n';
    for ( const auto& l : lights_ )
    {
        auto p = l.getPos();
        out << p.x << ' ' << p.y << ' ' << p.z << ' ' << l.getEmbeddedIntensity() << ' '
            << l.getDiffuseIntensity() << ' ' << l.getGlareIntensity() << '\n';
    }

    out << "OBJECTS " << objects_.size() << '\n';
    for ( const auto& obj_ptr : objects_ )
    {
        auto* obj = obj_ptr.get();
        auto  info =
            getObjectInfo( static_cast<size_t>( &obj_ptr - &objects_[0] ) ); // order preserved

        auto material = obj->getMaterial();
        out << info.type_name << ' ' << info.display_name << ' ' << info.pos.x << ' ' << info.pos.y
            << ' ' << info.pos.z << ' ';

        if ( auto* s = dynamic_cast<Sphere*>( obj ) )
        {
            out << s->getRadius() << " 0 0 0 ";
        } else if ( auto* a = dynamic_cast<AABB*>( obj ) )
        {
            auto hs = a->getHalfSize();
            out << hs.x << ' ' << hs.y << ' ' << hs.z << " 0 ";
        } else if ( auto* t = dynamic_cast<Torus*>( obj ) )
        {
            out << t->getMajorRadius() << ' ' << t->getMinorRadius() << " 0 0 ";
        } else if ( auto* h = dynamic_cast<HexPrism*>( obj ) )
        {
            out << h->getRadius() << ' ' << h->getHeight() << " 0 0 ";
        } else
        {
            out << "0 0 0 0 ";
        }

        out << static_cast<int>( material.color.r ) << ' ' << static_cast<int>( material.color.g )
            << ' ' << static_cast<int>( material.color.b ) << ' ' << material.reflection_factor
            << '\n';
    }

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

    clear();

    std::string header;
    size_t      count = 0;

    // Lights
    if ( !( in >> header >> count ) || header != "LIGHTS" )
    {
        return false;
    }
    for ( size_t i = 0; i < count; ++i )
    {
        float x, y, z, emb, diff, glare;
        if ( !( in >> x >> y >> z >> emb >> diff >> glare ) )
        {
            return false;
        }
        addLight( { x, y, z }, emb, diff, glare );
    }

    if ( !( in >> header >> count ) || header != "OBJECTS" )
    {
        return false;
    }

    std::string line;
    std::getline( in, line ); // consume endline
    for ( size_t i = 0; i < count; ++i )
    {
        if ( !std::getline( in, line ) )
            break;
        std::istringstream iss( line );
        std::string        type, name;
        float              x, y, z, p1, p2, p3, p4;
        int                r, g, b;
        float              refl;

        if ( !( iss >> type >> name >> x >> y >> z >> p1 >> p2 >> p3 >> p4 >> r >> g >> b >>
                refl ) )
        {
            continue;
        }

        Material mat( Color( static_cast<uint8_t>( r ),
                             static_cast<uint8_t>( g ),
                             static_cast<uint8_t>( b ) ),
                      refl );
        Vector3f origin{ x, y, z };

        if ( type == "Sphere" )
        {
            addSphere( mat, origin, p1 );
        } else if ( type == "AABB" )
        {
            addAABB( mat, origin, { p1, p2, p3 } );
        } else if ( type == "Torus" )
        {
            addTorus( mat, origin, p2, p1 );
        } else if ( type == "HexPrism" )
        {
            addHexPrism( mat, origin, p1, p2 );
        } else
        {
            continue;
        }

        if ( !objects_.empty() )
        {
            objects_.back()->setDisplayName( name );
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
} // namespace zemax
