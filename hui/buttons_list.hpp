#pragma once

#include "button.hpp"
#include "container_widget.hpp"
#include <memory>
#include <vector>

namespace hui {

class ButtonsList : public ContainerWidget {
  public:
    struct Theme
    {
        float         pad       = 0.0f;
        Button::Theme btn_theme = Button::Theme::Default();

        static Theme
        Default()
        {
            return Theme();
        }
    };

  public:
    explicit ButtonsList( WindowManager*    wm,
                          const dr4::Vec2f& pos,
                          const dr4::Vec2f& size,
                          Theme             theme = Theme::Default() );

    explicit ButtonsList( WindowManager*    wm,
                          const dr4::Vec2f& pos,
                          const dr4::Vec2f& size,
                          float             pad );

    Button*
    addButton( std::unique_ptr<Button> button );

    void
    clearButtons();

    void
    setPad( float pad );
    float
    getPad() const;

    void
    rebuildLayout();

    std::vector<std::unique_ptr<Button>>&
    getButtons();
    const std::vector<std::unique_ptr<Button>>&
    getButtons() const;

    bool
    propagateEventToChildren( const Event& event ) override;

    void
    RedrawMyTexture() const override;

  private:
    Theme                                theme_;
    std::vector<std::unique_ptr<Button>> btns_;
};

} // namespace hui
