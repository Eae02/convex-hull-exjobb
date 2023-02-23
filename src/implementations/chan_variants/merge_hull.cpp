#include "common.hpp"
#include "../../hull_impl.hpp"
#include "../../point.hpp"

#include <vector>
#include <math.h>
#include <span>

// This file implements the MergeHull algorithm which is our name for the divide and conquer algorithm by Bentley and Shamos from 1978. 
// It has a worst case running time of O(n log n). But if the input is randomized in such a way that the expected number of points on the convex hull
// is O(n^p) for p < 1, it runs in expected time O(n).

template <typename T>
static void runMergeHull(std::vector<point<T>>& pts, size_t exponent) {
	if (pts.size() <= 1) return;
    long long startsize = 1 << 8; // Start with sets of size 2^8

    long long numsets = (pts.size() + startsize - 1)/startsize; // Number of partitions, ceil(n/m)
    std::vector<std::span<point<T>>> spans;
    for (long long i = 0; i < numsets; i++) {
        long long start = i*startsize;
        long long end = std::min((i+1)*startsize, (long long) pts.size());
        std::span<point<T>> current_span = std::span<point<T>>(pts.begin()+start,pts.begin()+end);
        long long newsize = monotone_chain(current_span);
        current_span = current_span.subspan(0,newsize);
        spans.push_back(current_span);
    }
    std::vector<point<T>> temp; // Hulls are temporarily written here when computed.
    while (spans.size() > 1) {
        pairwiseMerge(spans, exponent, temp, true);
    }
    pts.resize(spans[0].size());
    return;
}

DEF_HULL_IMPL({
	.name = "merge_hull",
	.runInt = std::bind(runMergeHull<int64_t>, std::placeholders::_1, 3),
	.runDouble = std::bind(runMergeHull<double>, std::placeholders::_1, 3),
});
