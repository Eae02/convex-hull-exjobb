#include "../hull_impl.hpp"
#include "../point.hpp"
#include "impl1.cpp"

#include <algorithm>
#include <vector>
#include <math.h>

template <typename T>
bool Hull2DMerge(const std::vector<std::vector<point<T>>>& Pdiv, int H, std::vector<point<T>>& result) {
    int p = Pdiv.size();
    std::vector<int> indices(p);
    for (int pi = 0; pi<p; pi++) {
        for (int i = 0; i < Pdiv[pi].size(); i++) {
            if (Pdiv[pi][i]< Pdiv[pi][indices[pi]]) {
                indices[pi] = i;
            }
        }
    }

    // Put leftmost point into convex hull
    result.push_back(Pdiv[0][indices[0]]);
    for (int pi = 0; pi<p; pi++) {
        if (Pdiv[pi][indices[pi]]< result[0]) {
            result[0] = Pdiv[pi][indices[pi]];
        }
    }
    for (int i = 0; i < H; i++) {
        // For each hull find tangent from result.back()
        for (int pi = 0; pi<p; pi++) {
            while(true) {
                point<T> cur = Pdiv[pi][indices[pi]];
                point<T> next = Pdiv[pi][(indices[pi]+1)%Pdiv[pi].size()];
                side orientation = cur.sideOfLine(next,result.back());
                if (orientation == side::right || (orientation == side::on && (cur-result.back()).len2() < (next-result.back()).len2())) {
                    indices[pi]++;
                    indices[pi]%=Pdiv[pi].size();
                } else {
                    break;
                }
            }
        }
        // Pick next hull point from best of all tangents
        point<T> prevHullPoint = result.back();
        result.push_back(Pdiv[0][indices[0]]);
        for (int pi = 0; pi<p; pi++) {
            point<T> candidate = Pdiv[pi][indices[pi]];
            side orientation = result.back().sideOfLine(candidate,prevHullPoint);
            if (orientation == side::right || (orientation == side::on && (result.back()-prevHullPoint).len2() < (candidate-prevHullPoint).len2())) {
                result.back() = candidate;
            }
        }
        if (result.back() == result[0]) {
            result.pop_back();
            return true;
        }

    }


    return false;
}


template <typename T>
bool Hull2D(std::vector<point<T>>& pts, int m, int H) {
    int p = (pts.size()+m-1)/m; // Number of partitions, ceil(n/m)
    std::vector<std::vector<point<T>>> Pdiv(p); // Division of the points into p sets
    for (int i = 0; i< pts.size(); i++) {
        Pdiv[i%p].push_back(pts[i]);
    }
    for (int pi = 0; pi<p; pi++) {
        runImpl1(Pdiv[pi]);
    }
    std::vector<point<T>> result;
    if (Hull2DMerge(Pdiv,H,result)) {
        std::swap(result,pts);
        return true;
    }

    // Refinement idea 1 from chans paper, remove known interior points from further consideration.
    pts.clear();
    for (int pi = 0; pi<p; pi++) {
        std::copy(Pdiv[pi].begin(), Pdiv[pi].end(), std::back_inserter(pts));
    }

    return false;
}

template <typename T>
void runChen(std::vector<point<T>>& pts) {
	if (pts.size() <= 2) return;
    long long t = 1;
    while (true) {
        int H = std::min((long long) pts.size(),1LL << (1LL<<t));
        if (H<1) { // Catch overflow
            H = pts.size();
        }
        // Refinement idea 2 from chans paper, put m = H*logH
        if (Hull2D(pts,H*(1LL<<t),H)) {
            return;
        }
        t++;
    }
}

DEF_HULL_IMPL({
	.name = "chen",
	.runInt = &runChen<int64_t>,
	.runDouble = &runChen<double>,
});
