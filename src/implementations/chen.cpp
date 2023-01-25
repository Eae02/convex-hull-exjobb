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
            // if (result.back() == Pdiv[pi][indices[pi]]) {
            //     indices[pi]++;
            //     indices[pi]%=Pdiv[pi].size();
            // }
            while(true) {
                point<T> cur = Pdiv[pi][indices[pi]];
                point<T> next = Pdiv[pi][(indices[pi]+1)%Pdiv[pi].size()];
                if (cur.cross(next,result.back())<0 || (cur.cross(next,result.back())==0 && (cur-result.back()).len2() < (next-result.back()).len2())) {
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
            if (result.back().cross(candidate,prevHullPoint) < 0 || (result.back().cross(candidate,prevHullPoint) == 0 && (result.back()-prevHullPoint).len2() < (candidate-prevHullPoint).len2())) {
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
    return false;
}

template <typename T>
void runChen(std::vector<point<T>>& pts) {
	if (pts.size() <= 2) return;
    int t = 1;
    while (true) {
        int m = std::min(int(pts.size()),1 << (1<<t));
        if (m<1) { // Catch overflow
            m = pts.size();
        }
        if (Hull2D(pts,m,m)) {
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