#include "FeatherTree.hpp"
#include "engine/CollatzEngine.hpp"
#include <numbers>
#include <cmath>
#include <stack>
#include <queue>
#include <limits>
#include <iostream>

// Safety limit for maximum number of nodes to prevent memory exhaustion.
// This prevents unbounded memory growth when generating large trees.
constexpr std::size_t MAX_NODES = 1000000;

// Debug output control - set to true to enable debug messages.
// Note: Debug output should be disabled in production builds.
constexpr bool DEBUG_FEATHER_TREE = false;

// Helper macro for debug output - no-op when DEBUG_FEATHER_TREE is false.
// Usage: FEATHER_DEBUG("message" << variable << "\n");
#define FEATHER_DEBUG(msg) do { if (DEBUG_FEATHER_TREE) { std::cout << msg << std::flush; } } while (0)

void FeatherTree::insertSequence(const Sequence& seq, int64_t startVal) {
    if (seq.empty()) return;

    int currentIdx = 0;   // root
    nodes_[0].weight++;

    // Walk the sequence in reverse, skipping the first element (1 = root)
    for (auto it = seq.rbegin() + 1; it != seq.rend(); ++it) {
        int64_t val = *it;
        auto    key = std::make_pair(currentIdx, val);
        auto    found = lookup_.find(key);

        if (found == lookup_.end()) {
            int newIdx = static_cast<int>(nodes_.size());
            nodes_.push_back({ val, currentIdx, 1, 0.f, 0, {0.f, 0.f}, {}, startVal });
            nodes_[currentIdx].childIndices.push_back(newIdx);
            lookup_[key] = newIdx;
            currentIdx = newIdx;
        } else {
            nodes_[found->second].weight++;
            currentIdx = found->second;
        }
    }
    
    // Store the leaf node index for this starting value
    startValToLeafIdx_[startVal] = currentIdx;
}

void FeatherTree::build(const CollatzCollection& collection,
                        const RenderConfig& cfg,
                        sf::Vector2f origin)
{
    nodes_.clear();
    lookup_.clear();
    startValToLeafIdx_.clear();

    // Root node: value 1, no parent
    nodes_.push_back(FeatherNode{ 1, -1, 0, 0.f, 0, {}, {}, 0 });
    FEATHER_DEBUG("FeatherTree::build called with " << collection.size() << " sequences.\n");

    for (const auto& [startVal, seq] : collection) {
        insertSequence(seq, startVal);
        
        // Safety check: prevent unbounded memory growth
        if (nodes_.size() >= MAX_NODES) {
            throw std::runtime_error("Maximum node limit (" + std::to_string(MAX_NODES) + 
                                     ") exceeded. Please reduce range or step.");
        }
    }

    FEATHER_DEBUG("FeatherTree::build: " << nodes_.size() << " nodes created.\n");

    computeGeometry(cfg, origin);
}

void FeatherTree::recomputeGeometry(const RenderConfig& cfg, sf::Vector2f origin)
{
    computeGeometry(cfg, origin);
}

