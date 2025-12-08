#pragma once

#include "custom-hui-impl/widget.hpp"
#include "dr4/math/color.hpp"
#include "dr4/texture.hpp"
#include "zemax/model/rendering/vector2.hpp"
#include "zemax/model/rendering/vector3.hpp"

#include "custom-hui-impl/window_manager.hpp"

#include "zemax/config.hpp"
#include "zemax/model/primitives/material.hpp"
#include "zemax/model/primitives/primitive.hpp"
#include "zemax/model/rendering/camera.hpp"
#include "zemax/model/rendering/scene_manager.hpp"
#include "zemax/view/closable_panel.hpp"
#include "zemax/view/info_panel.hpp"

#include <cstddef>
#include <iomanip>
#include <memory>
#include <sstream>
#include <thread>
#include <vector>

namespace zemax {
namespace view {

class Scene : public ClosablePanel {
  public:
    ~Scene() = default;
    using SelectionChangedCb = std::function<void(std::optional<size_t>)>;

    explicit Scene( hui::WindowManager*           wm,
                    const dr4::Vec2f&             pos,
                    const dr4::Vec2f&             size,
                    const dr4::Color&             background_color,
                    const zemax::model::Vector3f& camera_pos )
        : ClosablePanel( wm, pos.x, pos.y, size.x, size.y, "Scene" ),
          model_( zemax::Config::Camera::Position, size.x, size.y ),
          background_color_( background_color ),
          info_panel_( wm, Config::Scene::ObjInfoPanel::Size )
    {
        setDraggable( true );

        auto content_pos  = contentOffset();
        auto content_size = contentSize();

        border_.reset( wm->getWindow()->CreateRectangle() );
        border_->SetPos( content_pos );
        border_->SetSize( content_size );
        border_->SetFillColor( { 0, 0, 0, 0 } );
        border_->SetBorderColor( { 118, 185, 0, 255 } );
        border_->SetBorderThickness( -2.0f );

        pixels_.reset( wm->getWindow()->CreateImage() );
        pixels_->SetSize( content_size );
        pixels_->SetPos( content_pos );

        camera_pos_text_.reset( wm->getWindow()->CreateText() );
        select_rect_.reset( wm->getWindow()->CreateRectangle() );

        info_panel_.setParent( this );

        camera_pos_text_->SetFont( wm->getWindow()->GetDefaultFont() );
        camera_pos_text_->SetColor( { 255, 255, 255, 255 } );
        camera_pos_text_->SetFontSize( 16 );
        camera_pos_text_->SetPos( { content_pos.x + 5.0f, content_pos.y + 5.0f } );

        model_.addLight( zemax::model::Vector3f( 3, 3, -3 ), 1.0, 0.3, 0.9 );
        model_.addLight( zemax::model::Vector3f( 0, 0, -11 ), 0.2, 0.3, 0.9 );

        // model_.addTorus( model::Material( zemax::model::Color( 118, 185, 0 ) ), { 0, 0, -5 }, 1,
        // 2
        // );

        model_.addAABB( model::Material( zemax::model::Color( 255, 8, 8 ), 0.8f ),
                        zemax::model::Vector3f( -2, 1, -8 ),
                        zemax::model::Vector3f( 0.75, 0.75, -6.75 ) );

        // model_.addAABB( model::Material( zemax::model::Color( 8, 8, 8 ), 0.5f ),
        //                 zemax::model::Vector3f( -1, 0.5, -5 ),
        //                 zemax::model::Vector3f( 0.25, 0.25, 0.25 ) );

        model_.addSphere( model::Material( zemax::model::Color( 8, 32, 8 ), 1.0f ),
                          zemax::model::Vector3f( 0, 0, -13 ),
                          0.3 );

        model_.addSphere( model::Material( zemax::model::Color( 8, 32, 8 ), 0.3f ),
                          zemax::model::Vector3f( 2, 2, -13 ),
                          1.5 );

        model_.addSphere( model::Material( zemax::model::Color( 118, 185, 0 ), 0.9f ),
                          zemax::model::Vector3f( 0, 3, -16 ),
                          1.5 );

        model_.addSphere( model::Material( zemax::model::Color( 118, 185, 0 ), 0.0, 0.7f, 0.8f ),
                          zemax::model::Vector3f( -1.5, -0.5, -13.6 ),
                          1.5 );

        model_.addSphere( model::Material( zemax::model::Color( 118, 185, 0 ), 0.0, 0.7f, 0.8f ),
                          zemax::model::Vector3f( 1.2, -1.4, -14.1 ),
                          1.0 );

        // model_.addAABB( model::Material( zemax::model::Color( 118, 185, 0 ), 0.9f
        // ),
        //                 zemax::model::Vector3f( 0, -1, -8 ),
        //                 { 1.5, 0.5, 1.5 } );

        model_.addAABB( model::Material( zemax::model::Color( 118, 185, 0 ), 0.95f ),
                        zemax::model::Vector3f( -4, 0, -12 ),
                        { 0.1, 3, 3 } );

        model_.addAABB( model::Material( zemax::model::Color( 118, 185, 0 ), 0.95f ),
                        zemax::model::Vector3f( 4, 0, -12 ),
                        { 0.1, 3, 3 } );

        model_.addAABB( model::Material( zemax::model::Color( 118, 185, 0 ), 0.95f ),
                        zemax::model::Vector3f( 0, 0, -18 ),
                        { 3, 3, 0.1 } );

        // model_.addSphere( model::Material( zemax::model::Color( 8, 32, 8 ), 0.3f
        // ),
        //   zemax::model::Vector3f( -2, 1, -1 ),
        //   1.5 );

        model_.addAABB( model::Material( zemax::model::Color( 58, 90, 0 ) ),
                        zemax::model::Vector3f( 0, -3.5f, -12 ),
                        zemax::model::Vector3f( 5.0f, 0.1, 5.0f ) );
        model_.addAABB( model::Material( zemax::model::Color( 58, 90, 0 ) ),
                        zemax::model::Vector3f( 0, 5.0f, -12 ),
                        zemax::model::Vector3f( 5.0f, 0.1, 5.0f ) );

        // model_.addHexPrism( model::Material( zemax::model::ColorColor( 32, 255, 32 ), 0.3f ),
        // zemax::model::Vector3f( -3, 0, -6 ),
        // 1.0f, // R
        // 2.0f  // r
        // );

        // model_.addPlane( model::Material( zemax::model::ColorColor( 1, 8, 127 ), 0.5f
        // ),
        //  zemax::model::Vector3f( -5, -5, -5 ),
        //  zemax::model::Vector3f( 1, 1, 1 ) );
    }

