#include "FeatherRenderer.hpp"
#include <iostream>
#include <numbers>
#include <cmath>
#include <algorithm>

// ─── build / config ──────────────────────────────────────────
void FeatherRenderer::build(const CollatzCollection& col,
                            const RenderConfig& cfg,
                            sf::Vector2u winSize)
{
    collection_ = col;
    config_ = cfg;
    std::cout << "FeatherRenderer::build: window=" << winSize.x << "x" << winSize.y << "\n";
    std::cout.flush();
    // Pass the window centre: FeatherTree will recentre the bounding
    // box of the whole feather onto this point after computing geometry.
    tree_.build(col, cfg, windowCentre(winSize));
    bfsOrder_  = tree_.getBFSOrder();
    maxWeight_ = tree_.maxWeight();
    shapes_.clear();
    cursor_ = 0;
    
    // Build sequential order if needed
    if (animationMode_ == Command::AnimationMode::Sequential) {
        buildSequentialOrder();
    }
    
    // Reserve space for shapes (use appropriate order size)
    const std::vector<int>& order = (animationMode_ == Command::AnimationMode::Sequential) ? sequentialOrder_ : bfsOrder_;
    shapes_.reserve(order.size());
    
    // Set up view to ensure tree fits in window
    setViewToFitTree(winSize);
}

void FeatherRenderer::applyConfig(const RenderConfig& cfg, sf::Vector2u winSize)
{
    config_ = cfg;
    std::cout << "FeatherRenderer::applyConfig: window=" << winSize.x << "x" << winSize.y << "\n";
    std::cout.flush();
    tree_.recomputeGeometry(cfg, windowCentre(winSize));
    bfsOrder_  = tree_.getBFSOrder();
    maxWeight_ = tree_.maxWeight();
    shapes_.clear();
    cursor_ = 0;
    
    // Rebuild sequential order if in sequential mode
    if (animationMode_ == Command::AnimationMode::Sequential) {
        buildSequentialOrder();
    }
    
    // Reserve space for shapes (use appropriate order size)
    const std::vector<int>& order = (animationMode_ == Command::AnimationMode::Sequential) ? sequentialOrder_ : bfsOrder_;
    shapes_.reserve(order.size());
    
    // Update view to fit the recomputed tree
    setViewToFitTree(winSize);
}

void FeatherRenderer::recolor(const RenderConfig& cfg)
{
    config_ = cfg;
    std::cout << "FeatherRenderer::recolor: keeping " << cursor_ << " shapes\n";
    std::cout.flush();
    std::size_t prev = cursor_;
    shapes_.clear();
    
    // Use the appropriate order based on animation mode
    const std::vector<int>& order = (animationMode_ == Command::AnimationMode::Sequential) ? sequentialOrder_ : bfsOrder_;
    for (std::size_t i = 0; i < prev && i < order.size(); ++i) {
        addShape(i, order);
    }
    // cursor_ remains unchanged, preserving animation progress
}

// ─── animation ───────────────────────────────────────────────
void FeatherRenderer::update(int steps)
{
    const std::vector<int>& order = (animationMode_ == Command::AnimationMode::Sequential) ? sequentialOrder_ : bfsOrder_;
    for (int i = 0; i < steps && cursor_ < order.size(); ++i, ++cursor_)
        addShape(cursor_, order);
}

void FeatherRenderer::addShape(std::size_t orderIndex, const std::vector<int>& order)
{
    const auto& nodes  = tree_.nodes();
    
    // Bounds checking for order index
    if (orderIndex >= order.size()) {
        return;
    }
    
    int idx = order[orderIndex];
    
    // Bounds checking for node index
    if (idx < 0 || static_cast<std::size_t>(idx) >= nodes.size()) {
        return;
    }
    
    const auto& node = nodes[idx];

    // Skip root node (no parent to draw from)
    if (node.parentIdx == -1) return;

    // Bounds checking for parent index
    if (node.parentIdx < 0 || static_cast<std::size_t>(node.parentIdx) >= nodes.size()) {
        return;
    }

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
    // Save the window's current view and set our custom view
    sf::View oldView = window.getView();
    window.setView(view_);
    
    for (const auto& shape : shapes_) window.draw(shape);
    
    // Restore the window's view
    window.setView(oldView);
}

void FeatherRenderer::reset()  { shapes_.clear(); cursor_ = 0; }
bool FeatherRenderer::isDone() const { 
    const std::vector<int>& order = (animationMode_ == Command::AnimationMode::Sequential) ? sequentialOrder_ : bfsOrder_;
    return cursor_ >= order.size(); 
}

