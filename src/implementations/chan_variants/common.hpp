#pragma once

#include <vector>
#include <span>
#include <algorithm>
#include <cassert>
#include "../../point.hpp"


// Adaption of Impl1 to work with span and return new length. Runs in O(n logn) time
template <typename T>
inline size_t monotone_chain(std::span<point<T>>& pts) {
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
// Appends generated hull to result vector.
// Hulls are assumed to be given in CCW order with first point being the leftmost point (lowest in case of ties).
template <typename T>
inline long long Merge2DHulls(std::vector<std::span<point<T>>>& spans, std::vector<point<T>>& result, long long H = -1, bool in_place = false) {
    size_t p = spans.size();
    std::vector<size_t> indices(p,0);
    long long output_size = 0;
    // Put leftmost point into convex hull
    point<T> leftMostPoint = spans[0][0];
    size_t lastHullUsed = 0;
    for (size_t pi = 1; pi < p; pi++) {
        if (spans[pi][0] < leftMostPoint) {
            leftMostPoint = spans[pi][0];
            lastHullUsed = pi;
        }
    }
    if(in_place) {
        result.clear();
    }
    size_t start_ind = result.size();
    result.push_back(leftMostPoint);
    output_size++;
    indices[lastHullUsed]++;
    for (long long i = 0; i != H; i++) {
        // For each input hull find tangent from result.back()
        point<T> prevHullPoint = result.back();
        for (size_t pi = 0; pi < p; pi++) {
            while(indices[pi] < spans[pi].size()) { // We might need to loop back to index 0 on a hull, but never beyond that
                point<T> cur = spans[pi][indices[pi]];
                point<T> next = spans[pi][(indices[pi] + 1) % spans[pi].size()];
                side orientation = cur.sideOfLine(next, prevHullPoint);
                if (orientation == side::right || (orientation == side::on && (cur - prevHullPoint).lenmh() < (next - prevHullPoint).lenmh())) {
                    indices[pi]++;
                } else {
                    break;
                }
            }
        }
        // Pick next hull point from best of all tangents
        point<T> nextHullPoint;
        lastHullUsed = p;
        for (size_t pi = 0; pi < p; pi++) {
            if(indices[pi] > spans[pi].size()) continue;
            if (lastHullUsed == p) {
                nextHullPoint = spans[pi][indices[pi] % spans[pi].size()];
                lastHullUsed = pi;
                continue;
            }
            point<T> candidate = spans[pi][indices[pi] % spans[pi].size()];
            side orientation = nextHullPoint.sideOfLine(candidate, prevHullPoint);
            if (orientation == side::right || (orientation == side::on && (nextHullPoint - prevHullPoint).lenmh() < (candidate - prevHullPoint).lenmh())) {
                nextHullPoint = candidate;
                lastHullUsed = pi;
            }
        }
        if (nextHullPoint == result[start_ind]) {
            if (in_place) {
                for (long long idx = 0; idx < output_size; idx++) {
                    spans[0][idx] = result[idx];
                }
            }
            return output_size;
        }
        result.push_back(nextHullPoint);
        output_size++;
        indices[lastHullUsed]++;
    }
    return -1;
}



// Takes a set of spans representing hulls. Splits these into sets of size exponent and merges them
// replacing the spans in the spans vector.
// If in_place is true b is just used for temporary storage, and resulting hulls will be written in the location of the spans
// It is important that the spans are consecutive and in order in memory, (possibly with empty gaps).
// If in_place is false, all created hulls are appended to the b vector.
template <typename T>
inline void pairwiseMerge(std::vector<std::span<point<T>>>& spans, size_t exponent, std::vector<point<T>>& b, bool in_place = false) {
    std::vector<std::span<point<T>>> output;
    std::vector<std::span<point<T>>> temp;
    for (size_t i = 0; i < spans.size(); i++) {
        temp.push_back(std::move(spans[i]));
        if (temp.size() == exponent) {
            long long size = Merge2DHulls(temp, b, -1, in_place);
            if (in_place) {
                output.push_back(temp[0].subspan(0,size));
            } else {
                output.push_back(std::span<point<T>>(b.end()-size, b.end()));
            }
            temp.clear();
        }
    }
    if (temp.size() >= 1) {
        long long size = Merge2DHulls(temp, b, -1, in_place);
        if (in_place) {
            output.push_back(temp[0].subspan(0,size));
        } else {
            output.push_back(std::span<point<T>>(b.end()-size, b.end()));
        }
    }
    spans = std::move(output);
}

inline long long calcH(long long t) {
    long long exponent = std::min(40LL, 1LL << t);
    return 1LL << exponent;
}

inline long long calcM(long long t, bool use_idea_2 = false) {
    if (!use_idea_2) {
        return calcH(t);
    }
    long long exponent = std::min(40LL, 1LL << t);
    long long H = calcH(t);
    long long m = H*exponent;
    assert(m > H); // Check for overflows
    return m;
}