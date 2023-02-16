#include "common.hpp"
#include "impl1.hpp"
#include "../hull_impl.hpp"
#include "../point.hpp"

#include <algorithm>
#include <vector>
#include <math.h>

template <typename T>
bool Hull2D(std::vector<point<T>>& pts, int m, int H) {
    int p = (pts.size() + m - 1)/m; // Number of partitions, ceil(n/m)
    std::vector<std::vector<point<T>>> Pdiv(p); // Division of the points into p sets
    for (size_t i = 0; i < pts.size(); i++) {
        Pdiv[i%p].push_back(pts[i]);
    }
    for (int pi = 0; pi < p; pi++) {
        runImpl1(Pdiv[pi]);
    }
    std::vector<point<T>> result;
    if (Hull2DMerge(Pdiv, H, result)) {
        std::swap(result, pts);
        return true;
    }

    // Refinement idea 1 from chans paper, remove known interior points from further consideration.
    pts.clear();
    for (int pi = 0; pi < p; pi++) {
        std::copy(Pdiv[pi].begin(), Pdiv[pi].end(), std::back_inserter(pts));
    }

    return false;
}

template <typename T>
void runChan(std::vector<point<T>>& pts) {
	if (pts.size() <= 2) return;
    long long t = 1;
    while (true) {
        int H = std::min((long long) pts.size(), 1LL << (1LL << t));
        if (H < 1) { // Catch overflow
            H = pts.size();
        }
        // Refinement idea 2 from chans paper, put m = H*logH
        if (Hull2D(pts, H * (1LL << t), H)) {
            return;
        }
        t++;
    }
}

DEF_HULL_IMPL({
	.name = "chan",
	.runInt = &runChan<int64_t>,
	.runDouble = &runChan<double>,
});