// ─── helpers ──────────────────────────────────────────────────
sf::Color FeatherRenderer::nodeColor(const FeatherNode& node) const
{
    switch (config_.colorMode) {
        case Command::ColorMode::Fixed:
            return config_.fixedColor;
        case Command::ColorMode::PerParity:
            return (node.value % 2 == 0)
                ? sf::Color{ 100, 149, 237, 255 }  // blue  — even
                : sf::Color{ 255, 140,   0, 255 };  // orange — odd
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

// ─────────────────────────────────────────────────────────────
// Maximum size for sequential order to prevent memory exhaustion
constexpr std::size_t MAX_SEQUENTIAL_ORDER = 500000;

void FeatherRenderer::buildSequentialOrder()
{
    // Build a sequential animation order: complete sequences from highest to lowest starting number
    sequentialOrder_.clear();
    
    // Collect all starting numbers from the collection, sorted in descending order
    std::vector<int64_t> startingNumbers;
    for (const auto& [startVal, seq] : collection_) {
        startingNumbers.push_back(startVal);
    }
    
    // Sort in descending order (highest first)
    std::sort(startingNumbers.begin(), startingNumbers.end(), std::greater<>());
    
    // For each starting number, find all nodes that belong to its sequence
    // and add them to the order in root-to-leaf order (so segments connect properly)
    const auto& nodes = tree_.nodes();
    
    for (int64_t startVal : startingNumbers) {
        // Safety check: prevent unbounded growth
        if (sequentialOrder_.size() >= MAX_SEQUENTIAL_ORDER) {
            std::cerr << "Warning: Sequential order reached maximum size (" 
                      << MAX_SEQUENTIAL_ORDER << "). Truncating.\n";
            break;
        }
        
        // Get the leaf node for this starting value directly from the tree
        int leafIdx = tree_.getLeafNodeForStartValue(startVal);
        
        if (leafIdx == -1) continue;  // Shouldn't happen - all startVals should have a leaf
        
        // Walk up the tree from leaf to root, collecting node indices
        std::vector<int> sequencePath;
        int temp = leafIdx;
        while (temp != -1) {
            sequencePath.push_back(temp);
            temp = nodes[temp].parentIdx;
        }
        
        // Reverse to get root-to-leaf order
        // This ensures that when we animate, parent nodes are added before children
        std::reverse(sequencePath.begin(), sequencePath.end());
        
        // Add nodes in root-to-leaf order, but skip the root node
        // (the root has no parent, so there's no line to draw for it)
        // This animates each sequence from 1 up to the starting number completely
        // (which visually appears as each sequence growing from the center)
        // Note: We do NOT deduplicate here - each sequence should have its full path
        // drawn, even if nodes are shared with other sequences
        for (int idx : sequencePath) {
            if (idx != 0) {  // Skip root node (index 0)
                sequentialOrder_.push_back(idx);
                
                // Safety check during building
                if (sequentialOrder_.size() >= MAX_SEQUENTIAL_ORDER) {
                    std::cerr << "Warning: Sequential order reached maximum size (" 
                              << MAX_SEQUENTIAL_ORDER << "). Truncating.\n";
                    break;
                }
            }
        }
        
        if (sequentialOrder_.size() >= MAX_SEQUENTIAL_ORDER) {
            break;
        }
    }
    
    std::cout << "Sequential order built with " << sequentialOrder_.size() << " nodes\n";
    std::cout.flush();
}

// ─────────────────────────────────────────────────────────────
void FeatherRenderer::setAnimationMode(Command::AnimationMode mode, const CollatzCollection& collection)
{
    if (animationMode_ == mode) return;
    
    animationMode_ = mode;
    collection_ = collection;
    
    if (mode == Command::AnimationMode::Sequential) {
        buildSequentialOrder();
    }
    
    // Reset animation to start fresh with new mode
    shapes_.clear();
    cursor_ = 0;
    
    std::cout << "Animation mode set to: " 
              << (mode == Command::AnimationMode::Parallel ? "Parallel" : "Sequential")
              << "\n";
    std::cout.flush();
}

// ─────────────────────────────────────────────────────────────
sf::FloatRect FeatherRenderer::computeTreeBounds() const
{
    const auto& nodes = tree_.nodes();
    if (nodes.empty()) return sf::FloatRect();
    
    float minX = nodes[0].pos.x;
    float maxX = nodes[0].pos.x;
    float minY = nodes[0].pos.y;
    float maxY = nodes[0].pos.y;
    
    for (const auto& node : nodes) {
        if (node.pos.x < minX) minX = node.pos.x;
        if (node.pos.x > maxX) maxX = node.pos.x;
        if (node.pos.y < minY) minY = node.pos.y;
        if (node.pos.y > maxY) maxY = node.pos.y;
    }
    
    return sf::FloatRect({minX, minY}, {maxX - minX, maxY - minY});
}

void FeatherRenderer::setViewToFitTree(sf::Vector2u windowSize)
{
    sf::FloatRect treeBounds = computeTreeBounds();
    
    std::cout << "Tree bounds: (" << treeBounds.position.x << ", " << treeBounds.position.y 
              << ") size=(" << treeBounds.size.x << ", " << treeBounds.size.y << ")\n";
    std::cout.flush();
    
    if (treeBounds.size.x <= 0 || treeBounds.size.y <= 0) {
        // Empty tree or single point, use default view
        std::cout << "  Empty tree bounds, using default view\n";
        view_ = sf::View(sf::FloatRect({0, 0}, {static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)}));
        return;
    }
    
    // Add margin around the tree (10% on each side = 20% total)
    const float marginFactor = 1.2f;  // 10% margin on each side
    float width = treeBounds.size.x * marginFactor;
    float height = treeBounds.size.y * marginFactor;
    
    // Center of the tree
    sf::Vector2f center(
        treeBounds.position.x + treeBounds.size.x * 0.5f,
        treeBounds.position.y + treeBounds.size.y * 0.5f
    );
    
    // Create view that always shows the entire tree with margin
    // SFML will automatically scale this to fit the window
    view_ = sf::View(center, sf::Vector2f(width, height));
    
    std::cout << "  View: center=(" << view_.getCenter().x << ", " << view_.getCenter().y
              << ") size=(" << view_.getSize().x << ", " << view_.getSize().y << ")\n";
    std::cout.flush();
}

// ─────────────────────────────────────────────────────────────

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


