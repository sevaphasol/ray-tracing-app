#pragma once

#include "rta/view/panels/editor/generic_fields.hpp"
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
