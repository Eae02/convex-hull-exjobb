#include "../hull_impl.hpp"
#include "../point.hpp"
#include "quickhull_common.hpp"

#include <algorithm>
#include <vector>
#include <span>
#include <cmath>

template <typename T>
static void quickhullRec2(std::span<point<T>> pts, point<T> maxPoint, point<T> leftHullPoint, point<T> rightHullPoint) {
	if (pts.empty())
		return;
	
	if (maxPoint.sideOfLine(leftHullPoint, rightHullPoint) != side::left) {
		std::fill(pts.begin(), pts.end(), point<T>::notOnHull);
		return;
	}
	
	point<T> normalR = (rightHullPoint - maxPoint).rotated90CCW();
	point<T> normalL = (maxPoint - leftHullPoint).rotated90CCW();
	
	point<T> maxPointR;
	point<T> maxPointL;
	T maxPointDistR = 0;
	T maxPointDistL = 0;
	
	bool upperHull = leftHullPoint < rightHullPoint;
	
	size_t numPointsRight = 0;
	size_t numPointsNotLeft = pts.size();
	for (size_t i = numPointsRight; i < numPointsNotLeft;) {
		if ((pts[i].x < maxPoint.x) ^ upperHull) {
			T dot = normalR.dot(pts[i] - maxPoint);
			if (dot > 0) {
				if (std::tie(dot, pts[i]) > std::tie(maxPointDistR, maxPointR)) {
					maxPointDistR = dot;
					maxPointR = pts[i];
				}
				if (numPointsRight != i) {
					std::swap(pts[numPointsRight], pts[i]);
				}
				numPointsRight++;
				i++;
				continue;
			}
		} else {
			T dot = normalL.dot(pts[i] - leftHullPoint);
			if (dot > 0) {
				if (std::tie(dot, pts[i]) > std::tie(maxPointDistL, maxPointL)) {
					maxPointDistL = dot;
					maxPointL = pts[i];
				}
				numPointsNotLeft--;
				if (numPointsNotLeft != i) {
					std::swap(pts[numPointsNotLeft], pts[i]);
				}
				continue;
			}
		}
		pts[i++] = point<T>::notOnHull;
	}
	
	pts[numPointsRight] = maxPoint;
	
	quickhullRec2<T>(pts.subspan(0, numPointsRight), maxPointR, maxPoint, rightHullPoint);
	quickhullRec2<T>(pts.subspan(numPointsNotLeft), maxPointL, leftHullPoint, maxPoint);
}

template <typename T>
void runQuickhull2(std::vector<point<T>>& pts) {
	std::pair<point<T>, size_t> maxX, minX;
	maxX = minX = std::make_pair(pts[0], (size_t)0);
	
	for (size_t i = 1; i < pts.size(); i++) {
		maxX = std::max(maxX, std::make_pair(pts[i], i));
		minX = std::min(minX, std::make_pair(pts[i], i));
	}
	
	if (minX.second != 0) {
		std::swap(pts.front(), pts[minX.second]);
		if (maxX.second == 0) {
			maxX.second = minX.second;
		}
	}
	if (maxX.second != pts.size() - 1) {
		std::swap(pts.back(), pts[maxX.second]);
	}
	
	point<T> leftmostPt = minX.first;
	point<T> rightmostPt = maxX.first;
	
	point<T> upDirection = (rightmostPt - leftmostPt).rotated90CCW();
	
	std::pair<T, point<T>> maxYPt(-1, {});
	std::pair<T, point<T>> minYPt(1, {});
	
	size_t belowPointsEndIdx = 1;
	for (size_t i = 1; i < pts.size() - 1; i++) {
		T dot = upDirection.dot(pts[i] - leftmostPt);
		if (dot > 0) {
			maxYPt = std::max(maxYPt, std::make_pair(dot, pts[i]));
		} else {
			minYPt = std::min(minYPt, std::make_pair(dot, pts[i]));
			std::swap(pts[i], pts[belowPointsEndIdx++]);
		}
	}
	
	std::swap(pts.back(), pts[belowPointsEndIdx]);
	
	quickhullRec2<T>(std::span<point<T>>(&pts[1], &pts[belowPointsEndIdx]), minYPt.second, rightmostPt, leftmostPt);
	quickhullRec2<T>(std::span<point<T>>(&pts[belowPointsEndIdx] + 1, pts.data() + pts.size()), maxYPt.second, leftmostPt, rightmostPt);
	
	removeNotOnHull(pts);
}

DEF_HULL_IMPL({
	.name = "qh_rec2_ss",
	.runInt = runQuickhull2<int64_t>,
	.runDouble = runQuickhull2<double>,
});
