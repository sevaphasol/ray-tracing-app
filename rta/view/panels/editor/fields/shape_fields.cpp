#include "rta/view/panels/editor/fields/shape_fields.hpp"
#include "rta/model/primitives/impls/aabb.hpp"
#include "rta/model/primitives/impls/capped_cone.hpp"
#include "rta/model/primitives/impls/capped_cylinder.hpp"
#include "rta/model/primitives/impls/capsule.hpp"
#include "rta/model/primitives/impls/ellipse.hpp"
#include "rta/model/primitives/impls/ellipsoid.hpp"
#include "rta/model/primitives/impls/goursat.hpp"
#include "rta/model/primitives/impls/hex_prism.hpp"
#include "rta/model/primitives/impls/rounded_box.hpp"
#include "rta/model/primitives/impls/rounded_cone.hpp"
#include "rta/model/primitives/impls/sphere.hpp"
#include "rta/model/primitives/impls/torus.hpp"
#include "rta/model/primitives/impls/triangle.hpp"
#include "rta/model/primitives/impls/wedge.hpp"
#include <cmath>

namespace rta {
namespace view {

namespace {
float
layoutCommonXYZ( ObjEditFields& f, float start_y )
{
    return f.layoutFields( { { "X", "x" }, { "Y", "y" }, { "Z", "z" } }, start_y );
}

void
setPosDefaults( ObjEditFields& f )
{
    f.setField( "x", "0.00" );
    f.setField( "y", "0.00" );
    f.setField( "z", "-10.00" );
}

} // namespace

// --- Sphere -----------------------------------------------------------------
SphereFields::SphereFields( hui::WindowManager* wm, hui::DialogBox* parent, const dr4::Vec2f& size )
    : ObjEditFields( wm, parent, size, "Sphere" )
{
    float y = layoutNameField();
    layoutFields( { { "X", "x" }, { "Y", "y" }, { "Z", "z" }, { "Radius", "p1" } }, y );
}

void
SphereFields::prefillDefaults()
{
    ObjEditFields::prefillDefaults();
    setPosDefaults( *this );
    setField( "p1", "1.00" );
}

void
SphereFields::prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj )
{
    ObjEditFields::prefillExisting( info, obj );
    if ( auto* s = dynamic_cast<model::Sphere*>( &obj ) )
    {
        setField( "x", fmt2( info.pos.x ) );
        setField( "y", fmt2( info.pos.y ) );
        setField( "z", fmt2( info.pos.z ) );
        setField( "p1", fmt2( s->getRadius() ) );
    }
}

std::unique_ptr<model::Primitive>
SphereFields::buildNew( const CommonValues& common )
{
    auto pos = parsePos();
    auto r   = parseField( "p1", []( double v ) { return v > 0; } );
    if ( !pos || !r )
    {
        return nullptr;
    }
    auto obj = std::make_unique<model::Sphere>( common.material, *pos, static_cast<float>( *r ) );
    obj->setDisplayName( common.name );
    return obj;
}

bool
SphereFields::applyToExisting( const CommonValues& common, model::Primitive& obj )
{
    auto* s = dynamic_cast<model::Sphere*>( &obj );
    if ( !s )
    {
        return false;
    }
    auto pos = parsePos();
    auto r   = parseField( "p1", []( double v ) { return v > 0; } );
    if ( !pos || !r )
    {
        return false;
    }
    s->setOrigin( *pos );
    s->setMaterial( common.material );
    s->setRadius( static_cast<float>( *r ) );
    s->setDisplayName( common.name );
    return true;
}

// --- AABB -------------------------------------------------------------------
AABBFields::AABBFields( hui::WindowManager* wm, hui::DialogBox* parent, const dr4::Vec2f& size )
    : ObjEditFields( wm, parent, size, "AABB" )
{
    float y = layoutNameField();
    layoutFields( { { "X", "x" },
                    { "Y", "y" },
                    { "Z", "z" },
                    { "Half size X", "p1" },
                    { "Half size Y", "p2" },
                    { "Half size Z", "p3" } },
                  y );
}

void
AABBFields::prefillDefaults()
{
    ObjEditFields::prefillDefaults();
    setPosDefaults( *this );
    setField( "p1", "1.00" );
    setField( "p2", "1.00" );
    setField( "p3", "1.00" );
}

void
AABBFields::prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj )
{
    ObjEditFields::prefillExisting( info, obj );
    if ( auto* a = dynamic_cast<model::AABB*>( &obj ) )
    {
        auto hs = a->getHalfSize();
        setField( "x", fmt2( info.pos.x ) );
        setField( "y", fmt2( info.pos.y ) );
        setField( "z", fmt2( info.pos.z ) );
        setField( "p1", fmt2( hs.x ) );
        setField( "p2", fmt2( hs.y ) );
        setField( "p3", fmt2( hs.z ) );
    }
}

std::unique_ptr<model::Primitive>
AABBFields::buildNew( const CommonValues& common )
{
    auto pos = parsePos();
    auto p1  = parseField( "p1", []( double v ) { return v > 0; } );
    auto p2  = parseField( "p2", []( double v ) { return v > 0; } );
    auto p3  = parseField( "p3", []( double v ) { return v > 0; } );
    if ( !pos || !p1 || !p2 || !p3 )
    {
        return nullptr;
    }
    auto obj = std::make_unique<model::AABB>( common.material,
                                              *pos,
                                              model::Vector3f( static_cast<float>( *p1 ),
                                                               static_cast<float>( *p2 ),
                                                               static_cast<float>( *p3 ) ) );
    obj->setDisplayName( common.name );
    return obj;
}

