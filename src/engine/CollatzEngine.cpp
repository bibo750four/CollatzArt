#include "CollatzEngine.hpp"
#include <stdexcept>
#include <algorithm>
#include <climits>

/**
 * Computes the Collatz sequence for a given starting number.
 * 
 * The Collatz sequence is generated as follows:
 * - If n is even: n → n/2
 * - If n is odd: n → 3*n + 1
 * 
 * The sequence terminates when n reaches 1.
 * 
 * @param n The starting number (must be >= 1).
 * @return The computed Collatz sequence.
 * @throws std::invalid_argument If n < 1.
 * @throws std::overflow_error If 3*n + 1 exceeds INT64_MAX (overflow check).
 */
Sequence CollatzEngine::compute(int64_t n)
{
    if (n < 1)
        throw std::invalid_argument("n must be >= 1");

    Sequence seq;
    seq.reserve(500); // Increased from 300 to handle longer sequences safely
    seq.push_back(n);

    while (n != 1)
    {
        if (n % 2 == 0) {
            n = n / 2;
        } else {
            // Check for overflow: 3*n + 1 > INT64_MAX
            // Rearranged to avoid overflow: n > (INT64_MAX - 1) / 3
            if (n > (INT64_MAX - 1) / 3) {
                throw std::overflow_error("Collatz sequence overflow: 3*n+1 exceeds int64_t maximum");
            }
            n = 3 * n + 1;
        }
        seq.push_back(n);
    }
    return seq;
}

CollatzCollection CollatzEngine::generate(int64_t range, int64_t step)
{
    CollatzCollection collection;
    collection.reserve(range);

    for (int64_t i = 0; i < range; ++i)
    {
        int64_t start = (int64_t)1 + static_cast<int64_t>(i) * step;
        collection.emplace(start, compute(start));
    }
    return collection;
}

int64_t CollatzEngine::maxValue(const Sequence& s)
{
    return *std::max_element(s.cbegin(), s.cend());
}

std::size_t CollatzEngine::stoppingTime(const Sequence& s)
{
    return s.size() - 1; // first element is n, last is 1
}
