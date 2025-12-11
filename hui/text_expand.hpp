#pragma once

#include "dr4/math/rect.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"

void
moveInCenterOfRect( dr4::Text&        text,
                    const dr4::Vec2f& rect_size,
                    const dr4::Vec2f& rect_pos = { 0, 0 } )
{
    const auto& text_bounds = text.GetBounds();

    text.SetPos( rect_size.x * 0.5f - text_bounds.x * 0.5f,
                 rect_size.y * 0.5f - text_bounds.y * 0.5f );
}