bool
AABBFields::applyToExisting( const CommonValues& common, model::Primitive& obj )
{
    auto* a = dynamic_cast<model::AABB*>( &obj );
    if ( !a )
    {
        return false;
    }
    auto pos = parsePos();
    auto p1  = parseField( "p1", []( double v ) { return v > 0; } );
    auto p2  = parseField( "p2", []( double v ) { return v > 0; } );
    auto p3  = parseField( "p3", []( double v ) { return v > 0; } );
    if ( !pos || !p1 || !p2 || !p3 )
    {
        return false;
    }
    a->setOrigin( *pos );
    a->setMaterial( common.material );
    a->setHalfSize( model::Vector3f( static_cast<float>( *p1 ),
                                     static_cast<float>( *p2 ),
                                     static_cast<float>( *p3 ) ) );
    a->setDisplayName( common.name );
    return true;
}

// --- Torus ------------------------------------------------------------------
TorusFields::TorusFields( hui::WindowManager* wm, hui::DialogBox* parent, const dr4::Vec2f& size )
    : ObjEditFields( wm, parent, size, "Torus" )
{
    float y = layoutNameField();
    layoutFields( { { "X", "x" },
                    { "Y", "y" },
                    { "Z", "z" },
                    { "Major radius", "p1" },
                    { "Minor radius", "p2" } },
                  y );
}

void
TorusFields::prefillDefaults()
{
    ObjEditFields::prefillDefaults();
    setPosDefaults( *this );
    setField( "p1", "2.00" );
    setField( "p2", "0.80" );
}

void
TorusFields::prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj )
{
    ObjEditFields::prefillExisting( info, obj );
    if ( auto* t = dynamic_cast<model::Torus*>( &obj ) )
    {
        setField( "x", fmt2( info.pos.x ) );
        setField( "y", fmt2( info.pos.y ) );
        setField( "z", fmt2( info.pos.z ) );
        setField( "p1", fmt2( t->getMajorRadius() ) );
        setField( "p2", fmt2( t->getMinorRadius() ) );
    }
}

std::unique_ptr<model::Primitive>
TorusFields::buildNew( const CommonValues& common )
{
    auto pos = parsePos();
    auto p1  = parseField( "p1", []( double v ) { return v > 0; } );
    auto p2  = parseField( "p2", []( double v ) { return v > 0; } );
    if ( !pos || !p1 || !p2 )
    {
        return nullptr;
    }
    auto obj = std::make_unique<model::Torus>( common.material,
                                               *pos,
                                               static_cast<float>( *p1 ),
                                               static_cast<float>( *p2 ) );
    obj->setDisplayName( common.name );
    return obj;
}

bool
TorusFields::applyToExisting( const CommonValues& common, model::Primitive& obj )
{
    auto* t = dynamic_cast<model::Torus*>( &obj );
    if ( !t )
    {
        return false;
    }
    auto pos = parsePos();
    auto p1  = parseField( "p1", []( double v ) { return v > 0; } );
    auto p2  = parseField( "p2", []( double v ) { return v > 0; } );
    if ( !pos || !p1 || !p2 )
    {
        return false;
    }
    t->setOrigin( *pos );
    t->setMaterial( common.material );
    t->setMajorRadius( static_cast<float>( *p1 ) );
    t->setMinorRadius( static_cast<float>( *p2 ) );
    t->setDisplayName( common.name );
    return true;
}

// --- HexPrism ---------------------------------------------------------------
HexPrismFields::HexPrismFields( hui::WindowManager* wm,
                                hui::DialogBox*     parent,
                                const dr4::Vec2f&   size )
    : ObjEditFields( wm, parent, size, "HexPrism" )
{
    float y = layoutNameField();
    layoutFields(
        { { "X", "x" }, { "Y", "y" }, { "Z", "z" }, { "Radius", "p1" }, { "Height", "p2" } },
        y );
}

void
HexPrismFields::prefillDefaults()
{
    ObjEditFields::prefillDefaults();
    setPosDefaults( *this );
    setField( "p1", "1.00" );
    setField( "p2", "2.00" );
}

void
HexPrismFields::prefillExisting( const model::SceneManager::ObjectInfo& info,
                                 model::Primitive&                      obj )
{
    ObjEditFields::prefillExisting( info, obj );
    if ( auto* h = dynamic_cast<model::HexPrism*>( &obj ) )
    {
        setField( "x", fmt2( info.pos.x ) );
        setField( "y", fmt2( info.pos.y ) );
        setField( "z", fmt2( info.pos.z ) );
        setField( "p1", fmt2( h->getRadius() ) );
        setField( "p2", fmt2( h->getHeight() ) );
    }
}

std::unique_ptr<model::Primitive>
HexPrismFields::buildNew( const CommonValues& common )
{
    auto pos = parsePos();
    auto r   = parseField( "p1", []( double v ) { return v > 0; } );
    auto h   = parseField( "p2", []( double v ) { return v > 0; } );
    if ( !pos || !r || !h )
    {
        return nullptr;
    }
    auto obj = std::make_unique<model::HexPrism>( common.material,
                                                  *pos,
                                                  static_cast<float>( *r ),
                                                  static_cast<float>( *h ) );
    obj->setDisplayName( common.name );
    return obj;
}

bool
HexPrismFields::applyToExisting( const CommonValues& common, model::Primitive& obj )
{
    auto* h = dynamic_cast<model::HexPrism*>( &obj );
    if ( !h )
    {
        return false;
    }
    auto pos = parsePos();
    auto r   = parseField( "p1", []( double v ) { return v > 0; } );
    auto hh  = parseField( "p2", []( double v ) { return v > 0; } );
    if ( !pos || !r || !hh )
    {
        return false;
    }
    h->setOrigin( *pos );
    h->setMaterial( common.material );
    h->setRadius( static_cast<float>( *r ) );
    h->setHeight( static_cast<float>( *hh ) );
    h->setDisplayName( common.name );
    return true;
}

