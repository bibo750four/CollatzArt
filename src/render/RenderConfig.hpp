#pragma once
#include "app/Command.hpp"
#include <SFML/Graphics/Color.hpp>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <algorithm>   // std::clamp

// Helper function to parse sf::Color from a string (e.g., "255,0,0")
inline sf::Color parseColor(const std::string& colorStr) {
    std::istringstream iss(colorStr);
    std::string token;
    std::vector<int> rgb;

    while (std::getline(iss, token, ',')) {
        try {
            rgb.push_back(std::stoi(token));
        } catch (const std::exception&) {
            throw std::invalid_argument("Invalid color format. Use R,G,B (e.g., '255,0,0')");
        }
    }

    if (rgb.size() != 3) {
        throw std::invalid_argument("Invalid color format. Use R,G,B (e.g., '255,0,0')");
    }

    return sf::Color(
        static_cast<unsigned char>(std::clamp(rgb[0], 0, 255)),
        static_cast<unsigned char>(std::clamp(rgb[1], 0, 255)),
        static_cast<unsigned char>(std::clamp(rgb[2], 0, 255))
    );
}

struct RenderJob {
    int64_t range;
    int64_t step;
    float evenAngle = 12.0f;
    float oddAngle = 24.0f;
    float branchAngle = 30.0f;
    float segmentLen = 1.0f;
    Command::SegmentMode segmentMode = Command::SegmentMode::Constant;
    sf::Color color = sf::Color::Black;
    sf::Color background = sf::Color::White;
    int speed = 4;
    Command::AnimationMode mode = Command::AnimationMode::Parallel;
    std::string rendererType = "feather";  // Default to "feather" for backward compatibility
};

struct RenderConfig {
    std::string rendererType = "feather";  // "feather" or "tree"
    float                angle      { 5.f };    // Deprecated: use evenAngle and oddAngle instead
    // Angles for branches (degrees)
    float                evenAngle  { 12.f };    // Angle for even branches
    float                oddAngle   { 24.f };    // Angle for odd branches (feather)
    float                branchAngle{ 30.f };    // Angle for tree branches (tree)
    float                segmentLen { 1.f };
    float                decay      { 0.95f };  // used in Decreasing segment mode
    Command::SegmentMode segmentMode{ Command::SegmentMode::Constant };
    Command::ColorMode   colorMode  { Command::ColorMode::Fixed };
    sf::Color            fixedColor { sf::Color::Black };
    sf::Color            backgroundColor { sf::Color::White };  // Background color of the render window
    int                  speed      { 4 };       // 1..8
};
