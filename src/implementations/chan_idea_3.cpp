#include "common.hpp"
#include "impl1.hpp"
#include "../hull_impl.hpp"
#include "../point.hpp"

#include <algorithm>
#include <vector>
#include <math.h>


// Variant of chans algorithm using Refinement idea 3 from the paper.
template <typename T>
void runChanId3(std::vector<point<T>>& pts) {
	if (pts.size() <= 2) return;
    // Starting point makes a significant difference, this is faster across the board (input sice from 100 to 1M) compared to t = 1 or 2.
    long long t = 4; 
    int m = std::min((long long) pts.size(), (1LL << 2*t));
    if (m < 1) { // Catch overflow
        m = pts.size();
    }
    int p = (pts.size() + m - 1)/m; // Number of partitions, ceil(n/m)
    std::vector<std::vector<point<T>>> Pdiv(p); // Division of the points into p sets
    for (int i = 0; i < pts.size(); i++) {
        Pdiv[i%p].push_back(pts[i]);
    }
    for (int pi = 0; pi < p; pi++) {
        runImpl1(Pdiv[pi]);
    }
    while (true) {
        t++;
        m = std::min((long long) pts.size(), (1LL << 2*t));
        if (m < 1) { // Catch overflow
            m = pts.size();
        }
        // Merge previous convex hulls to get bigger ones
        std::vector<std::vector<point<T>>> newPdiv;
        std::vector<std::vector<point<T>>> tempDiv;
        int tempSize = 0;
        for (auto& hull : Pdiv) {
            tempSize += hull.size();
            if (tempSize > m) {
                newPdiv.push_back(Hull2DMerge(tempDiv));
                tempSize = hull.size();
                tempDiv.clear();
            }
            tempDiv.push_back(std::move(hull));
        }
        newPdiv.push_back(Hull2DMerge(tempDiv));
        Pdiv = std::move(newPdiv);
        if (Pdiv.size() == 1) {
            pts = std::move(Pdiv[0]);
            return;
        }
    }
}

DEF_HULL_IMPL({
	.name = "chan_id3",
	.runInt = &runChanId3<int64_t>,
	.runDouble = &runChanId3<double>,
});
