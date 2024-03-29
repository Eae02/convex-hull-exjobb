#include "common.hpp"
#include "../../hull_impl.hpp"
#include "../../point.hpp"

#include <vector>
#include <math.h>
#include <cassert>

template <typename T>
static bool Hull2D(std::vector<point<T>>& pts, long long m, long long H, bool use_idea_1) {
    long long numsets = (pts.size() + m - 1)/m; // Number of partitions, ceil(n/m)

    // Division of the points into sets of size m and run O(nlogn) algorithm on each set.
    std::vector<std::span<point<T>>> spans;
    for (long long i = 0; i < numsets; i++) {
        long long start = i*m;
        long long end = std::min((i+1)*m, (long long) pts.size());
        std::span<point<T>> current_span = std::span<point<T>>(pts.begin()+start,pts.begin()+end);
        long long newsize = monotone_chain(current_span);
        current_span = current_span.subspan(0,newsize); // Truncate span to fit created hull
        spans.push_back(current_span);
    }
    
    std::vector<point<T>> result(H+1);
    long long try_size = Merge2DHulls(spans, result, 0, H);
    if (try_size > -1) {
        pts = result;
        pts.resize(try_size);
        return true;
    }

    if (use_idea_1) {
        // Refinement idea 1 from chans paper, remove known interior points from further consideration.
        std::vector<point<T>> temp;
        for (long long i = 0; i < numsets; i++) {
            std::copy(spans[i].begin(), spans[i].end(), std::back_inserter(temp));
        }
        std::swap(temp,pts);
    }

    return false;
}

template <typename T>
static void runChan(std::vector<point<T>>& pts, bool use_idea_1, bool use_idea_2) {
	if (pts.size() <= 2) return;
    long long t = 1;
    while (true) {
        // Refinement idea 2 from chans paper, put H = m/logm
        long long H = std::min((long long) pts.size(), calcH(t, use_idea_2)); // 2ˆ2ˆt (/2ˆt if use_idea_2) 
        long long m = calcM(t); // 2ˆ2ˆt
        if (Hull2D(pts, m, H, use_idea_1)) {
            return;
        }
        t++;
    }
}

DEF_HULL_IMPL({
	.name = "chan",
	.runInt = std::bind(runChan<int64_t>, std::placeholders::_1, false, false),
	.runDouble = std::bind(runChan<double>, std::placeholders::_1, false, false)
});

DEF_HULL_IMPL({
	.name = "chan_widea1",
	.runInt = std::bind(runChan<int64_t>, std::placeholders::_1, true, false),
	.runDouble = std::bind(runChan<double>, std::placeholders::_1, true, false)
});

DEF_HULL_IMPL({
	.name = "chan_widea2",
	.runInt = std::bind(runChan<int64_t>, std::placeholders::_1, false, true),
	.runDouble = std::bind(runChan<double>, std::placeholders::_1, false, true)
});

DEF_HULL_IMPL({
	.name = "chan_widea12",
	.runInt = std::bind(runChan<int64_t>, std::placeholders::_1, true, true),
	.runDouble = std::bind(runChan<double>, std::placeholders::_1, true, true)
});