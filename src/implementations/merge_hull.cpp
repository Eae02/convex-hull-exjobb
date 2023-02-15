#include "common.hpp"
#include "impl1.hpp"
#include "../hull_impl.hpp"
#include "../point.hpp"

#include <algorithm>
#include <vector>
#include <math.h>
#include <span>


// This file implements the MergeHull algorithm which is our name for the divide and conquer algorithm by Bentley and Shamos from 1978. 
// It has a worst case running time of O(n log n). But if the input is randomized in such a way that the expected number of points on the convex hull
// is O(n^p) for p < 1, it runs in expected time O(n).


// Adaption of Impl1 to work with span and return new length. Runs in O(n logn) time
template <typename T>
size_t bruteForce(std::span<point<T>>& pts) {
	if (pts.size() <= 1) return pts.size();
	std::sort(pts.begin(), pts.end());
	std::vector<point<T>> h;
	auto f = [&] (size_t s) {
		for (auto p : pts) {
			while (h.size() >= s + 2 && h.back().cross(p, h[h.size() - 2]) <= 0)
				h.pop_back();
			h.push_back(p);
		}
		h.pop_back();
	};
	f(0);
	reverse(pts.begin(), pts.end());
	f(h.size());
	if (h.size() == 2 && h[0] == h[1]) h.pop_back();
    for (size_t i = 0; i < h.size(); i++) {
        pts[i] = h[i];
    }
    return h.size();
}

// Merge k convex hulls given by spans, aborting if resulting hull has more than H points. Returns -1 if it fails, otherwise the size. If H is negative it will never fail.
// Time complexity O(n+k*h) where n is the total number of input points, and h is the number of output points (or H on failure).
// On success it places the resulting hull in spans[0][0:size]. Hulls are assumed to be given in CCW order with first point being the leftmost point (lowest in case of ties).
template <typename T>
int Hull2DMerge(std::vector<std::span<point<T>>>& spans, int H) {
    size_t p = spans.size();
    std::vector<int> indices(p,0);
    std::vector<point<T>> result;

    // Put leftmost point into convex hull
    point<T> leftMostPoint = spans[0][indices[0]];
    for (size_t pi = 1; pi < p; pi++) {
        if (spans[pi][indices[pi]] < leftMostPoint) {
            leftMostPoint = spans[pi][indices[pi]];
        }
    }
    result.push_back(leftMostPoint);

    for (int i = 0; i != H; i++) {
        // For each hull find tangent from result.back()
        point<T> prevHullPoint = result.back();
        for (size_t pi = 0; pi < p; pi++) {
            while(true) {
                point<T> cur = spans[pi][indices[pi]];
                point<T> next = spans[pi][(indices[pi] + 1) % spans[pi].size()];
                side orientation = cur.sideOfLine(next, prevHullPoint);
                if (orientation == side::right || (orientation == side::on && (cur - prevHullPoint).lenmh() < (next - prevHullPoint).lenmh())) {
                    indices[pi]++;
                    indices[pi] %= spans[pi].size();
                } else {
                    break;
                }
            }
        }
        // Pick next hull point from best of all tangents
        point<T> nextHullPoint = spans[0][indices[0]];
        for (size_t pi = 1; pi < p; pi++) {
            point<T> candidate = spans[pi][indices[pi]];
            side orientation = nextHullPoint.sideOfLine(candidate, prevHullPoint);
            if (orientation == side::right || (orientation == side::on && (nextHullPoint - prevHullPoint).lenmh() < (candidate - prevHullPoint).lenmh())) {
                nextHullPoint = candidate;
            }
        }
        result.push_back(nextHullPoint);
        if (nextHullPoint == result[0]) {
            result.pop_back();
            // Place result in the first span.
            for (size_t oi = 0; oi < result.size(); oi++) {
                spans[0][oi] = result[oi];
            }
            return result.size();
        }
    }
    return -1;
}

template <typename T>
void pairwiseMerge(std::vector<std::span<point<T>>>& spans, size_t exponent) {
    std::vector<std::span<point<T>>> output;
    std::vector<std::span<point<T>>> temp;
    for (size_t i = 0; i < spans.size(); i++) {
        temp.push_back(spans[i]);
        if (temp.size() == exponent) {
            int size = Hull2DMerge(temp, -1);
            output.push_back(temp[0].subspan(0,size));
            temp.clear();
        }
    }
    if (temp.size() >= 1) {
        int size = Hull2DMerge(temp, -1);
        output.push_back(temp[0].subspan(0,size));
    }
    spans = std::move(output);
}


template <typename T>
static void runMergeHull(std::vector<point<T>>& pts, size_t exponent) {
	if (pts.size() <= 1) return;
    int startsize = 1 << 8; // Start with sets of size 2^8

    int numsets = (pts.size() + startsize - 1)/startsize; // Number of partitions, ceil(n/m)
    std::vector<std::span<point<T>>> spans;
    for (int i = 0; i < numsets; i++) {
        int start = i*startsize;
        int end = std::min((i+1)*startsize, (int)pts.size());
        std::span<point<T>> my_span = std::span<point<T>>(pts.begin()+start,pts.begin()+end);
        int newsize = bruteForce(my_span);
        my_span = my_span.subspan(0,newsize);
        spans.push_back(my_span);
    }

    while (spans.size() > 1) {
        pairwiseMerge(spans, exponent);
    }
    pts.resize(spans[0].size());
    return;
}

DEF_HULL_IMPL({
	.name = "merge_hull",
	.runInt = std::bind(runMergeHull<int64_t>, std::placeholders::_1, 3),
	.runDouble = std::bind(runMergeHull<double>, std::placeholders::_1, 3),
});
