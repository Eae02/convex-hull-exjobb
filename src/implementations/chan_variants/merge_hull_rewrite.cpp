#include "common.hpp"
#include "../../hull_impl.hpp"
#include "../../point.hpp"

#include <vector>
#include <math.h>
#include <span>
#include <cassert>


// This file implements the MergeHull algorithm which is our name for the divide and conquer algorithm by Bentley and Shamos from 1978. 
// It has a worst case running time of O(n log n). But if the input is randomized in such a way that the expected number of points on the convex hull
// is O(n^p) for p < 1, it runs in expected time O(n).

template <typename T>
static void runMergeHull(std::vector<point<T>>& pts1, size_t exponent) {
	if (pts1.size() <= 1) return;
    long long startsize = 1 << 8; // Start with sets of size 2^8

    long long numsets = (pts1.size() + startsize - 1)/startsize; // Number of partitions, ceil(n/m)
    std::vector<std::span<point<T>>> spans;
    for (long long i = 0; i < numsets; i++) {
        long long start = i*startsize;
        long long end = std::min((i+1)*startsize, (long long) pts1.size());
        std::span<point<T>> current_span = std::span<point<T>>(pts1.begin()+start,pts1.begin()+end);
        long long newsize = monotone_chain(current_span);
        current_span = current_span.subspan(0,newsize);
        spans.push_back(current_span);
    }
    std::vector<point<T>> pts2; 
    pts2.reserve(pts1.size()); // Assert that pts2 never has to move the data. This would cause the spans to be invalid.
    bool pts1_has_spans = true;
    while (spans.size() > 1) {
        if (pts1_has_spans) {
            pairwiseMerge(spans, exponent, pts2);
            pts1.clear();
        } else {
            pairwiseMerge(spans, exponent, pts1);
            pts2.clear();
        }
        pts1_has_spans = !pts1_has_spans;
    }
    if (pts1_has_spans) {
        pts1.resize(spans[0].size());
    } else {
        pts1 = pts2;
    }
    return;
}

DEF_HULL_IMPL({
	.name = "merge_hull_rewrite",
	.runInt = std::bind(runMergeHull<int64_t>, std::placeholders::_1, 3),
	.runDouble = std::bind(runMergeHull<double>, std::placeholders::_1, 3),
});
