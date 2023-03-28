#include "../hull_impl.hpp"
#include "../point.hpp"

#include <algorithm>
#include <vector>
#include <span>
#include <cmath>
#include <thread>
#include <list>
#include <mutex>

struct ParallelData {
	std::mutex lock;
	std::list<std::thread> threads;
};

template <typename T>
void quickhullRecXYPartition(
	std::span<point<T>> pts, point<T> leftHullPoint, point<T> rightHullPoint,
	bool upperHull
) {
	if (pts.empty())
		return;
	
	point<T> normal = (rightHullPoint - leftHullPoint).rotated90CCW();
	
	size_t maxPointIdx = 0;
	T maxPointDot = normal.dot(pts[0] - leftHullPoint);
	point<T> maxPoint = pts[0];
	for (size_t i = 1; i < pts.size(); i++) {
		T dotProduct = normal.dot(pts[i] - leftHullPoint);
		if (std::tie(dotProduct, pts[i]) > std::tie(maxPointDot, maxPoint)) {
			maxPointIdx = i;
			maxPointDot = dotProduct;
			maxPoint = pts[i];
		}
	}
	
	if (pts[maxPointIdx].sideOfLine(leftHullPoint, rightHullPoint) != side::left) {
		std::fill(pts.begin(), pts.end(), point<T>::notOnHull);
		return;
	}
	
	if (pts.size() == 1)
		return;
	
	std::swap(pts[maxPointIdx], pts.back());
	
	// Partitioning on x-coordinate reduces number of sideOfLine calls.
	auto rightPointsEndIt = std::partition(pts.begin(), pts.end() - 1, [&] (const point<T>& p) -> bool {
		return (p.x < maxPoint.x) ^ upperHull;
	});
	bool rightYPartition = (maxPoint.y >= rightHullPoint.y) ^ upperHull;
	auto rightPointsValidEndIt = std::partition(pts.begin(), rightPointsEndIt, [&] (const point<T>& p) -> bool {
		if (rightYPartition && ((p.y >= maxPoint.y) ^ upperHull)) return false; // prune points that are clearly not above line.
		return p.sideOfLine(rightHullPoint, maxPoint) == side::right;
	});

	std::swap(*rightPointsEndIt, pts.back());
	std::fill(rightPointsValidEndIt, rightPointsEndIt, point<T>::notOnHull);

	bool leftYPartition = (maxPoint.y >= leftHullPoint.y) ^ upperHull;
	auto leftPointsEndIt = std::partition(rightPointsEndIt + 1, pts.end(), [&] (const point<T>& p) -> bool {
		if (leftYPartition && ((p.y >= maxPoint.y) ^ upperHull)) return false;
		return p.sideOfLine(maxPoint, leftHullPoint) == side::right;
	});

	size_t numPointsRight = rightPointsEndIt - pts.begin();
	size_t numPointsValidRight = rightPointsValidEndIt - pts.begin();
	size_t numPointsLeft = (leftPointsEndIt - rightPointsEndIt) - 1;
    
	std::fill(leftPointsEndIt, pts.end(), point<T>::notOnHull);

	quickhullRecXYPartition<T>(pts.subspan(0, numPointsValidRight), maxPoint, rightHullPoint, upperHull);	
	quickhullRecXYPartition<T>(pts.subspan(numPointsRight + 1, numPointsLeft), leftHullPoint, maxPoint, upperHull);
}

template <typename T>
void runQuickhullXYPartition(std::vector<point<T>>& pts) {
	size_t leftmost = std::min_element(pts.begin(), pts.end()) - pts.begin();
	point<T> leftmostPt = pts[leftmost];
	std::swap(pts.front(), pts[leftmost]);
	
	size_t rightmost = std::max_element(pts.begin(), pts.end()) - pts.begin();
	point<T> rightmostPt = pts[rightmost];
	std::swap(pts.back(), pts[rightmost]);
	
	T lineMinY = std::min(leftmostPt.y, rightmostPt.y);
	T lineMaxY = std::max(leftmostPt.y, rightmostPt.y);

	auto belowPointsEndIt = std::partition(pts.begin() + 1, pts.end() - 1, [&] (const point<T>& p) -> bool {
		if (p.y > lineMaxY) return false;
		if (p.y < lineMinY) return true;
		return p.sideOfLine(leftmostPt, rightmostPt) == side::right;
	});
	
	std::swap(pts.back(), *belowPointsEndIt);
	
	
	quickhullRecXYPartition<T>(std::span<point<T>>(&pts[1], &*belowPointsEndIt), rightmostPt, leftmostPt, false);
	quickhullRecXYPartition<T>(std::span<point<T>>(&*belowPointsEndIt + 1, pts.data() + pts.size()), leftmostPt, rightmostPt, true);

	removeNotOnHull(pts);
}

DEF_HULL_IMPL({
	.name = "qh_rec_xyp",
	.runInt = runQuickhullXYPartition<int64_t>,
	.runDouble = runQuickhullXYPartition<double>,
});
