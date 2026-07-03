#pragma once

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
        ToggleFullscreen,
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
};
