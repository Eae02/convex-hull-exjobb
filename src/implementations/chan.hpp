#pragma once

#include <vector>
#include "../point.hpp"

template <typename T>
bool Hull2DMerge(const std::vector<std::vector<point<T>>>& Pdiv, int H, std::vector<point<T>>& result) {
    int p = Pdiv.size();
    std::vector<int> indices(p);
    for (int pi = 0; pi < p; pi++) {
        for (int i = 0; i < Pdiv[pi].size(); i++) {
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

    for (int i = 0; i < H; i++) {
        // For each hull find tangent from result.back()
        point<T> prevHullPoint = result.back();
        for (int pi = 0; pi < p; pi++) {
            while(true) {
                point<T> cur = Pdiv[pi][indices[pi]];
                point<T> next = Pdiv[pi][(indices[pi] + 1) % Pdiv[pi].size()];
                side orientation = cur.sideOfLine(next, prevHullPoint);
                if (orientation == side::right || (orientation == side::on && (cur - prevHullPoint).len2() < (next - prevHullPoint).len2())) {
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
            side orientation = nextHullPoint.sideOfLine(candidate, prevHullPoint);
            if (orientation == side::right || (orientation == side::on && (nextHullPoint - prevHullPoint).len2() < (candidate - prevHullPoint).len2())) {
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
