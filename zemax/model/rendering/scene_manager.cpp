#include "zemax/model/rendering/scene_manager.hpp"
#include "zemax/model/primitives/impls/aabb.hpp"
#include "zemax/model/primitives/impls/hex_prism.hpp"
#include "zemax/model/primitives/impls/plane.hpp"
#include "zemax/model/primitives/impls/sphere.hpp"
#include "zemax/model/primitives/impls/torus.hpp"
#include "zemax/model/primitives/material.hpp"
#include "zemax/model/primitives/primitive.hpp"
#include "zemax/model/rendering/camera.hpp"
#include "zemax/model/rendering/color.hpp"
#include "zemax/model/rendering/vector3.hpp"
#include <cassert>
#include <iostream>
#include <memory>

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
    objects_.push_back( std::make_unique<Torus>( material, center, minor_radius, major_radius ) );
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
