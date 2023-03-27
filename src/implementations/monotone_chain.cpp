#include "monotone_chain.hpp"
#include "../hull_impl.hpp"

#include <vector>
#include <algorithm>

template <typename T>
void solveOneMonotoneChain(std::span<point<T>> pts, point<T> leftHullPoint, point<T> rightHullPoint) {
	if (leftHullPoint < rightHullPoint)
		std::sort(pts.begin(), pts.end(), std::greater<>());
	else
		std::sort(pts.begin(), pts.end(), std::less<>());
	
	size_t numOnHull = 0;
	auto removeNonConvex = [&] (const point<T>& newPoint) {
		while (numOnHull >= 2 && newPoint.sideOfLine(pts[numOnHull - 2], pts[numOnHull - 1]) != side::left) {
			pts[--numOnHull] = point<T>::notOnHull;
		}
		if (numOnHull == 1 && newPoint.sideOfLine(rightHullPoint, pts[0]) != side::left) {
			pts[--numOnHull] = point<T>::notOnHull;
		}
	};
	for (size_t i = 0; i < pts.size(); i++) {
		removeNonConvex(pts[i]);
		std::swap(pts[i], pts[numOnHull++]);
	}
	removeNonConvex(leftHullPoint);
}

template <typename T>
void solveMonotoneChain(std::vector<point<T>>& pts) {
	size_t leftmost = std::min_element(pts.begin(), pts.end()) - pts.begin();
	point<T> leftmostPt = pts[leftmost];
	std::swap(pts.front(), pts[leftmost]);
	
	size_t rightmost = std::max_element(pts.begin(), pts.end()) - pts.begin();
	point<T> rightmostPt = pts[rightmost];
	std::swap(pts.back(), pts[rightmost]);
	
	auto belowPointsEndIt = std::partition(pts.begin() + 1, pts.end() - 1, [&] (const point<T>& p) -> bool {
		return p.sideOfLine(leftmostPt, rightmostPt) == side::right;
	});
	
	std::swap(pts.back(), *belowPointsEndIt);
	
	solveOneMonotoneChain<T>(std::span<point<T>>(&pts[1], &*belowPointsEndIt), rightmostPt, leftmostPt);
	solveOneMonotoneChain<T>(std::span<point<T>>(&*belowPointsEndIt + 1, pts.data() + pts.size()), leftmostPt, rightmostPt);
	
	removeNotOnHull(pts);
}

DEF_HULL_IMPL({
	.name = "mc",
	.runInt = &solveMonotoneChain<int64_t>,
	.runDouble = &solveMonotoneChain<double>,
});
