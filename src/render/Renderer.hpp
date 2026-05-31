#pragma once
#include "RenderConfig.hpp"
#include "engine/CollatzEngine.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

class Renderer {
public:
    virtual ~Renderer() = default;

    // Builds the internal data structure from the Collatz collection
    virtual void build(const CollatzCollection&,
                       const RenderConfig&,
                       sf::Vector2u windowSize) = 0;

    // Recomputes geometry only (angle/length changed), resets animation
    virtual void applyConfig(const RenderConfig&, sf::Vector2u windowSize) = 0;

    // Recolors already-drawn shapes without touching geometry or animation
    virtual void recolor(const RenderConfig&) = 0;

    // Advances the animation by N steps
    virtual void update(int steps) = 0;

    virtual void draw(sf::RenderWindow&) const = 0;
    virtual void reset() = 0;
    virtual bool isDone() const = 0;
};
