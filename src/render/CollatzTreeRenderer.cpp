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
        // Use the full sequence from the CollatzCollection
        buildTree(it->second);
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
    
    // Initialize bounding box with the root node
    boundingBox_.left = center_.x;
    boundingBox_.top = center_.y;
    boundingBox_.width = 0.f;
    boundingBox_.height = 0.f;
    
    // Debug print
    std::cout << "Building tree for sequence: ";
    for (auto val : sequence) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    
    // Recursively build the tree
    buildTreeRecursive(root_, sequence, 0, 0.f, 0);
}

void CollatzTreeRenderer::buildTreeRecursive(std::shared_ptr<TreeNode> node, const std::vector<long long>& sequence, size_t index, float currentAngle, int depth) {
    if (index >= sequence.size()) {
        return;
    }
    
    long long value = sequence[index];
    
    // Add even child (n * 2)
    auto evenChild = std::make_shared<TreeNode>();
    float evenAngle = currentAngle + config_.branchAngle * (3.14159265f / 180.f);
    float radius = 50.f;
    evenChild->position = sf::Vector2f(
        node->position.x + radius * std::cos(evenAngle),
        node->position.y - radius * std::sin(evenAngle)
    );
    
    // Assign color based on depth
    int colorIndex = depth % 40;
    if (colorIndex < 20) {
        evenChild->color = sf::Color(255, 100 + colorIndex * 5, 100);
    } else {
        evenChild->color = sf::Color(100, 100 + (colorIndex - 20) * 5, 255);
    }
    node->children.push_back(evenChild);
    
    // Add odd child ((n - 1) / 3) if applicable
    if (value > 1 && (value - 1) % 3 == 0) {
        long long oddValue = (value - 1) / 3;
        auto oddChild = std::make_shared<TreeNode>();
        float oddAngle = currentAngle - config_.branchAngle * 2 * (3.14159265f / 180.f);
        oddChild->position = sf::Vector2f(
            node->position.x + radius * std::cos(oddAngle),
            node->position.y - radius * std::sin(oddAngle)
        );
        oddChild->color = sf::Color(255, 100, 100);
        node->children.push_back(oddChild);
        
        // Only recurse on the odd child if it matches the sequence
        if (index + 1 < sequence.size() && sequence[index + 1] == oddValue) {
            buildTreeRecursive(oddChild, sequence, index + 1, oddAngle, depth + 1);
        }
    }
    
    // Only recurse on the even child if it matches the sequence
    if (index + 1 < sequence.size() && sequence[index + 1] == value * 2) {
        buildTreeRecursive(evenChild, sequence, index + 1, evenAngle, depth + 1);
    }
    
    // Update bounding box
    for (const auto& child : node->children) {
        if (boundingBox_.width == 0.f && boundingBox_.height == 0.f) {
            boundingBox_.left = child->position.x;
            boundingBox_.top = child->position.y;
            boundingBox_.width = 0.f;
            boundingBox_.height = 0.f;
        } else {
            if (child->position.x < boundingBox_.left) {
                boundingBox_.width += boundingBox_.left - child->position.x;
                boundingBox_.left = child->position.x;
            } else if (child->position.x > boundingBox_.left + boundingBox_.width) {
                boundingBox_.width = child->position.x - boundingBox_.left;
            }
            if (child->position.y < boundingBox_.top) {
                boundingBox_.height += boundingBox_.top - child->position.y;
                boundingBox_.top = child->position.y;
            } else if (child->position.y > boundingBox_.top + boundingBox_.height) {
                boundingBox_.height = child->position.y - boundingBox_.top;
            }
        }
    }
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
    
    // Update center based on window size (center of the window)
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
    boundingBox_.left = 0.f;
    boundingBox_.top = 0.f;
    boundingBox_.width = 0.f;
    boundingBox_.height = 0.f;
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
            reset();
            std::vector<long long> testSequence = {1, 2, 4, 8, 16, 5, 10, 3, 6, 12, 24};
            buildTree(testSequence);
        }
    }
}

void CollatzTreeRenderer::adjustWindowSize(sf::RenderWindow& window) {
    if (boundingBox_.width == 0.f || boundingBox_.height == 0.f) {
        return;
    }
    
    // Add padding to the bounding box
    float padding = 50.f;
    float width = boundingBox_.width + 2 * padding;
    float height = boundingBox_.height + 2 * padding;
    
    // Resize the window
    window.setSize(sf::Vector2u(static_cast<unsigned int>(width), static_cast<unsigned int>(height)));
    
    // Update the center to match the new window size
    center_ = sf::Vector2f(width / 2.f, height / 2.f);
}

std::string CollatzTreeRenderer::getName() const {
    return "Collatz Tree Renderer";
}