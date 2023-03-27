/*
Quickhull that switches to a different algorithm at at certain point.
Write :D<n> or :P<n> after the implementation name to switch at depth <n> or when there are at most <n> points left, respectively.
For example, qh_hybrid_jw:P10 will switch to jarvis wrap when there are 10 points remaining.
 */

#include "../hull_impl.hpp"
#include "../point.hpp"
#include "monotone_chain.hpp"

#include <algorithm>
#include <vector>
#include <span>
#include <charconv>
#include <cmath>

template <typename T>
static void hybridJarvisWrap(std::span<point<T>> pts, point<T> leftHullPoint, point<T> rightHullPoint) {
	size_t numHullPts = 0;
	
	point<T> firstHullPoint = rightHullPoint;
	point<T> lastHullPoint = firstHullPoint;
	while (numHullPts < pts.size()) {
		point<T> nextHullPoint = leftHullPoint;
		size_t nextHullPointIdx = SIZE_MAX;
		for (size_t i = numHullPts; i < pts.size(); i++) {
			auto side = pts[i].sideOfLine(lastHullPoint, nextHullPoint);
			if (side == side::right ||
				(side == side::on && (pts[i] - lastHullPoint).lenmh() > (nextHullPoint - lastHullPoint).lenmh())
				) {
				nextHullPointIdx = i;
				nextHullPoint = pts[i];
			}
		}
		if (nextHullPointIdx == SIZE_MAX)
			break;
		lastHullPoint = pts[nextHullPointIdx];
		std::swap(pts[nextHullPointIdx], pts[numHullPts]);
		numHullPts++;
	}
	
	std::fill(pts.begin() + numHullPts, pts.end(), point<T>::notOnHull);
}

template <typename T>
void quickhullRecESX(std::span<point<T>> pts, point<T> leftHullPoint, point<T> rightHullPoint);

template <typename T>
static void hybridQuickhullESX(std::span<point<T>> pts, point<T> leftHullPoint, point<T> rightHullPoint) {
	if (leftHullPoint.x < rightHullPoint.x) {
		std::sort(pts.begin(), pts.end(), [] (const auto& a, const auto& b) { return a.x > b.x; });
	} else {
		std::sort(pts.begin(), pts.end(), [] (const auto& a, const auto& b) { return a.x < b.x; });
	}
	
	quickhullRecESX(pts, leftHullPoint, rightHullPoint);
}

template <typename T>
struct HybridData {
	size_t changeThresholdPoints = 0;
	size_t changeThresholdDepth = SIZE_MAX;
	void(*impl)(std::span<point<T>>, point<T>, point<T>);
};

template <typename T>
void quickhullHybridRec(
	std::span<point<T>> pts, point<T> leftHullPoint, point<T> rightHullPoint,
	bool upperHull, size_t depth, const HybridData<T>& hybridData
) {
	if (pts.empty())
		return;
	
	if ((pts.size() <= hybridData.changeThresholdPoints || depth >= hybridData.changeThresholdDepth) && depth != 0) {
		hybridData.impl(pts, leftHullPoint, rightHullPoint);
		return;
	}
	
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
	
	auto rightPointsValidEndIt = std::partition(pts.begin(), rightPointsEndIt, [&] (const point<T>& p) -> bool {
		return p.sideOfLine(rightHullPoint, maxPoint) == side::right;
	});

	std::swap(*rightPointsEndIt, pts.back());
	std::fill(rightPointsValidEndIt, rightPointsEndIt, point<T>::notOnHull);

	auto leftPointsEndIt = std::partition(rightPointsEndIt + 1, pts.end(), [&] (const point<T>& p) -> bool {
		return p.sideOfLine(maxPoint, leftHullPoint) == side::right;
	});

	size_t numPointsRight = rightPointsEndIt - pts.begin();
	size_t numPointsValidRight = rightPointsValidEndIt - pts.begin();
	size_t numPointsLeft = (leftPointsEndIt - rightPointsEndIt) - 1;
    
	std::fill(leftPointsEndIt, pts.end(), point<T>::notOnHull);

	auto rightSubspan = pts.subspan(0, numPointsValidRight);
	quickhullHybridRec<T>(rightSubspan, maxPoint, rightHullPoint, upperHull, depth + 1, hybridData);
	
	auto leftSubspan = pts.subspan(numPointsRight + 1, numPointsLeft);
	quickhullHybridRec<T>(leftSubspan, leftHullPoint, maxPoint, upperHull, depth + 1, hybridData);
}

template <typename T>
void runQuickhullHybrid(std::vector<point<T>>& pts, void(*innerImpl)(std::span<point<T>>, point<T>, point<T>)) {
	HybridData<T> hybridData;
	hybridData.impl = innerImpl;
	if (auto value = getImplArgInt("D"))
		hybridData.changeThresholdDepth = *value;
	if (auto value = getImplArgInt("P"))
		hybridData.changeThresholdPoints = *value;
	
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
	
	quickhullHybridRec<T>(std::span<point<T>>(&pts[1], &*belowPointsEndIt), rightmostPt, leftmostPt, false, 0, hybridData);
	quickhullHybridRec<T>(std::span<point<T>>(&*belowPointsEndIt + 1, pts.data() + pts.size()), leftmostPt, rightmostPt, true, 0, hybridData);
	
	removeNotOnHull(pts);
}

DEF_HULL_IMPL({
	.name = "qh_hybrid_mc",
	.runInt = std::bind(runQuickhullHybrid<int64_t>, std::placeholders::_1, &solveOneMonotoneChain<int64_t>),
	.runDouble = std::bind(runQuickhullHybrid<double>, std::placeholders::_1, &solveOneMonotoneChain<double>),
});

DEF_HULL_IMPL({
	.name = "qh_hybrid_jw",
	.runInt = std::bind(runQuickhullHybrid<int64_t>, std::placeholders::_1, &hybridJarvisWrap<int64_t>),
	.runDouble = std::bind(runQuickhullHybrid<double>, std::placeholders::_1, &hybridJarvisWrap<double>),
});

DEF_HULL_IMPL({
	.name = "qh_hybrid_esx",
	.runInt = std::bind(runQuickhullHybrid<int64_t>, std::placeholders::_1, &hybridQuickhullESX<int64_t>),
	.runDouble = std::bind(runQuickhullHybrid<double>, std::placeholders::_1, &hybridQuickhullESX<double>),
});
