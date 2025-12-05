#pragma once

#include "custom-hui-impl/container_widget.hpp"
#include "custom-hui-impl/widget.hpp"
#include "dr4/texture.hpp"
#include "zemax/config.hpp"
#include "zemax/view/control_panel.hpp"
#include "zemax/view/scene.hpp"
#include "zemax/view/snapshot_annotator.hpp"
#include <any>
#include <memory>

namespace zemax {
namespace view {

class Zemax : public hui::ContainerWidget {
  public:
    explicit Zemax( cum::Manager* pm, dr4::Window* window, dr4::Font* font )
        : hui::ContainerWidget( pm,
                                window,
                                { 0, 0 },
                                { Config::Window::Width, Config::Window::Height } ),
          scene_( pm,
                  window,
                  font,
                  Config::Scene::Position,
                  Config::Scene::Size,
                  Config::Scene::BackgroundColor,
                  Config::Camera::Position ),
          panel_( pm, window, font, scene_.getModel() ),
          snp_annotator_( pm, window, Config::Scene::Position, Config::Scene::Size, font )
    {
        // // fprintf( stderr, "debug in %s:%d:%s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__ );
        scene_.setParent( this );
        panel_.setParent( this );
        snp_annotator_.setParent( this );
    }

    ~Zemax() = default;

    bool
    propagateEventToChildren( const hui::Event& event ) override
    {
        if ( event.apply( &snp_annotator_ ) )
        {
            return true;
        }

        if ( event.apply( &panel_ ) )
        {
            return true;
        }

        if ( event.apply( &scene_ ) )
        {
            return true;
        }
        return false;
    }

    void
    RedrawMyTexture() const override
    {
        texture_->Clear( { 0, 0, 0 } );

        scene_.Redraw();
        panel_.Redraw();
        snp_annotator_.Redraw();
    }

  private:
    Scene             scene_;
    ControlPanel      panel_;
    SnapshotAnnotator snp_annotator_;
};

} // namespace view
} // namespace zemax
