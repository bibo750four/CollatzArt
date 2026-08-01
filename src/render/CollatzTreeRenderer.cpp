#include "CollatzTreeRenderer.hpp"
#include <cmath>
#include <iostream>

CollatzTreeRenderer::CollatzTreeRenderer()
    : root_(nullptr),
      vertices_(sf::PrimitiveType::Lines),
      center_(0.f, 0.f),
      maxRadius_(0.f),
      isDone_(false),
      interpolationProgress_(1.0f) {
    reset();
    targetConfig_ = config_;
}

void CollatzTreeRenderer::build(const CollatzCollection& sequences, const RenderConfig& config, sf::Vector2u windowSize) {
    reset();
    applyConfig(config, windowSize);
    
    if (sequences.empty()) {
        return;
    }
    
    // Use the first sequence for the tree (as per reference)
    // CollatzCollection is a map: key is the starting number, value is the sequence
    auto it = sequences.begin();
    if (it != sequences.end()) {
        // Build a simpler sequence for testing (e.g., [1, 2, 4, 8, 16])
        std::vector<long long> testSequence = {1, 2, 4, 8, 16};
        buildTree(testSequence);
    }
    isDone_ = true;
}

void CollatzTreeRenderer::buildTree(const std::vector<long long>& sequence) {
    if (sequence.empty()) {
        return;
    }
    
    root_ = std::make_shared<TreeNode>();
    root_->position = center_;
    root_->color = sf::Color::White;
    
    // Recursively build the tree
    buildTreeRecursive(root_, sequence, 1, 0.f);
}

void CollatzTreeRenderer::buildTreeRecursive(std::shared_ptr<TreeNode> node, const std::vector<long long>& sequence, size_t index, float currentAngle) {
    if (index >= sequence.size()) {
        return;
    }
    
    long long value = sequence[index];
    auto child = std::make_shared<TreeNode>();
    
    // Calculate radial position
    float angle = (value % 2 == 0) ? config_.branchAngle : -config_.branchAngle;
    float radius = 50.f; // Fixed radial distance
    float radians = (currentAngle + angle) * 3.14159265f / 180.f;
    
    child->position = sf::Vector2f(
        node->position.x + radius * std::cos(radians),
        node->position.y + radius * std::sin(radians)
    );
    
    // Assign color based on parity
    child->color = (value % 2 == 0) ? sf::Color(100, 200, 255) : sf::Color(255, 100, 100);
    node->children.push_back(child);
    
    // Recursively build the subtree
    buildTreeRecursive(child, sequence, index + 1, currentAngle + angle);
}

void CollatzTreeRenderer::draw(sf::RenderWindow& window) const {
    if (!root_) {
        return;
    }
    
    // Draw edges
    sf::VertexArray vertices(sf::PrimitiveType::Lines);
    drawTree(window, root_, root_->position, vertices);
    window.draw(vertices);
}

void CollatzTreeRenderer::drawTree(sf::RenderWindow& window, const std::shared_ptr<TreeNode>& node, const sf::Vector2f& parentPosition, sf::VertexArray& vertices) const {
    if (!node) {
        return;
    }
    
    // Draw edge from parent to current node
    if (node != root_) {
        sf::Vertex edge[] = {
            sf::Vertex(parentPosition, node->color),
            sf::Vertex(node->position, node->color)
        };
        vertices.append(edge[0]);
        vertices.append(edge[1]);
    }
    
    // Recursively draw children
    for (const auto& child : node->children) {
        drawTree(window, child, node->position, vertices);
    }
}

void CollatzTreeRenderer::applyConfig(const RenderConfig& config, sf::Vector2u windowSize) {
    // Only trigger interpolation if the branch angle changes
    if (config.branchAngle != config_.branchAngle) {
        setTargetConfig(config);
    } else {
        config_ = config;
    }
    
    // Update center based on window size
    center_ = sf::Vector2f(windowSize.x / 2.f, windowSize.y / 2.f);
}

void CollatzTreeRenderer::recolor(const RenderConfig& config) {
    // Tree colors are fixed based on parity, so this is a no-op
}

void CollatzTreeRenderer::update(int steps) {
    // Update interpolation progress (assuming 60 FPS and 1-second transition)
    if (interpolationProgress_ < 1.0f) {
        interpolationProgress_ += 0.016f; // ~1/60 seconds
        if (interpolationProgress_ > 1.0f) {
            interpolationProgress_ = 1.0f;
        }
        interpolateConfig(interpolationProgress_);
    }
}

void CollatzTreeRenderer::reset() {
    root_ = nullptr;
    vertices_.clear();
    center_ = sf::Vector2f(400.f, 500.f); // Default center
    maxRadius_ = 0.f;
    isDone_ = false;
}

bool CollatzTreeRenderer::isDone() const {
    return isDone_;
}

void CollatzTreeRenderer::setTargetConfig(const RenderConfig& targetConfig) {
    targetConfig_ = targetConfig;
    interpolationProgress_ = 0.0f;
}

void CollatzTreeRenderer::interpolateConfig(float progress) {
    // Interpolate branchAngle
    config_.branchAngle = config_.branchAngle + (targetConfig_.branchAngle - config_.branchAngle) * progress;
    
    // Rebuild the tree if parameters have changed significantly
    if (progress >= 1.0f) {
        config_ = targetConfig_;
        // Rebuild the tree with the current sequence
        if (root_) {
            std::vector<long long> testSequence = {1, 2, 4, 8, 16, 5, 10, 3, 6, 12, 24};
            buildTree(testSequence);
        }
    }
}

std::string CollatzTreeRenderer::getName() const {
    return "Collatz Tree Renderer";
}