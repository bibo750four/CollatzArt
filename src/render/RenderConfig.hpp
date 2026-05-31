#pragma once
#include "app/Command.hpp"
#include <SFML/Graphics/Color.hpp>
#include <iostream>

struct RenderConfig {
    float                angle      { 7.f };
    float                segmentLen { 8.f };
    float                decay      { 0.95f };  // per modalità Decreasing
    Command::SegmentMode segmentMode{ Command::SegmentMode::Constant };
    Command::ColorMode   colorMode  { Command::ColorMode::Fixed };
    sf::Color            fixedColor { sf::Color::White };
    int                  speed      { 4 };       // 1..8
    
    RenderConfig() {
        std::cout << "[DEBUG] RenderConfig initial angle = " << angle << std::endl;
    }
    
};
