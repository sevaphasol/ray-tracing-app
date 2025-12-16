#pragma once

#include "custom-hui/button.hpp"
#include "custom-hui/button_cancel.hpp"
#include "custom-hui/button_ok.hpp"
#include "custom-hui/closable_panel.hpp"
#include "custom-hui/dialog_box.hpp"
#include "custom-hui/scrollable_list_widget.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include "rta/model/rendering/scene_manager.hpp"
#include "rta/view/object_editor/generic_fields.hpp"
#include "rta/view/object_editor/types.hpp"
#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace rta {
namespace view {

class ObjectEditorPanel : public hui::ClosablePanel {
  public:
    explicit ObjectEditorPanel( hui::WindowManager*   wm,
                                model::SceneManager&  scene_manager,
                                const dr4::Vec2f&     pos,
                                const dr4::Vec2f&     size,
                                std::function<void()> objects_changed_cb = nullptr );

    void
    setTarget( std::optional<size_t> idx );

    bool
    propagateEventToChildren( const hui::Event& event ) override;
    void
    RedrawMyTexture() const override;

  private:
    std::unique_ptr<ObjEditFields>
    makeFields( ObjectType t );
    void
    switchToAdd();
    void
    switchToEdit( size_t idx );
    void
    wireButtons();
    void
    copyTarget();
    void
    deleteTarget();
    void
    notifyObjectsChanged();

    class TypePickerDialog : public hui::DialogBox {
      public:
        TypePickerDialog( hui::WindowManager*               wm,
                          float                             x,
                          float                             y,
                          ObjectType                        initial,
                          std::function<void( ObjectType )> on_ok,
                          std::function<void()>             on_cancel );

        bool
        propagateEventToChildren( const hui::Event& event ) override;
        void
        RedrawMyTexture() const override;

      private:
        void
        buildItems();

      private:
        hui::ScrollableListWidget         list_;
        hui::ButtonOk                     ok_;
        hui::ButtonCancel                 cancel_;
        std::function<void( ObjectType )> on_ok_;
        std::function<void()>             on_cancel_;
        ObjectType                        selected_ = ObjectType::Sphere;
    };

    class ObjEditorContent {
      public:
        virtual ~ObjEditorContent() = default;
        virtual bool
        propagateEvent( const hui::Event& ) = 0;
        virtual void
        redraw( dr4::Texture& target ) = 0;
        virtual bool
        onApply() = 0;
        virtual void
        prefill() = 0;
        virtual ObjectType
        type() const = 0;
        virtual ObjEditFields*
        fields() = 0;
        virtual const ObjEditFields*
        fields() const = 0;
    };

    class ObjEditorContentAdd : public ObjEditorContent {
      public:
        ObjEditorContentAdd( ObjectEditorPanel& panel, ObjectType type, model::SceneManager& sm );
        bool
        propagateEvent( const hui::Event& event ) override;
        void
        redraw( dr4::Texture& target ) override;
        bool
        onApply() override;
        void
        prefill() override;
        ObjectType
        type() const override
        {
            return type_;
        }
        ObjEditFields*
        fields() override
        {
            return fields_.get();
        }
        const ObjEditFields*
        fields() const override
        {
            return fields_.get();
        }
        void
        changeType( ObjectType t );

      private:
        ObjectEditorPanel&             panel_;
        ObjectType                     type_;
        model::SceneManager&           scene_manager_;
        std::unique_ptr<ObjEditFields> fields_;
    };

    class ObjEditorContentEdit : public ObjEditorContent {
      public:
        ObjEditorContentEdit( ObjectEditorPanel&   panel,
                              ObjectType           type,
                              size_t               idx,
                              model::SceneManager& sm );
        bool
        propagateEvent( const hui::Event& event ) override;
        void
        redraw( dr4::Texture& target ) override;
        bool
        onApply() override;
        void
        prefill() override;
        ObjectType
        type() const override
        {
            return type_;
        }
        ObjEditFields*
        fields() override
        {
            return fields_.get();
        }
        const ObjEditFields*
        fields() const override
        {
            return fields_.get();
        }

      private:
        ObjectEditorPanel&             panel_;
        ObjectType                     type_;
        size_t                         idx_;
        model::SceneManager&           scene_manager_;
        std::unique_ptr<ObjEditFields> fields_;
    };

  private:
    model::SceneManager&              scene_manager_;
    std::optional<size_t>             target_idx_;
    std::function<void()>             objects_changed_cb_;
    std::unique_ptr<ObjEditorContent> content_;

    hui::Button                type_btn_;
    hui::Button                add_btn_;
    hui::Button                copy_btn_;
    hui::Button                del_btn_;
    hui::Button                apply_btn_;
    std::unique_ptr<dr4::Text> label_text_;

    ObjectType current_type_;
    ObjectType creation_type_;
    bool       editing_mode_ = false;
    dr4::Vec2f size_;
};

} // namespace view
} // namespace rta