// --- Goursat ---------------------------------------------------------------
GoursatFields::GoursatFields( hui::WindowManager* wm,
                              hui::DialogBox*     parent,
                              const dr4::Vec2f&   size )
    : ObjEditFields( wm, parent, size, "Goursat" )
{
    float y = layoutNameField();
    layoutFields( { { "X", "x" }, { "Y", "y" }, { "Z", "z" }, { "ka", "ka" }, { "kb", "kb" } }, y );
}

void
GoursatFields::prefillDefaults()
{
    ObjEditFields::prefillDefaults();
    setPosDefaults( *this );
    setField( "ka", "1.00" );
    setField( "kb", "1.00" );
}

void
GoursatFields::prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj )
{
    ObjEditFields::prefillExisting( info, obj );
    if ( auto* g = dynamic_cast<model::Goursat*>( &obj ) )
    {
        setField( "x", fmt2( info.pos.x ) );
        setField( "y", fmt2( info.pos.y ) );
        setField( "z", fmt2( info.pos.z ) );
        setField( "ka", fmt2( g->getKa() ) );
        setField( "kb", fmt2( g->getKb() ) );
    }
}

std::unique_ptr<model::Primitive>
GoursatFields::buildNew( const CommonValues& common )
{
    auto pos = parsePos();
    auto ka  = parseField( "ka", []( double v ) { return v > 0; } );
    auto kb  = parseField( "kb", []( double v ) { return v > 0; } );
    if ( !pos || !ka || !kb )
    {
        return nullptr;
    }
    auto obj = std::make_unique<model::Goursat>( common.material,
                                                 *pos,
                                                 static_cast<float>( *ka ),
                                                 static_cast<float>( *kb ) );
    obj->setDisplayName( common.name );
    return obj;
}

bool
GoursatFields::applyToExisting( const CommonValues& common, model::Primitive& obj )
{
    auto* g = dynamic_cast<model::Goursat*>( &obj );
    if ( !g )
    {
        return false;
    }
    auto pos = parsePos();
    auto ka  = parseField( "ka", []( double v ) { return v > 0; } );
    auto kb  = parseField( "kb", []( double v ) { return v > 0; } );
    if ( !pos || !ka || !kb )
    {
        return false;
    }
    g->setOrigin( *pos );
    g->setMaterial( common.material );
    g->setKa( static_cast<float>( *ka ) );
    g->setKb( static_cast<float>( *kb ) );
    g->setDisplayName( common.name );
    return true;
}

// --- RoundedBox -------------------------------------------------------------
RoundedBoxFields::RoundedBoxFields( hui::WindowManager* wm,
                                    hui::DialogBox*     parent,
                                    const dr4::Vec2f&   size )
    : ObjEditFields( wm, parent, size, "RoundedBox" )
{
    float y = layoutNameField();
    layoutFields( { { "X", "x" },
                    { "Y", "y" },
                    { "Z", "z" },
                    { "Half size X", "p1" },
                    { "Half size Y", "p2" },
                    { "Half size Z", "p3" },
                    { "Radius", "p4" } },
                  y );
}

void
RoundedBoxFields::prefillDefaults()
{
    ObjEditFields::prefillDefaults();
    setPosDefaults( *this );
    setField( "p1", "1.00" );
    setField( "p2", "1.00" );
    setField( "p3", "1.00" );
    setField( "p4", "0.20" );
}

void
RoundedBoxFields::prefillExisting( const model::SceneManager::ObjectInfo& info,
                                   model::Primitive&                      obj )
{
    ObjEditFields::prefillExisting( info, obj );
    if ( auto* rb = dynamic_cast<model::RoundedBox*>( &obj ) )
    {
        auto hs = rb->getHalfSize();
        setField( "x", fmt2( info.pos.x ) );
        setField( "y", fmt2( info.pos.y ) );
        setField( "z", fmt2( info.pos.z ) );
        setField( "p1", fmt2( hs.x ) );
        setField( "p2", fmt2( hs.y ) );
        setField( "p3", fmt2( hs.z ) );
        setField( "p4", fmt2( rb->getRadius() ) );
    }
}

std::unique_ptr<model::Primitive>
RoundedBoxFields::buildNew( const CommonValues& common )
{
    auto pos = parsePos();
    auto p1  = parseField( "p1", []( double v ) { return v > 0; } );
    auto p2  = parseField( "p2", []( double v ) { return v > 0; } );
    auto p3  = parseField( "p3", []( double v ) { return v > 0; } );
    auto p4  = parseField( "p4", []( double v ) { return v > 0; } );
    if ( !pos || !p1 || !p2 || !p3 || !p4 )
    {
        return nullptr;
    }
    auto obj = std::make_unique<model::RoundedBox>( common.material,
                                                    *pos,
                                                    model::Vector3f( static_cast<float>( *p1 ),
                                                                     static_cast<float>( *p2 ),
                                                                     static_cast<float>( *p3 ) ),
                                                    static_cast<float>( *p4 ) );
    obj->setDisplayName( common.name );
    return obj;
}

bool
RoundedBoxFields::applyToExisting( const CommonValues& common, model::Primitive& obj )
{
    auto* rb = dynamic_cast<model::RoundedBox*>( &obj );
    if ( !rb )
    {
        return false;
    }
    auto pos = parsePos();
    auto p1  = parseField( "p1", []( double v ) { return v > 0; } );
    auto p2  = parseField( "p2", []( double v ) { return v > 0; } );
    auto p3  = parseField( "p3", []( double v ) { return v > 0; } );
    auto p4  = parseField( "p4", []( double v ) { return v > 0; } );
    if ( !pos || !p1 || !p2 || !p3 || !p4 )
    {
        return false;
    }
    rb->setOrigin( *pos );
    rb->setMaterial( common.material );
    rb->setHalfSize( model::Vector3f( static_cast<float>( *p1 ),
                                      static_cast<float>( *p2 ),
                                      static_cast<float>( *p3 ) ) );
    rb->setRadius( static_cast<float>( *p4 ) );
    rb->setDisplayName( common.name );
    return true;
}

