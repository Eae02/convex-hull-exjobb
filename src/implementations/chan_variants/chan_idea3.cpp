#include "common.hpp"
#include "../../hull_impl.hpp"
#include "../../point.hpp"

#include <vector>
#include <math.h>
#include <span>
#include <cassert>


// This file implements Chans algorithm with idea 3 presented in his paper. It is very similar to MergeHull

template <typename T>
static void runChanId3(std::vector<point<T>>& pts1, size_t exponent, bool use_idea_2 = false) {
	if (pts1.size() <= 1) return;
    long long t = 3;
    long long m = 1LL << (1LL << t); // Start with sets of size 2^8
    long long current_set_size = m;

    long long numsets = (pts1.size() + current_set_size - 1)/current_set_size; // Number of partitions, ceil(n/m)
    std::vector<std::span<point<T>>> spans;
    for (long long i = 0; i < numsets; i++) {
        long long start = i*current_set_size;
        long long end = std::min((i+1)*current_set_size, (long long) pts1.size());
        std::span<point<T>> current_span = std::span<point<T>>(pts1.begin()+start,pts1.begin()+end);
        long long newsize = monotone_chain(current_span);
        current_span = current_span.subspan(0,newsize);
        spans.push_back(current_span);
    }
    std::vector<point<T>> pts2; 
    pts2.reserve(pts1.size()); // Assert that pts2 never has to move the data. This would cause the spans to be invalid.

    std::vector<point<T>> *a = &pts1, *b = &pts2; // a always points to vector currently containing the points.
    bool pts1_has_spans = true;
    while (spans.size() > 1) {
        t++;
        m = calcM(t);
        while (current_set_size < m && spans.size() > 1) {
            pairwiseMerge(spans, exponent, *b); // Move spans from a to b
            a->clear();
            std::swap(a,b);
            pts1_has_spans = !pts1_has_spans;
            current_set_size *= exponent;
        }
        if (spans.size() == 1) break; // Early break to avoid unnecessary computations

        long long H = calcH(t, use_idea_2); // 2^2^t or 2^(2^t-t)
        if (Merge2DHulls(spans, *b, H) > -1) {
            pts1_has_spans = !pts1_has_spans;
            if (!pts1_has_spans) {
                pts1 = pts2;
            }
            return;
        }
        b->clear(); // Clear b again because the merge failed.
    }
    if (!pts1_has_spans) {
        pts1 = pts2;
    }
    pts1.resize(spans[0].size());
    return;
}

DEF_HULL_IMPL({
	.name = "chan_widea3", // Something like O(n loglog m) expected time on randomized inputs.
	.runInt = std::bind(runChanId3<int64_t>, std::placeholders::_1, 3, false),
	.runDouble = std::bind(runChanId3<double>, std::placeholders::_1, 3, false),
});

DEF_HULL_IMPL({
	.name = "chan_refined", // Has O(n) expected time complexity on randomized inputs.
	.runInt = std::bind(runChanId3<int64_t>, std::placeholders::_1, 3, true),
	.runDouble = std::bind(runChanId3<double>, std::placeholders::_1, 3, true),
});