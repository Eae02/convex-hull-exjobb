#include "../hull_impl.hpp"
#include "../point.hpp"

#include <algorithm>
#include <vector>

template <typename T>
void quickhullRec(const std::vector<point<T>>& pts, std::vector<point<T>>& output, point<T> leftHullPoint, point<T> rightHullPoint) {
	if (pts.size() == 1) {
		output.push_back(pts[0]);
		return;
	}
	if (pts.size() == 0)
		return;
	
	point<T> normal = (rightHullPoint - leftHullPoint).rotated90CCW();
	
	size_t maxPointIdx = 0;
	T maxPointDot = normal.dot(pts[0]);
	for (size_t i = 1; i < pts.size(); i++) {
		T dotProduct = normal.dot(pts[i]);
		if (dotProduct > maxPointDot) {
			maxPointIdx = i;
			maxPointDot = dotProduct;
		}
	}
	
	std::vector<point<T>> ptsL, ptsR;
	for (size_t i = 0; i < pts.size(); i++) {
		if (pts[i].cross(pts[maxPointIdx], leftHullPoint) < 0) {
			ptsL.push_back(pts[i]);
		} else if (pts[i].cross(rightHullPoint, pts[maxPointIdx]) < 0) {
			ptsR.push_back(pts[i]);
		}
	}
	
	quickhullRec(ptsL, output, leftHullPoint, pts[maxPointIdx]);
	output.push_back(pts[maxPointIdx]);
	quickhullRec(ptsR, output, pts[maxPointIdx], rightHullPoint);
}

template <typename T>
void runQuickhull(std::vector<point<T>>& pts) {
	size_t leftmost = std::min_element(pts.begin(), pts.end()) - pts.begin();
	std::swap(pts[0], pts[leftmost]);
	
	size_t rightmost = std::max_element(pts.begin(), pts.end()) - pts.begin();
	std::swap(pts[1], pts[rightmost]);
	
	std::vector<point<T>> ptsAbove, ptsBelow, result;
	
	for (size_t i = 2; i < pts.size(); i++) {
		auto cross = pts[i].cross(pts[1], pts[0]);
		if (cross > 0) {
			ptsBelow.push_back(pts[i]);
		} else if (cross < 0) {
			ptsAbove.push_back(pts[i]);
		}
	}
	
	result.push_back(pts[0]);
	quickhullRec(ptsAbove, result, pts[0], pts[1]);
	result.push_back(pts[1]);
	quickhullRec(ptsBelow, result, pts[1], pts[0]);
	std::reverse(result.begin(), result.end());
	pts = result;
}

DEF_HULL_IMPL({
	.name = "quickhull",
	.runInt = &runQuickhull<int64_t>,
	.runDouble = &runQuickhull<double>,
});