// --- Ellipsoid --------------------------------------------------------------
EllipsoidFields::EllipsoidFields( hui::WindowManager* wm,
                                  hui::DialogBox*     parent,
                                  const dr4::Vec2f&   size )
    : ObjEditFields( wm, parent, size, "Ellipsoid" )
{
    float y = layoutNameField();
    layoutFields( { { "X", "x" },
                    { "Y", "y" },
                    { "Z", "z" },
                    { "Radius X", "p1" },
                    { "Radius Y", "p2" },
                    { "Radius Z", "p3" } },
                  y );
}

void
EllipsoidFields::prefillDefaults()
{
    ObjEditFields::prefillDefaults();
    setPosDefaults( *this );
    setField( "p1", "1.00" );
    setField( "p2", "1.50" );
    setField( "p3", "0.75" );
}

void
EllipsoidFields::prefillExisting( const model::SceneManager::ObjectInfo& info,
                                  model::Primitive&                      obj )
{
    ObjEditFields::prefillExisting( info, obj );
    if ( auto* e = dynamic_cast<model::Ellipsoid*>( &obj ) )
    {
        auto r = e->getRadii();
        setField( "x", fmt2( info.pos.x ) );
        setField( "y", fmt2( info.pos.y ) );
        setField( "z", fmt2( info.pos.z ) );
        setField( "p1", fmt2( r.x ) );
        setField( "p2", fmt2( r.y ) );
        setField( "p3", fmt2( r.z ) );
    }
}

std::unique_ptr<model::Primitive>
EllipsoidFields::buildNew( const CommonValues& common )
{
    auto pos = parsePos();
    auto p1  = parseField( "p1", []( double v ) { return v > 0; } );
    auto p2  = parseField( "p2", []( double v ) { return v > 0; } );
    auto p3  = parseField( "p3", []( double v ) { return v > 0; } );
    if ( !pos || !p1 || !p2 || !p3 )
    {
        return nullptr;
    }
    auto obj = std::make_unique<model::Ellipsoid>( common.material,
                                                   *pos,
                                                   model::Vector3f( static_cast<float>( *p1 ),
                                                                    static_cast<float>( *p2 ),
                                                                    static_cast<float>( *p3 ) ) );
    obj->setDisplayName( common.name );
    return obj;
}

bool
EllipsoidFields::applyToExisting( const CommonValues& common, model::Primitive& obj )
{
    auto* e = dynamic_cast<model::Ellipsoid*>( &obj );
    if ( !e )
    {
        return false;
    }
    auto pos = parsePos();
    auto p1  = parseField( "p1", []( double v ) { return v > 0; } );
    auto p2  = parseField( "p2", []( double v ) { return v > 0; } );
    auto p3  = parseField( "p3", []( double v ) { return v > 0; } );
    if ( !pos || !p1 || !p2 || !p3 )
    {
        return false;
    }
    e->setOrigin( *pos );
    e->setMaterial( common.material );
    e->setRadii( model::Vector3f( static_cast<float>( *p1 ),
                                  static_cast<float>( *p2 ),
                                  static_cast<float>( *p3 ) ) );
    e->setDisplayName( common.name );
    return true;
}

// --- Capsule ----------------------------------------------------------------
CapsuleFields::CapsuleFields( hui::WindowManager* wm,
                              hui::DialogBox*     parent,
                              const dr4::Vec2f&   size )
    : ObjEditFields( wm, parent, size, "Capsule" )
{
    float y = layoutNameField();
    layoutFields(
        { { "X", "x" }, { "Y", "y" }, { "Z", "z" }, { "Height", "p1" }, { "Radius", "p2" } },
        y );
}

void
CapsuleFields::prefillDefaults()
{
    ObjEditFields::prefillDefaults();
    setPosDefaults( *this );
    setField( "p1", "2.00" );
    setField( "p2", "0.50" );
}

void
CapsuleFields::prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj )
{
    ObjEditFields::prefillExisting( info, obj );
    if ( auto* c = dynamic_cast<model::Capsule*>( &obj ) )
    {
        auto  pa = c->getPaLocal();
        auto  pb = c->getPbLocal();
        float h  = std::abs( ( pb - pa ).y );
        setField( "x", fmt2( info.pos.x ) );
        setField( "y", fmt2( info.pos.y ) );
        setField( "z", fmt2( info.pos.z ) );
        setField( "p1", fmt2( h ) );
        setField( "p2", fmt2( c->getRadius() ) );
    }
}

std::unique_ptr<model::Primitive>
CapsuleFields::buildNew( const CommonValues& common )
{
    auto pos = parsePos();
    auto h   = parseField( "p1", []( double v ) { return v > 0; } );
    auto r   = parseField( "p2", []( double v ) { return v > 0; } );
    if ( !pos || !h || !r )
    {
        return nullptr;
    }
    float fh  = static_cast<float>( *h );
    auto  obj = std::make_unique<model::Capsule>( common.material,
                                                 *pos,
                                                 model::Vector3f{ 0.0f, -0.5f * fh, 0.0f },
                                                 model::Vector3f{ 0.0f, 0.5f * fh, 0.0f },
                                                 static_cast<float>( *r ) );
    obj->setDisplayName( common.name );
    return obj;
}

