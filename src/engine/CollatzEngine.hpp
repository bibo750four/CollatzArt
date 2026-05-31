#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>

// Type aliases
using Sequence           = std::vector<int64_t>;
using CollatzCollection  = std::unordered_map<int64_t, Sequence>;

class CollatzEngine
{
public:
    // Calculates the Collatz sequence for a single starting integer
    static Sequence     compute(int64_t n);

    // Generates the collection for all starting numbers:
    //   start=1, 1+step, 1+2*step, ..., for `range` elements
    static CollatzCollection generate(int64_t range, int64_t step);

    // Statistical utilities (useful for the renderer)
    static int64_t maxValue(const Sequence& s);
    static std::size_t stoppingTime(const Sequence& s); // steps to reach 1
};
