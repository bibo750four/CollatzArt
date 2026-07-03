#pragma once
#include "Renderer.hpp"
#include "FeatherTree.hpp"
#include "engine/CollatzEngine.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <vector>
#include <algorithm>

class FeatherRenderer : public Renderer {
public:
    void build(const CollatzCollection&, const RenderConfig&,
               sf::Vector2u windowSize) override;

    void applyConfig(const RenderConfig&, sf::Vector2u windowSize) override;
    void recolor(const RenderConfig&) override;
    void update(int steps) override;
    void draw(sf::RenderWindow&) const override;
    void reset() override;
    bool isDone() const override;
    void setViewToFitTree(sf::Vector2u windowSize);
    void setAnimationMode(Command::AnimationMode mode, const CollatzCollection& collection);

private:
    static sf::Color          hsvToColor(float h, float s, float v);
    sf::Color                 nodeColor(const FeatherNode& node) const;
    static sf::RectangleShape makeThickLine(sf::Vector2f from, sf::Vector2f to,
                                            float thickness, sf::Color color);
    void                      addShape(std::size_t index, const std::vector<int>& order);
    sf::FloatRect              computeTreeBounds() const;
    void                      buildSequentialOrder();

    // Returns the window centre as a Vector2f
    static sf::Vector2f windowCentre(sf::Vector2u size) {
        return { static_cast<float>(size.x) * 0.5f,
                 static_cast<float>(size.y) * 0.5f };
    }

    FeatherTree                     tree_;
    CollatzCollection               collection_;
    std::vector<int>                bfsOrder_;
    std::vector<int>                sequentialOrder_;
    std::size_t                     cursor_    { 0 };
    int                             maxWeight_ { 1 };
    std::vector<sf::RectangleShape> shapes_;
    RenderConfig                    config_;
    sf::View                        view_;
    Command::AnimationMode           animationMode_ { Command::AnimationMode::Parallel };

    static constexpr float kMinThickness = 1.f;
    static constexpr float kMaxThickness = 8.f;
};
