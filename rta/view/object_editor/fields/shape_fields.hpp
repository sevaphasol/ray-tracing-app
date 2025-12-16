#pragma once

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
#include "rta/view/object_editor/generic_fields.hpp"
#include <memory>

namespace rta {
namespace view {

class SphereFields : public ObjEditFields {
  public:
    SphereFields( hui::WindowManager* wm, hui::DialogBox* parent, const dr4::Vec2f& size );
    void
    prefillDefaults() override;
    void
    prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj ) override;
    std::unique_ptr<model::Primitive>
    buildNew( const CommonValues& common ) override;
    bool
    applyToExisting( const CommonValues& common, model::Primitive& obj ) override;
};

class AABBFields : public ObjEditFields {
  public:
    AABBFields( hui::WindowManager* wm, hui::DialogBox* parent, const dr4::Vec2f& size );
    void
    prefillDefaults() override;
    void
    prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj ) override;
    std::unique_ptr<model::Primitive>
    buildNew( const CommonValues& common ) override;
    bool
    applyToExisting( const CommonValues& common, model::Primitive& obj ) override;
};

class TorusFields : public ObjEditFields {
  public:
    TorusFields( hui::WindowManager* wm, hui::DialogBox* parent, const dr4::Vec2f& size );
    void
    prefillDefaults() override;
    void
    prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj ) override;
    std::unique_ptr<model::Primitive>
    buildNew( const CommonValues& common ) override;
    bool
    applyToExisting( const CommonValues& common, model::Primitive& obj ) override;
};

class HexPrismFields : public ObjEditFields {
  public:
    HexPrismFields( hui::WindowManager* wm, hui::DialogBox* parent, const dr4::Vec2f& size );
    void
    prefillDefaults() override;
    void
    prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj ) override;
    std::unique_ptr<model::Primitive>
    buildNew( const CommonValues& common ) override;
    bool
    applyToExisting( const CommonValues& common, model::Primitive& obj ) override;
};

class GoursatFields : public ObjEditFields {
  public:
    GoursatFields( hui::WindowManager* wm, hui::DialogBox* parent, const dr4::Vec2f& size );
    void
    prefillDefaults() override;
    void
    prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj ) override;
    std::unique_ptr<model::Primitive>
    buildNew( const CommonValues& common ) override;
    bool
    applyToExisting( const CommonValues& common, model::Primitive& obj ) override;
};

class RoundedBoxFields : public ObjEditFields {
  public:
    RoundedBoxFields( hui::WindowManager* wm, hui::DialogBox* parent, const dr4::Vec2f& size );
    void
    prefillDefaults() override;
    void
    prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj ) override;
    std::unique_ptr<model::Primitive>
    buildNew( const CommonValues& common ) override;
    bool
    applyToExisting( const CommonValues& common, model::Primitive& obj ) override;
};

class EllipsoidFields : public ObjEditFields {
  public:
    EllipsoidFields( hui::WindowManager* wm, hui::DialogBox* parent, const dr4::Vec2f& size );
    void
    prefillDefaults() override;
    void
    prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj ) override;
    std::unique_ptr<model::Primitive>
    buildNew( const CommonValues& common ) override;
    bool
    applyToExisting( const CommonValues& common, model::Primitive& obj ) override;
};

class CapsuleFields : public ObjEditFields {
  public:
    CapsuleFields( hui::WindowManager* wm, hui::DialogBox* parent, const dr4::Vec2f& size );
    void
    prefillDefaults() override;
    void
    prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj ) override;
    std::unique_ptr<model::Primitive>
    buildNew( const CommonValues& common ) override;
    bool
    applyToExisting( const CommonValues& common, model::Primitive& obj ) override;
};

class RoundedConeFields : public ObjEditFields {
  public:
    RoundedConeFields( hui::WindowManager* wm, hui::DialogBox* parent, const dr4::Vec2f& size );
    void
    prefillDefaults() override;
    void
    prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj ) override;
    std::unique_ptr<model::Primitive>
    buildNew( const CommonValues& common ) override;
    bool
    applyToExisting( const CommonValues& common, model::Primitive& obj ) override;
};

class CappedConeFields : public ObjEditFields {
  public:
    CappedConeFields( hui::WindowManager* wm, hui::DialogBox* parent, const dr4::Vec2f& size );
    void
    prefillDefaults() override;
    void
    prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj ) override;
    std::unique_ptr<model::Primitive>
    buildNew( const CommonValues& common ) override;
    bool
    applyToExisting( const CommonValues& common, model::Primitive& obj ) override;
};

class CappedCylinderFields : public ObjEditFields {
  public:
    CappedCylinderFields( hui::WindowManager* wm, hui::DialogBox* parent, const dr4::Vec2f& size );
    void
    prefillDefaults() override;
    void
    prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj ) override;
    std::unique_ptr<model::Primitive>
    buildNew( const CommonValues& common ) override;
    bool
    applyToExisting( const CommonValues& common, model::Primitive& obj ) override;
};

class WedgeFields : public ObjEditFields {
  public:
    WedgeFields( hui::WindowManager* wm, hui::DialogBox* parent, const dr4::Vec2f& size );
    void
    prefillDefaults() override;
    void
    prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj ) override;
    std::unique_ptr<model::Primitive>
    buildNew( const CommonValues& common ) override;
    bool
    applyToExisting( const CommonValues& common, model::Primitive& obj ) override;
};

class EllipseFields : public ObjEditFields {
  public:
    EllipseFields( hui::WindowManager* wm, hui::DialogBox* parent, const dr4::Vec2f& size );
    void
    prefillDefaults() override;
    void
    prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj ) override;
    std::unique_ptr<model::Primitive>
    buildNew( const CommonValues& common ) override;
    bool
    applyToExisting( const CommonValues& common, model::Primitive& obj ) override;
};

class TriangleFields : public ObjEditFields {
  public:
    TriangleFields( hui::WindowManager* wm, hui::DialogBox* parent, const dr4::Vec2f& size );
    void
    prefillDefaults() override;
    void
    prefillExisting( const model::SceneManager::ObjectInfo& info, model::Primitive& obj ) override;
    std::unique_ptr<model::Primitive>
    buildNew( const CommonValues& common ) override;
    bool
    applyToExisting( const CommonValues& common, model::Primitive& obj ) override;
};

} // namespace view
} // namespace rta