    void
    setFont( const dr4::Font* font )
    {
        info_panel_.setFont( font );
    }

    void
    setOnSelectionChanged( SelectionChangedCb cb )
    {
        on_selection_changed_ = std::move( cb );
    }

    model::SceneManager&
    getModel()
    {
        need_update_ = true;

        return model_;
    }

    virtual bool
    onIdle( const hui::Event& event ) override final
    {
        if ( !visible_ )
        {
            return false;
        }
        update();
        model_.needUpdate() = false;

        return false;
    }

    virtual bool
    onMousePress( const hui::Event& event ) override final
    {
        if ( !visible_ )
        {
            return false;
        }

        if ( !isHovered() )
        {
            return false;
        }

        auto content_pos = contentOffset();
        auto px          = event.info.mouseButton.pos.x - getAbsPos().x - content_pos.x;
        auto py          = event.info.mouseButton.pos.y - getAbsPos().y - content_pos.y;

        if ( py < 0 )
        {
            return false;
        }

        model::Primitive* obj = model_.getIntersectedObj( px, py );

        // if ( model_.getTargetObj() != nullptr )
        // {
        // model_.getTargetObj()->revert_paint();
        // }

        model_.setTargetObj( obj );

        if ( obj != nullptr )
        {
            auto content_pos = contentOffset();
            info_panel_.setRelPos( px + content_pos.x, py + content_pos.y );
            info_panel_.update( obj );
            info_panel_.setVisible( true );
            if ( on_selection_changed_ )
            {
                on_selection_changed_( findIndexForObj( obj ) );
            }
        } else
        {
            info_panel_.setVisible( false );
            if ( on_selection_changed_ )
            {
                on_selection_changed_( std::nullopt );
            }
        }

        if ( ObjInfoBox::propagateEventToChildren( event ) )
        {
            return true;
        }

        // Let base handle dragging/topbar
        if ( hui::Widget::onMousePress( event ) )
        {
            return true;
        }

        // if ( obj != nullptr )
        // {
        // model_.getTargetObj()->paint();
        // }

        return true;
    }

