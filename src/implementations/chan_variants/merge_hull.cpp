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
static void runMergeHull(std::vector<point<T>>& pts1, size_t exponent, bool reduce_copy) {
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
    if (reduce_copy) { // Allocate 2 vectors, and alternatively read from one and write to the other. Less copying of data
        std::vector<point<T>> pts2; 
        pts2.reserve(pts1.size()); // Make sure that pts2 never has to reallocate the data. This would cause the spans to become invalid.
        std::vector<point<T>> *a = &pts1, *b = &pts2; // a always points to vector currently containing the points.
        bool pts1_has_spans = true;
        while (spans.size() > 1) {
            pairwiseMerge(spans, exponent, *b); // Spans move from a to b
            a->clear();
            std::swap(a,b); 
            pts1_has_spans = !pts1_has_spans;
        }
        if (!pts1_has_spans) {
            pts1 = pts2;
        }
    } else { // Simple implementation. Computed hulls are temporarily written to temp and then back to pts vector.
        std::vector<point<T>> temp; // Hulls are temporarily written here when computed.
        while (spans.size() > 1) {
            pairwiseMerge(spans, exponent, temp, true);
        }
    }

    pts1.resize(spans[0].size()); // This catches the special case where startsize >= n
    return;

}

DEF_HULL_IMPL({
	.name = "merge_hull",
	.runInt = std::bind(runMergeHull<int64_t>, std::placeholders::_1, 3, false),
	.runDouble = std::bind(runMergeHull<double>, std::placeholders::_1, 3, false),
});


DEF_HULL_IMPL({
	.name = "merge_hull_reduce_copy",
	.runInt = std::bind(runMergeHull<int64_t>, std::placeholders::_1, 3, true),
	.runDouble = std::bind(runMergeHull<double>, std::placeholders::_1, 3, true),
});
