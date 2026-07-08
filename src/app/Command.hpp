#pragma once
#include <SFML/Graphics/Color.hpp>
#include <string>

struct Command
{
    enum class Type {
        Play, Pause, Reset,
        SpeedUp, SpeedDown,
        SetAngle,       // payload: floatVal (degrees)
        SetEvenAngle,   // payload: floatVal (degrees)
        SetOddAngle,    // payload: floatVal (degrees)
        SetSegmentLen,  // payload: floatVal (length)
        SetSegmentMode, // payload: segmentMode
        SetColorMode,   // payload: colorMode
        SetAnimationMode, // payload: animationMode
        SetRenderColor, // payload: color (sf::Color)
        SetBackgroundColor, // payload: color (sf::Color)
        ToggleFullscreen,
        BatchMode,      // payload: stringVal (job file path)
        Quit
    };

    enum class ColorMode    { Fixed, PerSequence, PerParity };
    enum class SegmentMode  { Constant, Decreasing };
    enum class AnimationMode { Parallel, Sequential };

    Type            type;
    float           floatVal    { 0.f };
    ColorMode       colorMode   { ColorMode::Fixed };
    SegmentMode     segmentMode { SegmentMode::Constant };
    AnimationMode   animationMode { AnimationMode::Parallel };
    sf::Color        color { sf::Color::Black };  // Used for SetRenderColor and SetBackgroundColor
    std::string      stringVal;  // Used for BatchMode (job file path)
};
