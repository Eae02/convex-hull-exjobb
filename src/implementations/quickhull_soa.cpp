#include "../hull_impl.hpp"
#include "../point.hpp"
#include "../soa_points.hpp"

#include <algorithm>
#include <span>

template <typename T>
void quickhullSoaRec(SOAPoints<T> pts, point<T> leftHullPoint, point<T> rightHullPoint, bool upperHull) {
	if (pts.size() == 0)
		return;
	
	point<T> normal = (rightHullPoint - leftHullPoint).rotated90CCW();
	
	size_t maxPointIdx = 0;
	T maxPointDot = normal.dot(pts[0] - leftHullPoint);
	point<T> maxPoint = pts[0];
	for (size_t i = 1; i < pts.size(); i++) {
		T dotProduct = normal.dot(pts[i] - leftHullPoint);
		if (std::make_pair(dotProduct, pts[i]) > std::make_pair(maxPointDot, maxPoint)) {
			maxPointIdx = i;
			maxPointDot = dotProduct;
			maxPoint = pts[i];
		}
	}
	
	if (pts[maxPointIdx].sideOfLine(leftHullPoint, rightHullPoint) != side::left) {
		std::fill(pts.x.begin(), pts.x.end(), point<T>::notOnHull.x);
		return;
	}
	
	if (pts.size() == 1)
		return;
	
	pts.swapPoints(maxPointIdx, pts.size() - 1);
	
	// Partitioning on x-coordinate reduces number of sideOfLine calls.
	size_t numPointsRight = pts.subspan(0, pts.size() - 1).partition([&] (size_t i) {
		return (pts.x[i] < maxPoint.x) ^ upperHull;
	});
	
	size_t numValidPointsRight = pts.subspan(0, numPointsRight).partition([&] (size_t i) {
		return pts[i].sideOfLine(rightHullPoint, maxPoint) == side::right;
	});
	
	pts.swapPoints(numPointsRight, pts.size() - 1);
	std::fill(pts.x.begin() + numValidPointsRight, pts.x.begin() + numPointsRight, point<T>::notOnHull.x);
	
	auto ptsLeft = pts.subspan(numPointsRight + 1);
	size_t numPointsLeft = ptsLeft.partition([&] (size_t i) {
		return ptsLeft[i].sideOfLine(maxPoint, leftHullPoint) == side::right;
	});
	
	std::fill(ptsLeft.x.begin() + numPointsLeft, ptsLeft.x.end(), point<T>::notOnHull.x);
	
	quickhullSoaRec<T>(pts.subspan(0, numValidPointsRight), maxPoint, rightHullPoint, upperHull);
	quickhullSoaRec<T>(ptsLeft.subspan(0, numPointsLeft), leftHullPoint, maxPoint, upperHull);
}

template <typename T>
size_t runQuickhullSOA(SOAPoints<T> pts) {
	size_t leftmost = pts.findMinIndex();
	point<T> leftmostPt = pts[leftmost];
	pts.swapPoints(0, leftmost);
	
	size_t rightmost = pts.findMaxIndex();
	point<T> rightmostPt = pts[rightmost];
	pts.swapPoints(pts.size() - 1, rightmost);
	
	T lineMinY = std::min(leftmostPt.y, rightmostPt.y);
	T lineMaxY = std::max(leftmostPt.y, rightmostPt.y);
	
	auto ptsInner = pts.subspan(1, pts.size() - 2);
	size_t numPointsBelow = ptsInner.partition([&] (size_t i) {
		//if (ptsInner.y[i] > lineMaxY) return false;
		//if (ptsInner.y[i] < lineMinY) return true;
		return ptsInner[i].sideOfLine(leftmostPt, rightmostPt) == side::right;
	});
	
	pts.swapPoints(pts.size() - 1, numPointsBelow + 1);
	
	quickhullSoaRec<T>(pts.subspan(1, numPointsBelow), rightmostPt, leftmostPt, false);
	quickhullSoaRec<T>(pts.subspan(numPointsBelow + 2), leftmostPt, rightmostPt, true);
	
	return pts.partition([&] (size_t i) { return !isNotOnHull(pts.x[i]); });
}

DEF_HULL_IMPL({
	.name = "qh_soa",
	.runIntSoa = &runQuickhullSOA<int64_t>,
	.runDoubleSoa = &runQuickhullSOA<double>,
});
