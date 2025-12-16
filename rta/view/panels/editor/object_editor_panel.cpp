#include "rta/view/panels/editor/object_editor_panel.hpp"
#include "rta/view/panels/editor/fields/shape_fields.hpp"
#include "rta/view/panels/editor/types.hpp"
#include <utility>

namespace rta {
namespace view {

ObjectEditorPanel::ObjectEditorPanel( hui::WindowManager*   wm,
                                      model::SceneManager&  scene_manager,
                                      const dr4::Vec2f&     pos,
                                      const dr4::Vec2f&     size,
                                      std::function<void()> objects_changed_cb )
    : hui::ClosablePanel( wm, pos.x, pos.y, size.x, size.y, "Object Editor" ),
      scene_manager_( scene_manager ),
      objects_changed_cb_( std::move( objects_changed_cb ) ),
      label_text_( wm->getWindow()->CreateText() ),
      type_btn_( wm, { 12.0f, size.y - 34.0f }, { 88.0f, 24.0f }, "Type" ),
      add_btn_( wm, { 112.0f, size.y - 34.0f }, { 88.0f, 24.0f }, "Add" ),
      copy_btn_( wm, { 12.0f, size.y - 34.0f }, { 88.0f, 24.0f }, "Copy" ),
      del_btn_( wm, { 112.0f, size.y - 34.0f }, { 88.0f, 24.0f }, "Delete" ),
      apply_btn_( wm, { size.x - 100.0f, size.y - 34.0f }, { 88.0f, 24.0f }, "Apply" ),
      current_type_( ObjectType::Torus ),
      creation_type_( ObjectType::Torus ),
      size_( size )
{
    setDraggable( true );
    wireButtons();
    switchToAdd();
}

void
ObjectEditorPanel::setTarget( std::optional<size_t> idx )
{
    target_idx_ = idx;
    if ( idx.has_value() )
    {
        auto info      = scene_manager_.getObjectInfo( idx.value() );
        current_type_  = objectTypeFromCode( info.code );
        creation_type_ = current_type_;
        editing_mode_  = true;
        switchToEdit( idx.value() );
    } else
    {
        editing_mode_ = false;
        current_type_ = creation_type_;
        switchToAdd();
    }
}

bool
ObjectEditorPanel::propagateEventToChildren( const hui::Event& event )
{
    if ( !visible_ )
    {
        return false;
    }

    if ( editing_mode_ )
    {
        if ( event.apply( &copy_btn_ ) )
        {
            return true;
        }
        if ( event.apply( &del_btn_ ) )
        {
            return true;
        }
    } else
    {
        if ( event.apply( &type_btn_ ) )
        {
            return true;
        }
        if ( event.apply( &add_btn_ ) )
        {
            return true;
        }
    }

    if ( content_ && content_->propagateEvent( event ) )
    {
        return true;
    }

    if ( event.apply( &apply_btn_ ) )
    {
        return true;
    }

    return hui::DialogBox::propagateEventToChildren( event );
}

void
ObjectEditorPanel::RedrawMyTexture() const
{
    if ( !visible_ )
    {
        return;
    }

    hui::DialogBox::RedrawMyTexture();

    if ( content_ )
    {
        const_cast<ObjEditorContent*>( content_.get() )->redraw( *texture_ );
    }

    if ( editing_mode_ )
    {
        copy_btn_.Redraw();
        del_btn_.Redraw();
    } else
    {
        type_btn_.Redraw();
        add_btn_.Redraw();
    }
    apply_btn_.Redraw();

    const auto* font = wm_->getWindow()->GetDefaultFont();
    label_text_->SetFont( font );
    label_text_->SetFontSize( 14 );
    label_text_->SetColor( { 220, 220, 220, 255 } );
    label_text_->SetText( editing_mode_
                              ? "Editing"
                              : ( "Adding " + std::string( objectTypeName( current_type_ ) ) ) );
    label_text_->SetPos( { 12.0f, TopBarHeight + 4.0f } );
    label_text_->DrawOn( *texture_ );
}

std::unique_ptr<ObjEditFields>
ObjectEditorPanel::makeFields( ObjectType t )
{
    switch ( t )
    {
        case ObjectType::Sphere:
            return std::make_unique<SphereFields>( wm_, this, size_ );
        case ObjectType::AABB:
            return std::make_unique<AABBFields>( wm_, this, size_ );
        case ObjectType::Torus:
            return std::make_unique<TorusFields>( wm_, this, size_ );
        case ObjectType::HexPrism:
            return std::make_unique<HexPrismFields>( wm_, this, size_ );
        case ObjectType::Goursat:
            return std::make_unique<GoursatFields>( wm_, this, size_ );
        case ObjectType::RoundedBox:
            return std::make_unique<RoundedBoxFields>( wm_, this, size_ );
        case ObjectType::Ellipsoid:
            return std::make_unique<EllipsoidFields>( wm_, this, size_ );
        case ObjectType::Capsule:
            return std::make_unique<CapsuleFields>( wm_, this, size_ );
        case ObjectType::RoundedCone:
            return std::make_unique<RoundedConeFields>( wm_, this, size_ );
        case ObjectType::CappedCone:
            return std::make_unique<CappedConeFields>( wm_, this, size_ );
        case ObjectType::CappedCylinder:
            return std::make_unique<CappedCylinderFields>( wm_, this, size_ );
        case ObjectType::Wedge:
            return std::make_unique<WedgeFields>( wm_, this, size_ );
        case ObjectType::Ellipse:
            return std::make_unique<EllipseFields>( wm_, this, size_ );
        case ObjectType::Triangle:
            return std::make_unique<TriangleFields>( wm_, this, size_ );
        default:
            break;
    }

    return std::make_unique<SphereFields>( wm_, this, size_ );
}

void
ObjectEditorPanel::switchToAdd()
{
    editing_mode_ = false;
    content_      = std::make_unique<ObjEditorContentAdd>( *this, creation_type_, scene_manager_ );
    content_->prefill();
}

void
ObjectEditorPanel::switchToEdit( size_t idx )
{
    editing_mode_ = true;
    content_ = std::make_unique<ObjEditorContentEdit>( *this, current_type_, idx, scene_manager_ );
    content_->prefill();
}

void
ObjectEditorPanel::wireButtons()
{
    type_btn_.setParent( this );
    add_btn_.setParent( this );
    copy_btn_.setParent( this );
    del_btn_.setParent( this );
    apply_btn_.setParent( this );

    add_btn_.setOnClick( [this]() {
        if ( auto* add = dynamic_cast<ObjEditorContentAdd*>( content_.get() ) )
        {
            add->onApply();
        }
    } );
    type_btn_.setOnClick( [this]() {
        wm_->pushModal( std::make_unique<TypePickerDialog>(
            wm_,
            getRelPos().x + 20.0f,
            getRelPos().y + 20.0f,
            current_type_,
            [this]( ObjectType t ) {
                creation_type_ = t;
                current_type_  = t;
                if ( auto* add = dynamic_cast<ObjEditorContentAdd*>( content_.get() ) )
                {
                    add->changeType( t );
                }
                wm_->popModal();
            },
            [this]() { wm_->popModal(); } ) );
    } );
    copy_btn_.setOnClick( [this]() { copyTarget(); } );
    del_btn_.setOnClick( [this]() { deleteTarget(); } );
    apply_btn_.setOnClick( [this]() {
        if ( content_ )
        {
            content_->onApply();
        }
    } );
}

void
ObjectEditorPanel::copyTarget()
{
    if ( !target_idx_.has_value() )
    {
        return;
    }
    auto* target = scene_manager_.getTargetObj();
    if ( !target )
    {
        return;
    }
    auto  origin = target->getOrigin();
    float dx     = 0.2f;
    scene_manager_.copyTargetObj( origin.x + dx, origin.y, origin.z );
    target_idx_   = scene_manager_.getObjects().size() - 1;
    current_type_ = objectTypeFromCode( scene_manager_.getObjectInfo( target_idx_.value() ).code );
    scene_manager_.setTargetObj( scene_manager_.getObjects().back().get() );
    switchToEdit( target_idx_.value() );
    notifyObjectsChanged();
}

void
ObjectEditorPanel::deleteTarget()
{
    if ( !target_idx_.has_value() )
    {
        return;
    }
    scene_manager_.deleteTargetObj();
    scene_manager_.needUpdate() = true;
    target_idx_.reset();
    current_type_ = creation_type_;
    switchToAdd();
    notifyObjectsChanged();
}

void
ObjectEditorPanel::notifyObjectsChanged()
{
    if ( objects_changed_cb_ )
    {
        objects_changed_cb_();
    }
}

ObjectEditorPanel::TypePickerDialog::TypePickerDialog( hui::WindowManager*               wm,
                                                       float                             x,
                                                       float                             y,
                                                       ObjectType                        initial,
                                                       std::function<void( ObjectType )> on_ok,
                                                       std::function<void()>             on_cancel )
    : DialogBox( wm, x, y, 220.0f, 290.0f, on_cancel, "Choose Type" ),
      list_( wm, { 10.0f, TopBarHeight + 10.0f }, { 200.0f, 180.0f }, 10.0f ),
      ok_( wm, { 20.0f, 240.0f }, { 80.0f, 26.0f } ),
      cancel_( wm, { 120.0f, 240.0f }, { 80.0f, 26.0f } ),
      on_ok_( std::move( on_ok ) ),
      on_cancel_( std::move( on_cancel ) ),
      selected_( initial )
{
    list_.setParent( this );
    ok_.setParent( this );
    cancel_.setParent( this );

    ok_.setOnClick( [this]() {
        if ( on_ok_ )
        {
            on_ok_( selected_ );
        }
        if ( on_cancel_ )
        {
            on_cancel_();
        }
    } );
    cancel_.setOnClick( [this]() {
        if ( on_cancel_ )
        {
            on_cancel_();
        }
    } );

    buildItems();
}

bool
ObjectEditorPanel::TypePickerDialog::propagateEventToChildren( const hui::Event& event )
{
    if ( event.apply( &ok_ ) )
    {
        return true;
    }
    if ( event.apply( &list_ ) )
    {
        return true;
    }
    if ( event.apply( &cancel_ ) )
    {
        return true;
    }
    return DialogBox::propagateEventToChildren( event );
}

void
ObjectEditorPanel::TypePickerDialog::RedrawMyTexture() const
{
    DialogBox::RedrawMyTexture();
    list_.Redraw();
    ok_.Redraw();
    cancel_.Redraw();
}

void
ObjectEditorPanel::TypePickerDialog::buildItems()
{
    list_.clearItems();
    auto addItem = [&]( ObjectType t ) {
        auto label                   = std::string( objectTypeName( t ) );
        auto base_theme              = hui::Button::Theme::Default();
        auto selected_theme          = base_theme;
        selected_theme.default_color = base_theme.pressed_color;
        selected_theme.hovered_color = base_theme.hovered_color;
        selected_theme.pressed_color = base_theme.pressed_color;
        auto btn                     = std::make_unique<hui::Button>( wm_,
                                                  dr4::Vec2f{ 0, 0 },
                                                  dr4::Vec2f{ 180.0f, 32.0f },
                                                  label,
                                                  t == selected_ ? selected_theme : base_theme );
        btn->setOnClick( [this, t]() {
            selected_ = t;
            buildItems();
        } );
        list_.addItem( std::move( btn ) );
    };

    for ( auto t : allObjectTypes() )
    {
        addItem( t );
    }
    list_.rebuildLayout();
}

ObjectEditorPanel::ObjEditorContentAdd::ObjEditorContentAdd( ObjectEditorPanel&   panel,
                                                             ObjectType           type,
                                                             model::SceneManager& sm )
    : panel_( panel ), type_( type ), scene_manager_( sm )
{
    fields_ = panel_.makeFields( type_ );
}

bool
ObjectEditorPanel::ObjEditorContentAdd::propagateEvent( const hui::Event& event )
{
    return fields_ ? fields_->propagateEvent( event ) : false;
}

void
ObjectEditorPanel::ObjEditorContentAdd::redraw( dr4::Texture& target )
{
    if ( fields_ )
    {
        fields_->redraw( target );
    }
}

bool
ObjectEditorPanel::ObjEditorContentAdd::onApply()
{
    if ( !fields_ )
    {
        return false;
    }
    try
    {
        auto common  = fields_->parseCommon();
        auto created = fields_->buildNew( common );
        if ( !created )
        {
            return false;
        }
        created->setDisplayName( common.name );
        scene_manager_.getObjects().push_back( std::move( created ) );
        auto idx = scene_manager_.getObjects().size() - 1;
        scene_manager_.setTargetObj( scene_manager_.getObjects().back().get() );
        scene_manager_.needUpdate() = true;
        panel_.notifyObjectsChanged();
        panel_.setTarget( idx );
        return true;
    } catch ( ... )
    {
        return false;
    }
}

void
ObjectEditorPanel::ObjEditorContentAdd::prefill()
{
    if ( fields_ )
    {
        fields_->prefillDefaults();
    }
}

void
ObjectEditorPanel::ObjEditorContentAdd::changeType( ObjectType t )
{
    type_   = t;
    fields_ = panel_.makeFields( type_ );
    prefill();
}

ObjectEditorPanel::ObjEditorContentEdit::ObjEditorContentEdit( ObjectEditorPanel&   panel,
                                                               ObjectType           type,
                                                               size_t               idx,
                                                               model::SceneManager& sm )
    : panel_( panel ), type_( type ), idx_( idx ), scene_manager_( sm )
{
    fields_ = panel_.makeFields( type_ );
}

bool
ObjectEditorPanel::ObjEditorContentEdit::propagateEvent( const hui::Event& event )
{
    return fields_ ? fields_->propagateEvent( event ) : false;
}

void
ObjectEditorPanel::ObjEditorContentEdit::redraw( dr4::Texture& target )
{
    if ( fields_ )
    {
        fields_->redraw( target );
    }
}

bool
ObjectEditorPanel::ObjEditorContentEdit::onApply()
{
    if ( !fields_ )
    {
        return false;
    }
    if ( idx_ >= scene_manager_.getObjects().size() )
    {
        return false;
    }
    auto& obj = *scene_manager_.getObjects()[idx_];
    try
    {
        auto common = fields_->parseCommon();
        if ( !fields_->applyToExisting( common, obj ) )
        {
            return false;
        }
        scene_manager_.setTargetObj( &obj );
        scene_manager_.needUpdate() = true;
        panel_.notifyObjectsChanged();
        return true;
    } catch ( ... )
    {
        return false;
    }
}

void
ObjectEditorPanel::ObjEditorContentEdit::prefill()
{
    if ( !fields_ )
    {
        return;
    }
    auto info = scene_manager_.getObjectInfo( idx_ );
    fields_->prefillExisting( info, *scene_manager_.getObjects()[idx_] );
}

} // namespace view
} // namespace rta