bool
CapsuleFields::applyToExisting( const CommonValues& common, model::Primitive& obj )
{
    auto* c = dynamic_cast<model::Capsule*>( &obj );
    if ( !c )
    {
        return false;
    }
    auto pos = parsePos();
    auto h   = parseField( "p1", []( double v ) { return v > 0; } );
    auto r   = parseField( "p2", []( double v ) { return v > 0; } );
    if ( !pos || !h || !r )
    {
        return false;
    }
    float fh = static_cast<float>( *h );
    c->setOrigin( *pos );
    c->setMaterial( common.material );
    c->setPaLocal( { 0.0f, -0.5f * fh, 0.0f } );
    c->setPbLocal( { 0.0f, 0.5f * fh, 0.0f } );
    c->setRadius( static_cast<float>( *r ) );
    c->setDisplayName( common.name );
    return true;
}

// --- RoundedCone ------------------------------------------------------------
RoundedConeFields::RoundedConeFields( hui::WindowManager* wm,
                                      hui::DialogBox*     parent,
                                      const dr4::Vec2f&   size )
    : ObjEditFields( wm, parent, size, "RoundedCone" )
{
    float y = layoutNameField();
    layoutFields( { { "X", "x" },
                    { "Y", "y" },
                    { "Z", "z" },
                    { "Height", "p1" },
                    { "Radius A", "p2" },
                    { "Radius B", "p3" } },
                  y );
}

void
RoundedConeFields::prefillDefaults()
{
    ObjEditFields::prefillDefaults();
    setPosDefaults( *this );
    setField( "p1", "2.00" );
    setField( "p2", "1.00" );
    setField( "p3", "0.50" );
}

void
RoundedConeFields::prefillExisting( const model::SceneManager::ObjectInfo& info,
                                    model::Primitive&                      obj )
{
    ObjEditFields::prefillExisting( info, obj );
    if ( auto* rc = dynamic_cast<model::RoundedCone*>( &obj ) )
    {
        auto  pa = rc->getPaLocal();
        auto  pb = rc->getPbLocal();
        float h  = std::abs( ( pb - pa ).y );
        setField( "x", fmt2( info.pos.x ) );
        setField( "y", fmt2( info.pos.y ) );
        setField( "z", fmt2( info.pos.z ) );
        setField( "p1", fmt2( h ) );
        setField( "p2", fmt2( rc->getRadiusA() ) );
        setField( "p3", fmt2( rc->getRadiusB() ) );
    }
}

std::unique_ptr<model::Primitive>
RoundedConeFields::buildNew( const CommonValues& common )
{
    auto pos = parsePos();
    auto h   = parseField( "p1", []( double v ) { return v > 0; } );
    auto ra  = parseField( "p2", []( double v ) { return v > 0; } );
    auto rb  = parseField( "p3", []( double v ) { return v > 0; } );
    if ( !pos || !h || !ra || !rb )
    {
        return nullptr;
    }
    float fh  = static_cast<float>( *h );
    auto  obj = std::make_unique<model::RoundedCone>( common.material,
                                                     *pos,
                                                     model::Vector3f{ 0.0f, -0.5f * fh, 0.0f },
                                                     model::Vector3f{ 0.0f, 0.5f * fh, 0.0f },
                                                     static_cast<float>( *ra ),
                                                     static_cast<float>( *rb ) );
    obj->setDisplayName( common.name );
    return obj;
}

bool
RoundedConeFields::applyToExisting( const CommonValues& common, model::Primitive& obj )
{
    auto* rc = dynamic_cast<model::RoundedCone*>( &obj );
    if ( !rc )
    {
        return false;
    }
    auto pos = parsePos();
    auto h   = parseField( "p1", []( double v ) { return v > 0; } );
    auto ra  = parseField( "p2", []( double v ) { return v > 0; } );
    auto rb  = parseField( "p3", []( double v ) { return v > 0; } );
    if ( !pos || !h || !ra || !rb )
    {
        return false;
    }
    float fh = static_cast<float>( *h );
    rc->setOrigin( *pos );
    rc->setMaterial( common.material );
    rc->setPaLocal( { 0.0f, -0.5f * fh, 0.0f } );
    rc->setPbLocal( { 0.0f, 0.5f * fh, 0.0f } );
    rc->setRadiusA( static_cast<float>( *ra ) );
    rc->setRadiusB( static_cast<float>( *rb ) );
    rc->setDisplayName( common.name );
    return true;
}

// --- CappedCone -------------------------------------------------------------
CappedConeFields::CappedConeFields( hui::WindowManager* wm,
                                    hui::DialogBox*     parent,
                                    const dr4::Vec2f&   size )
    : ObjEditFields( wm, parent, size, "CappedCone" )
{
    float y = layoutNameField();
    layoutFields( { { "X", "x" },
                    { "Y", "y" },
                    { "Z", "z" },
                    { "Height", "p1" },
                    { "Radius A", "p2" },
                    { "Radius B", "p3" } },
                  y );
}

void
CappedConeFields::prefillDefaults()
{
    ObjEditFields::prefillDefaults();
    setPosDefaults( *this );
    setField( "p1", "2.00" );
    setField( "p2", "1.00" );
    setField( "p3", "0.50" );
}

void
CappedConeFields::prefillExisting( const model::SceneManager::ObjectInfo& info,
                                   model::Primitive&                      obj )
{
    ObjEditFields::prefillExisting( info, obj );
    if ( auto* cc = dynamic_cast<model::CappedCone*>( &obj ) )
    {
        auto  pa = cc->getPaLocal();
        auto  pb = cc->getPbLocal();
        float h  = std::abs( ( pb - pa ).y );
        setField( "x", fmt2( info.pos.x ) );
        setField( "y", fmt2( info.pos.y ) );
        setField( "z", fmt2( info.pos.z ) );
        setField( "p1", fmt2( h ) );
        setField( "p2", fmt2( cc->getRadiusA() ) );
        setField( "p3", fmt2( cc->getRadiusB() ) );
    }
}

