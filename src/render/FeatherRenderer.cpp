#include "FeatherRenderer.hpp"
#include <numbers>
#include <cmath>
#include <algorithm>

// ─── build / config ──────────────────────────────────────────
void FeatherRenderer::build(const CollatzCollection& col,
                            const RenderConfig& cfg,
                            sf::Vector2u winSize)
{
    config_    = cfg;
    sf::Vector2f origin{
        static_cast<float>(winSize.x) * 0.5f,
        static_cast<float>(winSize.y) * 0.9f
    };
    tree_.build(col, cfg, origin);
    bfsOrder_  = tree_.getBFSOrder();
    maxWeight_ = tree_.maxWeight();
    shapes_.clear();
    shapes_.reserve(bfsOrder_.size());
    cursor_ = 0;
}

void FeatherRenderer::applyConfig(const RenderConfig& cfg, sf::Vector2u winSize)
{
    config_ = cfg;
    sf::Vector2f origin{
        static_cast<float>(winSize.x) * 0.5f,
        static_cast<float>(winSize.y) * 0.9f
    };
    tree_.recomputeGeometry(cfg, origin);
    bfsOrder_  = tree_.getBFSOrder();
    maxWeight_ = tree_.maxWeight();
    shapes_.clear();
    cursor_ = 0;
}

void FeatherRenderer::recolor(const RenderConfig& cfg)
{
    config_ = cfg;
    std::size_t prev = cursor_;
    shapes_.clear();
    cursor_ = 0;
    for (std::size_t i = 0; i < prev; ++i) addShape(i);
}

// ─── animation ───────────────────────────────────────────────
void FeatherRenderer::update(int steps)
{
    for (int i = 0; i < steps && cursor_ < bfsOrder_.size(); ++i, ++cursor_)
        addShape(cursor_);
}

void FeatherRenderer::addShape(std::size_t bfsIndex)
{
    const auto& nodes  = tree_.nodes();
    int         idx    = bfsOrder_[bfsIndex];
    const auto& node   = nodes[idx];
    const auto& parent = nodes[node.parentIdx];

    float t = (maxWeight_ > 1)
        ? static_cast<float>(node.weight - 1) / static_cast<float>(maxWeight_ - 1)
        : 0.f;
    float thickness = kMinThickness + t * (kMaxThickness - kMinThickness);

    shapes_.push_back(makeThickLine(parent.pos, node.pos, thickness, nodeColor(node)));
}

// ─── draw / state ─────────────────────────────────────────────
void FeatherRenderer::draw(sf::RenderWindow& window) const
{
    for (const auto& shape : shapes_) window.draw(shape);
}

void FeatherRenderer::reset()  { shapes_.clear(); cursor_ = 0; }
bool FeatherRenderer::isDone() const { return cursor_ >= bfsOrder_.size(); }

// ─── helpers ──────────────────────────────────────────────────
sf::Color FeatherRenderer::nodeColor(const FeatherNode& node) const
{
    switch (config_.colorMode) {
        case Command::ColorMode::Fixed:
            return config_.fixedColor;
        case Command::ColorMode::PerParity:
            return (node.value % 2 == 0)
                ? sf::Color{ 100, 149, 237, 255 }  // blue — pari
                : sf::Color{ 255, 140,   0, 255 };  // arancio — dispari
        case Command::ColorMode::PerSequence: {
            float hue = std::fmod(static_cast<float>(node.colorHint) * 137.508f, 360.f);
            return hsvToColor(hue, 0.75f, 0.9f);
        }
    }
    return sf::Color::White;
}

sf::RectangleShape FeatherRenderer::makeThickLine(
    sf::Vector2f from, sf::Vector2f to, float thickness, sf::Color color)
{
    sf::Vector2f delta = to - from;
    float length   = std::hypot(delta.x, delta.y);
    float angleDeg = std::atan2(delta.y, delta.x)
                     * (180.f / std::numbers::pi_v<float>);

    sf::RectangleShape shape(sf::Vector2f{ length, thickness });
    shape.setOrigin(sf::Vector2f{ 0.f, thickness * 0.5f });
    shape.setPosition(from);
    shape.setRotation(sf::degrees(angleDeg));
    shape.setFillColor(color);
    return shape;
}

sf::Color FeatherRenderer::hsvToColor(float h, float s, float v)
{
    float c = v * s;
    float x = c * (1.f - std::abs(std::fmod(h / 60.f, 2.f) - 1.f));
    float m = v - c;
    float r, g, b;
    if      (h <  60.f) { r = c; g = x; b = 0; }
    else if (h < 120.f) { r = x; g = c; b = 0; }
    else if (h < 180.f) { r = 0; g = c; b = x; }
    else if (h < 240.f) { r = 0; g = x; b = c; }
    else if (h < 300.f) { r = x; g = 0; b = c; }
    else                { r = c; g = 0; b = x; }
    return sf::Color{
        static_cast<uint8_t>((r + m) * 255.f),
        static_cast<uint8_t>((g + m) * 255.f),
        static_cast<uint8_t>((b + m) * 255.f)
    };
}