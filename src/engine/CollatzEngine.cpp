#include "CollatzEngine.hpp"
#include <stdexcept>
#include <algorithm>

Sequence CollatzEngine::compute(int64_t n)
{
    if (n < 1)
        throw std::invalid_argument("n must be >= 1");

    Sequence seq;
    seq.reserve(300); // most of the sequences have less than 300 steps
    seq.push_back(n);

    while (n != 1)
    {
        n = (n % 2 == 0) ? n / 2 : 3 * n + 1;
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
    return s.size() - 1; // il primo elemento è n, l'ultimo è 1
}