std::unique_ptr<model::Primitive>
CappedConeFields::buildNew( const CommonValues& common )
{
    auto pos = parsePos();
    auto h   = parseField( "p1", []( double v ) { return v > 0; } );
    auto ra  = parseField( "p2", []( double v ) { return v > 0; } );
    auto rb  = parseField( "p3", []( double v ) { return v > 0; } );
    if ( !pos || !h || !ra || !rb )
    {
        return nullptr;
    }
    float fh  = static_cast<float>( *h );
    auto  obj = std::make_unique<model::CappedCone>( common.material,
                                                    *pos,
                                                    model::Vector3f{ 0.0f, -0.5f * fh, 0.0f },
                                                    model::Vector3f{ 0.0f, 0.5f * fh, 0.0f },
                                                    static_cast<float>( *ra ),
                                                    static_cast<float>( *rb ) );
    obj->setDisplayName( common.name );
    return obj;
}

bool
CappedConeFields::applyToExisting( const CommonValues& common, model::Primitive& obj )
{
    auto* cc = dynamic_cast<model::CappedCone*>( &obj );
    if ( !cc )
    {
        return false;
    }
    auto pos = parsePos();
    auto h   = parseField( "p1", []( double v ) { return v > 0; } );
    auto ra  = parseField( "p2", []( double v ) { return v > 0; } );
    auto rb  = parseField( "p3", []( double v ) { return v > 0; } );
    if ( !pos || !h || !ra || !rb )
    {
        return false;
    }
    float fh = static_cast<float>( *h );
    cc->setOrigin( *pos );
    cc->setMaterial( common.material );
    cc->setPaLocal( { 0.0f, -0.5f * fh, 0.0f } );
    cc->setPbLocal( { 0.0f, 0.5f * fh, 0.0f } );
    cc->setRadiusA( static_cast<float>( *ra ) );
    cc->setRadiusB( static_cast<float>( *rb ) );
    cc->setDisplayName( common.name );
    return true;
}

// --- CappedCylinder ---------------------------------------------------------
CappedCylinderFields::CappedCylinderFields( hui::WindowManager* wm,
                                            hui::DialogBox*     parent,
                                            const dr4::Vec2f&   size )
    : ObjEditFields( wm, parent, size, "CappedCylinder" )
{
    float y = layoutNameField();
    layoutFields(
        { { "X", "x" }, { "Y", "y" }, { "Z", "z" }, { "Height", "p1" }, { "Radius", "p2" } },
        y );
}

void
CappedCylinderFields::prefillDefaults()
{
    ObjEditFields::prefillDefaults();
    setPosDefaults( *this );
    setField( "p1", "2.00" );
    setField( "p2", "0.50" );
}

void
CappedCylinderFields::prefillExisting( const model::SceneManager::ObjectInfo& info,
                                       model::Primitive&                      obj )
{
    ObjEditFields::prefillExisting( info, obj );
    if ( auto* c = dynamic_cast<model::CappedCylinder*>( &obj ) )
    {
        auto  a = c->getALocal();
        auto  b = c->getBLocal();
        float h = std::abs( ( b - a ).y );
        setField( "x", fmt2( info.pos.x ) );
        setField( "y", fmt2( info.pos.y ) );
        setField( "z", fmt2( info.pos.z ) );
        setField( "p1", fmt2( h ) );
        setField( "p2", fmt2( c->getRadius() ) );
    }
}

std::unique_ptr<model::Primitive>
CappedCylinderFields::buildNew( const CommonValues& common )
{
    auto pos = parsePos();
    auto h   = parseField( "p1", []( double v ) { return v > 0; } );
    auto r   = parseField( "p2", []( double v ) { return v > 0; } );
    if ( !pos || !h || !r )
    {
        return nullptr;
    }
    float fh  = static_cast<float>( *h );
    auto  obj = std::make_unique<model::CappedCylinder>( common.material,
                                                        *pos,
                                                        model::Vector3f{ 0.0f, -0.5f * fh, 0.0f },
                                                        model::Vector3f{ 0.0f, 0.5f * fh, 0.0f },
                                                        static_cast<float>( *r ) );
    obj->setDisplayName( common.name );
    return obj;
}

bool
CappedCylinderFields::applyToExisting( const CommonValues& common, model::Primitive& obj )
{
    auto* c = dynamic_cast<model::CappedCylinder*>( &obj );
    if ( !c )
    {
        return false;
    }
    auto pos = parsePos();
    auto h   = parseField( "p1", []( double v ) { return v > 0; } );
    auto r   = parseField( "p2", []( double v ) { return v > 0; } );
    if ( !pos || !h || !r )
    {
        return false;
    }
    float fh = static_cast<float>( *h );
    c->setOrigin( *pos );
    c->setMaterial( common.material );
    c->setALocal( { 0.0f, -0.5f * fh, 0.0f } );
    c->setBLocal( { 0.0f, 0.5f * fh, 0.0f } );
    c->setRadius( static_cast<float>( *r ) );
    c->setDisplayName( common.name );
    return true;
}

// --- Wedge ------------------------------------------------------------------
WedgeFields::WedgeFields( hui::WindowManager* wm, hui::DialogBox* parent, const dr4::Vec2f& size )
    : ObjEditFields( wm, parent, size, "Wedge" )
{
    float y = layoutNameField();
    layoutFields( { { "X", "x" },
                    { "Y", "y" },
                    { "Z", "z" },
                    { "Half size X", "p1" },
                    { "Half size Y", "p2" },
                    { "Half size Z", "p3" } },
                  y );
}

void
WedgeFields::prefillDefaults()
{
    ObjEditFields::prefillDefaults();
    setPosDefaults( *this );
    setField( "p1", "1.00" );
    setField( "p2", "1.00" );
    setField( "p3", "1.00" );
}

