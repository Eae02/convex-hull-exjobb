#pragma once

#include <vector>
#include <climits>
#include <cstddef>
#include "../../point.hpp"


// Merge k convex hulls given by Pdiv, aborting if resulting hull has more than H points.
// Time complexity O(n+k*h) where n is the total number of input points, and h is the number of output points.
template <typename T>
bool Hull2DMerge(const std::vector<std::vector<point<T>>>& Pdiv, int H, std::vector<point<T>>& result) {
    int p = Pdiv.size();
    std::vector<int> indices(p,0);
    for (int pi = 0; pi < p; pi++) {
        for (size_t i = 0; i < Pdiv[pi].size(); i++) {
            if (Pdiv[pi][i] < Pdiv[pi][indices[pi]]) {
                indices[pi] = i;
            }
        }
    }

    // Put leftmost point into convex hull
    point<T> leftMostPoint = Pdiv[0][indices[0]];
    for (int pi = 1; pi < p; pi++) {
        if (Pdiv[pi][indices[pi]] < leftMostPoint) {
            leftMostPoint = Pdiv[pi][indices[pi]];
        }
    }
    result.push_back(leftMostPoint);

    for (int i = 0; i != H; i++) {
        // For each hull find tangent from result.back()
        point<T> prevHullPoint = result.back();
        for (int pi = 0; pi < p; pi++) {
            while(true) {
                point<T> cur = Pdiv[pi][indices[pi]];
                point<T> next = Pdiv[pi][(indices[pi] + 1) % Pdiv[pi].size()];
                if (cur == prevHullPoint && (cur - prevHullPoint).lenmh() < (next - prevHullPoint).lenmh()) {
                    indices[pi]++;
                    indices[pi] %= Pdiv[pi].size();
                    continue;
                }
                side orientation = cur.sideOfLine(next, prevHullPoint);
                if (orientation == side::right || (orientation == side::on && (cur - prevHullPoint).lenmh() < (next - prevHullPoint).lenmh())) {
                    indices[pi]++;
                    indices[pi] %= Pdiv[pi].size();
                } else {
                    break;
                }
            }
        }
        // Pick next hull point from best of all tangents
        point<T> nextHullPoint = Pdiv[0][indices[0]];
        for (int pi = 1; pi < p; pi++) {
            point<T> candidate = Pdiv[pi][indices[pi]];
            if (nextHullPoint == prevHullPoint && (nextHullPoint - prevHullPoint).lenmh() < (candidate - prevHullPoint).lenmh()) {
                nextHullPoint = candidate;
                continue;
            }
            side orientation = nextHullPoint.sideOfLine(candidate, prevHullPoint);
            if (orientation == side::right || (orientation == side::on && (nextHullPoint - prevHullPoint).lenmh() < (candidate - prevHullPoint).lenmh())) {
                nextHullPoint = candidate;
            }
        }
        result.push_back(nextHullPoint);
        if (nextHullPoint == result[0]) {
            result.pop_back();
            return true;
        }
    }
    return false;
}

// Merge k convex hulls given by Pdiv
// Time complexity O(n+k*h) where n is the total number of input points, and h is the number of output points.
template <typename T>
std::vector<point<T>> Hull2DMerge(const std::vector<std::vector<point<T>>>& Pdiv) {
    if (Pdiv.size()==1) {
        return Pdiv[0];
    }
    std::vector<point<T>> result;
    Hull2DMerge(Pdiv, -1, result);
    return result;
}