// ─────────────────────────────────────────────────────────────
// Geometry uses logarithmic axes to ensure the tree remains visually balanced:
//
//   - Radial distance (r) is computed as: r(depth) = segmentLen * log2(depth + 2)
//     This compresses deeper branches logarithmically, preventing visual clutter.
//
//   - Angular step: cfg.evenAngle for even-valued nodes (branches right) and
//     cfg.oddAngle for odd-valued nodes (branches left).
//
//   - After computing positions, the tree is translated so its bounding box is
//     centred on the window.
// ─────────────────────────────────────────────────────────────
void FeatherTree::computeGeometry(const RenderConfig& cfg, sf::Vector2f origin)
{
    FEATHER_DEBUG("computeGeometry called. origin: [" << origin.x << ", " << origin.y << "] (nodes: " << nodes_.size() << ")\n");

    const float pi          = std::numbers::pi_v<float>;
    const float evenThetaRad = cfg.evenAngle * pi / 180.f;  // Convert degrees to radians
    const float oddThetaRad  = cfg.oddAngle  * pi / 180.f;  // Convert degrees to radians
    const float initAngle   = -pi / 2.f;   // Initialize angle to point upward (y grows downward in screen space)

    // Initialize root node at origin (temporary position; will be recentred later)
    nodes_[0].pos   = { 0.f, 0.f };
    nodes_[0].angle = initAngle;
    nodes_[0].depth = 0;

    // Use a stack for depth-first traversal of the tree
    std::stack<int> stack;
    stack.push(0);

    while (!stack.empty()) {
        int parentIdx = stack.top();
        stack.pop();

        const auto& parent = nodes_[parentIdx];

        for (int childIdx : parent.childIndices) {
            auto& child = nodes_[childIdx];
            child.depth = parent.depth + 1;

            // Determine branching direction based on node value (even or odd)
            // Even: branch right (+cfg.evenAngle), Odd: branch left (-cfg.oddAngle)
            float delta = (child.value % 2 == 0) ? +evenThetaRad : -oddThetaRad;
            child.angle = parent.angle + delta;

            // ── Logarithmic radial distance ──────────────────────────────
            // The logarithmic scale ensures that deeper branches are compressed,
            // while the first few generations remain spread out for clarity.
            float logScale = std::log2(static_cast<float>(child.depth) + 2.f);

            float len;
            if (cfg.segmentMode == Command::SegmentMode::Constant) {
                // Constant mode: segment length scales only with log2(depth + 2)
                len = cfg.segmentLen * logScale;
            } else {
                // Decreasing mode: segment length decays exponentially with depth,
                // compounded with the logarithmic scale for smoother compression.
                len = cfg.segmentLen
                      * std::pow(cfg.decay, static_cast<float>(child.depth - 1))
                      * logScale;
            }

            // Compute child position using polar coordinates (angle + length)
            child.pos = parent.pos + sf::Vector2f{
                std::cos(child.angle) * len,
                std::sin(child.angle) * len
            };

            stack.push(childIdx);
        }
    }

    // ── Recentre the tree on the window ──────────────────────────────────
    // Compute bounding box of all node positions (in the temporary
    // coordinate system where the root is at {0,0}).
    float minX =  std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float minY =  std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();

    for (const auto& n : nodes_) {
        if (n.pos.x < minX) minX = n.pos.x;
        if (n.pos.x > maxX) maxX = n.pos.x;
        if (n.pos.y < minY) minY = n.pos.y;
        if (n.pos.y > maxY) maxY = n.pos.y;
    }
    float bbWidth = maxX - minX;
    float bbHeight = maxY - minY;
    FEATHER_DEBUG("Bounding box: minX=" << minX << ", maxX=" << maxX << " (width=" << bbWidth << "), minY=" << minY << ", maxY=" << maxY << " (height=" << bbHeight << ")\n");

    // Centre of the bounding box
    float bbCentreX = (minX + maxX) * 0.5f;
    float bbCentreY = (minY + maxY) * 0.5f;

    // Translate every node so that the bounding-box centre maps to `origin`
    // (which is the window centre passed in by the caller)
    sf::Vector2f offset{ origin.x - bbCentreX, origin.y - bbCentreY };
    for (auto& n : nodes_)
        n.pos += offset;

    FEATHER_DEBUG("Offset applied for centering: [" << offset.x << ", " << offset.y << "]\n");
}

// ─────────────────────────────────────────────────────────────
std::vector<int> FeatherTree::getBFSOrder() const
{
    std::vector<int> order;
    order.reserve(nodes_.size());

    std::queue<int> q;
    for (int c : nodes_[0].childIndices) q.push(c);

    while (!q.empty()) {
        int idx = q.front(); q.pop();
        order.push_back(idx);
        for (int c : nodes_[idx].childIndices) q.push(c);
    }
    return order;
}

int FeatherTree::getLeafNodeForStartValue(int64_t startVal) const
{
    auto it = startValToLeafIdx_.find(startVal);
    if (it != startValToLeafIdx_.end()) {
        return it->second;
    }
    return -1;  // Not found
}

int FeatherTree::maxWeight() const
{
    int maxW = 0;
    for (const auto& n : nodes_) maxW = std::max(maxW, n.weight);
    return maxW;
}