void
WedgeFields::prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj )
{
    ObjEditFields::prefillExisting( info, obj );
    if ( auto* w = dynamic_cast<model::Wedge*>( &obj ) )
    {
        auto s = w->getS();
        setField( "x", fmt2( info.pos.x ) );
        setField( "y", fmt2( info.pos.y ) );
        setField( "z", fmt2( info.pos.z ) );
        setField( "p1", fmt2( s.x ) );
        setField( "p2", fmt2( s.y ) );
        setField( "p3", fmt2( s.z ) );
    }
}

std::unique_ptr<model::Primitive>
WedgeFields::buildNew( const CommonValues& common )
{
    auto pos = parsePos();
    auto p1  = parseField( "p1", []( double v ) { return v > 0; } );
    auto p2  = parseField( "p2", []( double v ) { return v > 0; } );
    auto p3  = parseField( "p3", []( double v ) { return v > 0; } );
    if ( !pos || !p1 || !p2 || !p3 )
    {
        return nullptr;
    }
    auto obj = std::make_unique<model::Wedge>( common.material,
                                               *pos,
                                               model::Vector3f( static_cast<float>( *p1 ),
                                                                static_cast<float>( *p2 ),
                                                                static_cast<float>( *p3 ) ) );
    obj->setDisplayName( common.name );
    return obj;
}

bool
WedgeFields::applyToExisting( const CommonValues& common, model::Primitive& obj )
{
    auto* w = dynamic_cast<model::Wedge*>( &obj );
    if ( !w )
    {
        return false;
    }
    auto pos = parsePos();
    auto p1  = parseField( "p1", []( double v ) { return v > 0; } );
    auto p2  = parseField( "p2", []( double v ) { return v > 0; } );
    auto p3  = parseField( "p3", []( double v ) { return v > 0; } );
    if ( !pos || !p1 || !p2 || !p3 )
    {
        return false;
    }
    w->setOrigin( *pos );
    w->setMaterial( common.material );
    w->setS( model::Vector3f( static_cast<float>( *p1 ),
                              static_cast<float>( *p2 ),
                              static_cast<float>( *p3 ) ) );
    w->setDisplayName( common.name );
    return true;
}

// --- Ellipse ----------------------------------------------------------------
EllipseFields::EllipseFields( hui::WindowManager* wm,
                              hui::DialogBox*     parent,
                              const dr4::Vec2f&   size )
    : ObjEditFields( wm, parent, size, "Ellipse" )
{
    float y = layoutNameField();
    layoutFields( { { "X", "x" },
                    { "Y", "y" },
                    { "Z", "z" },
                    { "U.x", "ux" },
                    { "U.y", "uy" },
                    { "U.z", "uz" },
                    { "V.x", "vx" },
                    { "V.y", "vy" },
                    { "V.z", "vz" } },
                  y );
}

void
EllipseFields::prefillDefaults()
{
    ObjEditFields::prefillDefaults();
    setPosDefaults( *this );
    setField( "ux", "1.00" );
    setField( "uy", "0.00" );
    setField( "uz", "0.00" );
    setField( "vx", "0.00" );
    setField( "vy", "1.00" );
    setField( "vz", "0.00" );
}

void
EllipseFields::prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj )
{
    ObjEditFields::prefillExisting( info, obj );
    if ( auto* e = dynamic_cast<model::Ellipse*>( &obj ) )
    {
        auto u = e->getU();
        auto v = e->getV();
        setField( "x", fmt2( info.pos.x ) );
        setField( "y", fmt2( info.pos.y ) );
        setField( "z", fmt2( info.pos.z ) );
        setField( "ux", fmt2( u.x ) );
        setField( "uy", fmt2( u.y ) );
        setField( "uz", fmt2( u.z ) );
        setField( "vx", fmt2( v.x ) );
        setField( "vy", fmt2( v.y ) );
        setField( "vz", fmt2( v.z ) );
    }
}

std::unique_ptr<model::Primitive>
EllipseFields::buildNew( const CommonValues& common )
{
    auto pos = parsePos();
    auto ux  = parseField( "ux", []( double ) { return true; } );
    auto uy  = parseField( "uy", []( double ) { return true; } );
    auto uz  = parseField( "uz", []( double ) { return true; } );
    auto vx  = parseField( "vx", []( double ) { return true; } );
    auto vy  = parseField( "vy", []( double ) { return true; } );
    auto vz  = parseField( "vz", []( double ) { return true; } );
    if ( !pos || !ux || !uy || !uz || !vx || !vy || !vz )
    {
        return nullptr;
    }
    auto obj = std::make_unique<model::Ellipse>( common.material,
                                                 *pos,
                                                 model::Vector3f( static_cast<float>( *ux ),
                                                                  static_cast<float>( *uy ),
                                                                  static_cast<float>( *uz ) ),
                                                 model::Vector3f( static_cast<float>( *vx ),
                                                                  static_cast<float>( *vy ),
                                                                  static_cast<float>( *vz ) ) );
    obj->setDisplayName( common.name );
    return obj;
}

bool
EllipseFields::applyToExisting( const CommonValues& common, model::Primitive& obj )
{
    auto* e = dynamic_cast<model::Ellipse*>( &obj );
    if ( !e )
    {
        return false;
    }
    auto pos = parsePos();
    auto ux  = parseField( "ux", []( double ) { return true; } );
    auto uy  = parseField( "uy", []( double ) { return true; } );
    auto uz  = parseField( "uz", []( double ) { return true; } );
    auto vx  = parseField( "vx", []( double ) { return true; } );
    auto vy  = parseField( "vy", []( double ) { return true; } );
    auto vz  = parseField( "vz", []( double ) { return true; } );
    if ( !pos || !ux || !uy || !uz || !vx || !vy || !vz )
    {
        return false;
    }
    e->setOrigin( *pos );
    e->setMaterial( common.material );
    e->setU( model::Vector3f( static_cast<float>( *ux ),
                              static_cast<float>( *uy ),
                              static_cast<float>( *uz ) ) );
    e->setV( model::Vector3f( static_cast<float>( *vx ),
                              static_cast<float>( *vy ),
                              static_cast<float>( *vz ) ) );
    e->setDisplayName( common.name );
    return true;
}