  private:
    void
    update()
    {
        if ( !need_update_ && !model_.needUpdate() )
        {
            return;
        }

        auto         content_size = contentSize();
        const size_t w            = content_size.x;
        const size_t h            = content_size.y;

        constexpr size_t         num_threads     = 16;
        const size_t             rows_per_thread = h / num_threads;
        std::vector<std::thread> workers;
        workers.reserve( num_threads );

        for ( size_t t = 0; t < num_threads; ++t )
        {
            const size_t start_row = t * rows_per_thread;
            const size_t end_row   = ( t == num_threads - 1 ) ? h : ( t + 1 ) * rows_per_thread;

            workers.emplace_back( [this, w, start_row, end_row]() {
                for ( size_t row = start_row; row < end_row; ++row )
                {
                    for ( size_t col = 0; col < w; ++col )
                    {
                        zemax::model::Color color =
                            model_.calcPixelColor( row,
                                                   col,
                                                   { background_color_.r,
                                                     background_color_.g,
                                                     background_color_.b,
                                                     background_color_.a } );
                        pixels_->SetPixel( col, row, { color.r, color.g, color.b, color.a } );
                    }
                }
            } );
        }

        for ( auto& worker : workers )
        {
            worker.join();
        }

        setCameraPosString();

        need_update_ = false;
    }

    void
    setCameraPosString()
    {
        auto cam_pos = model_.getCamera().getPos();

        std::ostringstream oss;
        oss << "( " << std::fixed << std::setprecision( 1 ) << cam_pos.x << ", " << cam_pos.y
            << ", " << cam_pos.z << " )";

        camera_pos_text_->SetText( oss.str() );
    }

  public:
    void
    RedrawMyTexture() const override final
    {
        if ( !visible_ )
        {
            return;
        }

        ObjInfoBox::RedrawMyTexture();

        texture_->Draw( *pixels_ );

        // window.draw( border_, widget_transform );

        // window.draw( pixels_.data(),
        //  pixels_.size(),
        //  gfx::core::PrimitiveType::Points,
        //  widget_transform );

        if ( model_.getTargetObj() != nullptr )
        {
            auto corners3d = model_.getTargetObj()->getCircumscribedAABB();

            float min_x = std::numeric_limits<float>::max();
            float min_y = std::numeric_limits<float>::max();
            float max_x = -std::numeric_limits<float>::max();
            float max_y = -std::numeric_limits<float>::max();

            auto  content_pos  = contentOffset();
            auto  content_size = contentSize();
            float scr_w        = content_size.x;
            float scr_h        = content_size.y;

            for ( const auto& p3d : corners3d )
            {
                auto p2d = model_.getCamera().projectToScreen( p3d );

                if ( p2d.has_value() )
                {
                    min_x = std::max( 0.0f, std::min( min_x, p2d.value().x ) );
                    max_x = std::min( scr_w, std::max( max_x, p2d.value().x ) );
                    min_y = std::max( 0.0f, std::min( min_y, p2d.value().y ) );
                    max_y = std::min( scr_h, std::max( max_y, p2d.value().y ) );
                }
            }

            // std::vector<gfx::core::Vertex> outline = {
            // { { min_x, min_y }, dr4::Color::Red },
            // { { max_x, min_y }, dr4::Color::Red },
            // { { max_x, max_y }, dr4::Color::Red },
            // { { min_x, max_y }, dr4::Color::Red },
            // { { min_x, min_y }, dr4::Color::Red } };

            select_rect_->SetPos( { min_x + content_pos.x, min_y + content_pos.y } );
            select_rect_->SetSize( { max_x - min_x, max_y - min_y } );
            select_rect_->SetFillColor( { 0, 0, 0, 0 } );
            select_rect_->SetBorderColor( { 255, 0, 0, 255 } );
            select_rect_->SetBorderThickness( -2.0f );

            texture_->Draw( *select_rect_ );
            //
            // window.draw( outline.data(),
            //  outline.size(),
            //  gfx::core::PrimitiveType::LineStrip,
            //  widget_transform );
        }

        info_panel_.Redraw();
        texture_->Draw( *camera_pos_text_ );
        texture_->Draw( *border_ );

        // Draw topbar/close last so it stays visible
    }

    void
    show()
    {
        visible_ = true;
    }

    void
    hide()
    {
        visible_ = false;
    }

    bool
    isVisible() const
    {
        return visible_;
    }

    std::optional<size_t>
    findIndexForObj( model::Primitive* obj ) const
    {
        const auto& objects = model_.getObjects();
        for ( size_t i = 0; i < objects.size(); ++i )
        {
            if ( objects[i].get() == obj )
            {
                return i;
            }
        }
        return std::nullopt;
    }

  private:
    bool need_update_ = true;
    SelectionChangedCb on_selection_changed_;

    std::unique_ptr<dr4::Text>      camera_pos_text_;
    std::unique_ptr<dr4::Rectangle> select_rect_;
    std::unique_ptr<dr4::Rectangle> border_;
    model::SceneManager             model_;
    dr4::Color                      background_color_;
    view::ObjInfoPanel              info_panel_;
    std::unique_ptr<dr4::Image>     pixels_;
};

} // namespace view
} // namespace zemax
