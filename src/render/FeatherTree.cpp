#include "FeatherTree.hpp"
#include <numbers>
#include <cmath>
#include <stack>
#include <queue>

void FeatherTree::build(const CollatzCollection& collection,
                        const RenderConfig& cfg,
                        sf::Vector2f origin)
{
    nodes_.clear();
    lookup_.clear();

    // nodo radice: valore 1, nessun genitore
    nodes_.push_back(FeatherNode{ 1, -1, 0, 0.f, 0, {}, {}, 0 });

    for (const auto& [startVal, seq] : collection)
        insertSequence(seq, startVal);

    computeGeometry(cfg, origin);
}

void FeatherTree::recomputeGeometry(const RenderConfig& cfg, sf::Vector2f origin)
{
    computeGeometry(cfg, origin);
}

// ─────────────────────────────────────────────────────────────
void FeatherTree::insertSequence(const Sequence& seq, int64_t startVal)
{
    if (seq.empty()) return;

    int currentIdx = 0;   // radice
    nodes_[0].weight++;

    // itero la sequenza al contrario, saltando il primo elemento (1 = radice)
    for (auto it = seq.rbegin() + 1; it != seq.rend(); ++it) {
        int64_t val = *it;
        auto    key = std::make_pair(currentIdx, val);
        auto    found = lookup_.find(key);

        if (found == lookup_.end()) {
            int newIdx = static_cast<int>(nodes_.size());
            nodes_.push_back({ val, currentIdx, 1, 0.f, 0,
                               {0.f, 0.f}, {}, startVal });
            nodes_[currentIdx].childIndices.push_back(newIdx);
            lookup_[key] = newIdx;
            currentIdx = newIdx;
        } else {
            nodes_[found->second].weight++;
            currentIdx = found->second;
        }
    }
}

// ─────────────────────────────────────────────────────────────
void FeatherTree::computeGeometry(const RenderConfig& cfg, sf::Vector2f origin)
{
    const float pi       = std::numbers::pi_v<float>;
    const float thetaRad = cfg.angle * pi / 180.f;
    const float initAngle = -pi / 2.f;   // punta verso l'alto sullo schermo

    nodes_[0].pos   = origin;
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

            // pari → destra (+θ sullo schermo, y cresce verso il basso)
            // dispari → sinistra (-θ)
            float delta = (child.value % 2 == 0) ? +thetaRad : -thetaRad;
            child.angle = parent.angle + delta;

            float len = (cfg.segmentMode == Command::SegmentMode::Constant)
                ? cfg.segmentLen
                : cfg.segmentLen * std::pow(cfg.decay,
                                            static_cast<float>(child.depth - 1));

            child.pos = parent.pos + sf::Vector2f{
                std::cos(child.angle) * len,
                std::sin(child.angle) * len
            };

            stack.push(childIdx);
        }
    }
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

int FeatherTree::maxWeight() const
{
    int maxW = 0;
    for (const auto& n : nodes_) maxW = std::max(maxW, n.weight);
    return maxW;
}