// --- Triangle ---------------------------------------------------------------
TriangleFields::TriangleFields( hui::WindowManager* wm,
                                hui::DialogBox*     parent,
                                const dr4::Vec2f&   size )
    : ObjEditFields( wm, parent, size, "Triangle" )
{
    float y = layoutNameField();
    layoutFields( { { "V0.x", "v0x" },
                    { "V0.y", "v0y" },
                    { "V0.z", "v0z" },
                    { "V1.x", "v1x" },
                    { "V1.y", "v1y" },
                    { "V1.z", "v1z" },
                    { "V2.x", "v2x" },
                    { "V2.y", "v2y" },
                    { "V2.z", "v2z" } },
                  y );
}

void
TriangleFields::prefillDefaults()
{
    ObjEditFields::prefillDefaults();
    std::vector<std::pair<const char*, const char*>> defaults = { { "v0x", "0.0" },
                                                                  { "v0y", "0.0" },
                                                                  { "v0z", "0.0" },
                                                                  { "v1x", "1.0" },
                                                                  { "v1y", "0.0" },
                                                                  { "v1z", "0.0" },
                                                                  { "v2x", "0.0" },
                                                                  { "v2y", "1.0" },
                                                                  { "v2z", "0.0" } };
    for ( auto& p : defaults )
    {
        setField( p.first, p.second );
    }
}

void
TriangleFields::prefillExisting( const model::SceneManager::ObjectInfo& info,
                                 model::Primitive&                      obj )
{
    ObjEditFields::prefillExisting( info, obj );
    if ( auto* t = dynamic_cast<model::Triangle*>( &obj ) )
    {
        auto set = [&]( const char* key, const model::Vector3f& v, int comp ) {
            float val = comp == 0 ? v.x : ( comp == 1 ? v.y : v.z );
            setField( key, fmt2( val ) );
        };
        auto v0 = t->getV0();
        auto v1 = t->getV1();
        auto v2 = t->getV2();
        set( "v0x", v0, 0 );
        set( "v0y", v0, 1 );
        set( "v0z", v0, 2 );
        set( "v1x", v1, 0 );
        set( "v1y", v1, 1 );
        set( "v1z", v1, 2 );
        set( "v2x", v2, 0 );
        set( "v2y", v2, 1 );
        set( "v2z", v2, 2 );
    }
}

std::unique_ptr<model::Primitive>
TriangleFields::buildNew( const CommonValues& common )
{
    auto fetch = [&]( const char* key ) {
        return parseField( key, []( double ) { return true; } );
    };
    auto v0x = fetch( "v0x" ), v0y = fetch( "v0y" ), v0z = fetch( "v0z" );
    auto v1x = fetch( "v1x" ), v1y = fetch( "v1y" ), v1z = fetch( "v1z" );
    auto v2x = fetch( "v2x" ), v2y = fetch( "v2y" ), v2z = fetch( "v2z" );
    if ( !v0x || !v0y || !v0z || !v1x || !v1y || !v1z || !v2x || !v2y || !v2z )
    {
        return nullptr;
    }
    auto obj = std::make_unique<model::Triangle>( common.material,
                                                  model::Vector3f( static_cast<float>( *v0x ),
                                                                   static_cast<float>( *v0y ),
                                                                   static_cast<float>( *v0z ) ),
                                                  model::Vector3f( static_cast<float>( *v1x ),
                                                                   static_cast<float>( *v1y ),
                                                                   static_cast<float>( *v1z ) ),
                                                  model::Vector3f( static_cast<float>( *v2x ),
                                                                   static_cast<float>( *v2y ),
                                                                   static_cast<float>( *v2z ) ) );
    obj->setDisplayName( common.name );
    return obj;
}

bool
TriangleFields::applyToExisting( const CommonValues& common, model::Primitive& obj )
{
    auto* t = dynamic_cast<model::Triangle*>( &obj );
    if ( !t )
    {
        return false;
    }
    auto fetch = [&]( const char* key ) {
        return parseField( key, []( double ) { return true; } );
    };
    auto v0x = fetch( "v0x" ), v0y = fetch( "v0y" ), v0z = fetch( "v0z" );
    auto v1x = fetch( "v1x" ), v1y = fetch( "v1y" ), v1z = fetch( "v1z" );
    auto v2x = fetch( "v2x" ), v2y = fetch( "v2y" ), v2z = fetch( "v2z" );
    if ( !v0x || !v0y || !v0z || !v1x || !v1y || !v1z || !v2x || !v2y || !v2z )
    {
        return false;
    }
    t->setMaterial( common.material );
    t->setV0World( model::Vector3f( static_cast<float>( *v0x ),
                                    static_cast<float>( *v0y ),
                                    static_cast<float>( *v0z ) ) );
    t->setV1World( model::Vector3f( static_cast<float>( *v1x ),
                                    static_cast<float>( *v1y ),
                                    static_cast<float>( *v1z ) ) );
    t->setV2World( model::Vector3f( static_cast<float>( *v2x ),
                                    static_cast<float>( *v2y ),
                                    static_cast<float>( *v2z ) ) );
    t->setDisplayName( common.name );
    return true;
}

} // namespace view
} // namespace rta
