#include "rta/view/annotator/tool_panel.hpp"
#include "custom-hui/button.hpp"
#include "custom-hui/window_manager.hpp"
#include "pp/tool.hpp"
#include <optional>
#include <vector>

bool
rta::view::ToolPanel::propagateEventToChildren( const hui::Event& event )
{
    for ( auto& btn : buttons_ )
    {
        if ( event.apply( btn.get() ) )
        {
            return true;
        }
    }

    return false;
}

void
rta::view::ToolPanel::RedrawMyTexture() const
{
    for ( const auto& btn : buttons_ )
    {
        btn->Redraw();
    }
}

rta::view::ToolPanel::ToolPanel( hui::WindowManager* wm, float x, float y, float visible_height )
    : hui::ContainerWidget( wm, { x, y }, { 50.0f + 2.0f * 10.0f, visible_height } ),
      font_( wm->getWindow()->GetDefaultFont() ),
      button_size_( 50.0f ),
      padding_( 10.0f )
{
    setDraggable( true );
    icon_font_.reset( wm->getWindow()->CreateFont() );
    try
    {
        icon_font_->LoadFromFile( "assets/CaskaydiaCoveNerdFontMono-Regular.ttf" );
        icon_font_loaded_ = true;
    } catch ( ... )
    {
        icon_font_loaded_ = false;
    }
}

void
rta::view::ToolPanel::addTools( const std::vector<pp::Tool*>& tools )
{
    tools_.clear();
    active_tool_idx_.reset();

    buttons_.clear();

    if ( tools.empty() )
    {
        setSize( { button_size_ + 2 * padding_, padding_ * 2 } );
        return;
    }

    tools_.assign( tools.begin(), tools.end() );

    for ( size_t i = 0; i < tools_.size(); ++i )
    {
        hui::Button::Theme theme{ { 30, 30, 30, 255 },
                                  { 50, 70, 30, 255 },
                                  { 100, 150, 0, 255 },
                                  { 220, 220, 220, 255 },
                                  35 };

        auto btn = std::make_unique<hui::Button>(
            wm_,
            dr4::Vec2f( padding_, padding_ + i * ( button_size_ + padding_ ) ),
            dr4::Vec2f( button_size_, button_size_ ),
            std::string( tools_[i]->Icon() ),
            theme );
        if ( icon_font_loaded_ )
        {
            btn->setFont( icon_font_.get() );
        }

        size_t capture_index = i;
        btn->setOnClick( [this, capture_index]() {
            if ( active_tool_idx_.has_value() )
            {
                tools_[active_tool_idx_.value()]->OnEnd();
            }
            active_tool_idx_ = capture_index;
            tools_[capture_index]->OnStart();
        } );

        btn->setParent( this );
        buttons_.push_back( std::move( btn ) );
    }

    float height = padding_ + tools_.size() * ( button_size_ + padding_ );
    setSize( { button_size_ + 2 * padding_, height } );
}

std::optional<size_t>
rta::view::ToolPanel::getActiveToolIdx() const
{
    return active_tool_idx_;
}

void
rta::view::ToolPanel::setActiveTool( std::optional<size_t> idx )
{
    active_tool_idx_ = idx;
}
