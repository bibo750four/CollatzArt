#pragma once

#include "Renderer.hpp"
#include "RenderConfig.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

class CollatzTreeRenderer : public Renderer {
public:
    CollatzTreeRenderer();
    ~CollatzTreeRenderer() override = default;

    // Override Renderer interface
    void build(const CollatzCollection& sequences, const RenderConfig& config, sf::Vector2u windowSize) override;
    void draw(sf::RenderWindow& window) const override;
    void applyConfig(const RenderConfig& config, sf::Vector2u windowSize) override;
    void recolor(const RenderConfig& config) override;
    void update(int steps) override;
    void reset() override;
    bool isDone() const override;
    std::string getName() const;
    
    // Adjust the window size to fit the tree
    void adjustWindowSize(sf::RenderWindow& window);

    // Set target configuration for smooth interpolation
    void setTargetConfig(const RenderConfig& targetConfig);

private:
    struct TreeNode {
        sf::Vector2f position;
        sf::Color color;
        std::vector<std::shared_ptr<TreeNode>> children;
    };

    void buildTree(const std::vector<long long>& sequence);
    void buildTreeRecursive(std::shared_ptr<TreeNode> node, const std::vector<long long>& sequence, size_t index, float currentAngle);
    void drawTree(sf::RenderWindow& window, const std::shared_ptr<TreeNode>& node, const sf::Vector2f& parentPosition, sf::VertexArray& vertices) const;

    RenderConfig config_;
    RenderConfig targetConfig_;
    float interpolationProgress_;
    std::shared_ptr<TreeNode> root_;
    sf::VertexArray vertices_;
    sf::Vector2f center_;
    float maxRadius_;
    bool isDone_;
    struct BoundingBox {
        float left;
        float top;
        float width;
        float height;
    } boundingBox_;  // Bounding box of the tree

    // Interpolate between current and target config
    void interpolateConfig(float progress);
};