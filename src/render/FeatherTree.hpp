#pragma once
#include "RenderConfig.hpp"
#include "engine/CollatzEngine.hpp"
#include <SFML/System/Vector2.hpp>
#include <vector>
#include <unordered_map>
#include <utility>
#include <cstdint>

struct FeatherNode {
    int64_t             value      { 0 };
    int                 parentIdx  { -1 };
    int                 weight     { 1 };   // sequenze che passano per questo nodo
    float               angle      { 0.f }; // radianti, coordinate schermo
    int                 depth      { 0 };
    sf::Vector2f        pos        { 0.f, 0.f };
    std::vector<int>    childIndices;
    int64_t             colorHint  { 0 };   // valore di partenza della prima sequenza
};

class FeatherTree {
public:
    void build(const CollatzCollection& collection,
               const RenderConfig& cfg,
               sf::Vector2f origin);

    void recomputeGeometry(const RenderConfig& cfg, sf::Vector2f origin);

    const std::vector<FeatherNode>& nodes() const { return nodes_; }
    std::vector<int>                getBFSOrder()  const;
    int                             maxWeight()    const;

private:
    struct PairHash {
        std::size_t operator()(const std::pair<int, int64_t>& p) const noexcept {
            auto h1 = std::hash<int>{}(p.first);
            auto h2 = std::hash<int64_t>{}(p.second);
            return h1 ^ (h2 * 0x9e3779b97f4a7c15ULL
                         + 0x6c62272e07bb0142ULL
                         + (h1 << 6) + (h1 >> 2));
        }
    };

    void insertSequence(const Sequence& seq, int64_t startVal);
    void computeGeometry(const RenderConfig& cfg, sf::Vector2f origin);

    std::vector<FeatherNode>                                   nodes_;
    std::unordered_map<std::pair<int,int64_t>, int, PairHash>  lookup_;
};
