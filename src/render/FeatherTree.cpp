#include "FeatherTree.hpp"
#include "engine/CollatzEngine.hpp"
#include <numbers>
#include <cmath>
#include <stack>
#include <queue>
#include <limits>
#include <iostream>

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
    std::cout << "FeatherTree::build called with " << collection.size() << " sequences.\n";
    std::cout.flush();

    for (const auto& [startVal, seq] : collection)
        insertSequence(seq, startVal);

    std::cout << "FeatherTree::build: " << nodes_.size() << " nodes created.\n";
    std::cout.flush();

    computeGeometry(cfg, origin);
}

void FeatherTree::recomputeGeometry(const RenderConfig& cfg, sf::Vector2f origin)
{
    computeGeometry(cfg, origin);
}

// ─────────────────────────────────────────────────────────────
// Geometry uses logarithmic axes:
//
//   radial distance  r(depth) = segmentLen * log2(depth + 2)
//       — compresses deep branches so they remain visible
//
//   angular step uses cfg.evenAngle for even-valued nodes and
//   cfg.oddAngle for odd-valued nodes (+/- angles applied accordingly)
//
// After all positions are computed the whole tree is translated so
// that its bounding box is centred on the window.
// ─────────────────────────────────────────────────────────────
void FeatherTree::computeGeometry(const RenderConfig& cfg, sf::Vector2f origin)
{
    std::cout << "computeGeometry called. origin: [" << origin.x << ", " << origin.y << "] (nodes: " << nodes_.size() << ")\n";
    std::cout.flush();

    const float pi          = std::numbers::pi_v<float>;
    const float evenThetaRad = cfg.evenAngle * pi / 180.f;
    const float oddThetaRad  = cfg.oddAngle  * pi / 180.f;
    const float initAngle   = -pi / 2.f;   // points upward on screen (y grows downward)

    nodes_[0].pos   = { 0.f, 0.f };  // temporarily at origin; will be recentred later
    nodes_[0].angle = initAngle;
    nodes_[0].depth = 0;

    std::stack<int> stack;
    stack.push(0);

    while (!stack.empty()) {
        int parentIdx = stack.top();
        stack.pop();

        const auto& parent = nodes_[parentIdx];

        for (int childIdx : parent.childIndices) {
            auto& child = nodes_[childIdx];
            child.depth = parent.depth + 1;

            // Even value → branch right (+cfg.evenAngle in screen space, y grows downward)
            // Odd value  → branch left  (-cfg.oddAngle)
            float delta = (child.value % 2 == 0) ? +evenThetaRad : -oddThetaRad;
            child.angle = parent.angle + delta;

            // ── Logarithmic radial distance ──────────────────────────────
            // Base segment length scales with log2(depth+2) so that the
            // first few generations are spread out and deeper levels are
            // progressively compressed, keeping the whole feather visible.
            float logScale = std::log2(static_cast<float>(child.depth) + 2.f);

            float len;
            if (cfg.segmentMode == Command::SegmentMode::Constant) {
                len = cfg.segmentLen * logScale;
            } else {
                // Decreasing mode: apply both the exponential decay AND the
                // logarithmic scale so the two effects compound gracefully.
                len = cfg.segmentLen
                      * std::pow(cfg.decay, static_cast<float>(child.depth - 1))
                      * logScale;
            }

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
    std::cout << "Bounding box: minX=" << minX << ", maxX=" << maxX << " (width=" << bbWidth << "), minY=" << minY << ", maxY=" << maxY << " (height=" << bbHeight << ")\n";
    std::cout.flush();

    // Centre of the bounding box
    float bbCentreX = (minX + maxX) * 0.5f;
    float bbCentreY = (minY + maxY) * 0.5f;

    // Translate every node so that the bounding-box centre maps to `origin`
    // (which is the window centre passed in by the caller)
    sf::Vector2f offset{ origin.x - bbCentreX, origin.y - bbCentreY };
    for (auto& n : nodes_)
        n.pos += offset;

    std::cout << "Offset applied for centering: [" << offset.x << ", " << offset.y << "]\n";
    std::cout.flush();
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
