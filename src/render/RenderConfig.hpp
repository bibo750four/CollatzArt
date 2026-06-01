#pragma once
#include "app/Command.hpp"
#include <SFML/Graphics/Color.hpp>

struct RenderConfig {
    float                angle      { 5.f };
    float                segmentLen { 1.f };
    float                decay      { 0.95f };  // used in Decreasing segment mode
    Command::SegmentMode segmentMode{ Command::SegmentMode::Constant };
    Command::ColorMode   colorMode  { Command::ColorMode::Fixed };
    sf::Color            fixedColor { sf::Color::Black };
    int                  speed      { 4 };       // 1..8
};
