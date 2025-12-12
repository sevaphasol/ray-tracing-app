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

    ButtonsList( WindowManager*    wm,
                 const dr4::Vec2f& pos,
                 const dr4::Vec2f& size,
                 Theme             theme = Theme::Default() );
    ButtonsList( WindowManager* wm, const dr4::Vec2f& pos, const dr4::Vec2f& size, float pad );

    virtual ~ButtonsList() = default;

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

  protected:
    virtual void
    layoutButtons() = 0;

  protected:
    Theme                                theme_;
    std::vector<std::unique_ptr<Button>> btns_;
};

class VerticalButtonsList : public ButtonsList {
  public:
    explicit VerticalButtonsList( WindowManager* wm, Theme theme = Theme::Default() );

    explicit VerticalButtonsList( WindowManager* wm, float pad );

    explicit VerticalButtonsList( WindowManager*    wm,
                                  const dr4::Vec2f& pos,
                                  const dr4::Vec2f& size,
                                  Theme             theme = Theme::Default() );

    explicit VerticalButtonsList( WindowManager*    wm,
                                  const dr4::Vec2f& pos,
                                  const dr4::Vec2f& size,
                                  float             pad );

  protected:
    void
    layoutButtons() override;
};

class HorizontalButtonsList : public ButtonsList {
  public:
    explicit HorizontalButtonsList( WindowManager* wm, Theme theme = Theme::Default() );

    explicit HorizontalButtonsList( WindowManager* wm, float pad );

    HorizontalButtonsList( WindowManager*    wm,
                           const dr4::Vec2f& pos,
                           const dr4::Vec2f& size,
                           Theme             theme = Theme::Default() );

    HorizontalButtonsList( WindowManager*    wm,
                           const dr4::Vec2f& pos,
                           const dr4::Vec2f& size,
                           float             pad );

    void
    setAutoWidthByLabel( bool enabled );

    void
    setLabelPadding( float padding );
    float
    getLabelPadding() const;

  protected:
    void
    layoutButtons() override;

  private:
    bool  auto_width_by_label_ = true;
    float label_padding_       = 8.0f;
};

} // namespace hui
