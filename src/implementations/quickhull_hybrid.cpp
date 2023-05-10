/*
Quickhull that switches to a different algorithm at at certain point.
Write :D<n> or :P<n> after the implementation name to switch at depth <n> or when there are at most <n> points left, respectively.
For example, qh_hybrid_jw:P10 will switch to jarvis wrap when there are 10 points remaining.
 */

#include "../hull_impl.hpp"
#include "../point.hpp"
#include "monotone_chain.hpp"
#include "quickhull_common.hpp"

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

template <qhPartitionStrategy S, typename T>
void quickhullHybridRec(
	std::span<point<T>> pts, point<T> leftHullPoint, point<T> rightHullPoint,
	size_t depth, const HybridData<T>& hybridData
) {
	if (pts.empty())
		return;
	
	if (pts.size() <= hybridData.changeThresholdPoints || depth >= hybridData.changeThresholdDepth) {
		hybridData.impl(pts, leftHullPoint, rightHullPoint);
		return;
	}
	
	size_t maxPointIdx = findFurthestPointFromLine<T>(pts, leftHullPoint, rightHullPoint);
	point<T> maxPoint = pts[maxPointIdx];
	
	if (pts[maxPointIdx].sideOfLine(leftHullPoint, rightHullPoint) != side::left) {
		std::fill(pts.begin(), pts.end(), point<T>::notOnHull);
		return;
	}
	
	if (pts.size() == 1)
		return;
	
	auto [rightSubspan, leftSubspan] = quickhullPartitionPoints<S, T>(pts, leftHullPoint, rightHullPoint, maxPointIdx);
	
	quickhullHybridRec<S, T>(rightSubspan, maxPoint, rightHullPoint, depth + 1, hybridData);
	quickhullHybridRec<S, T>(leftSubspan, leftHullPoint, maxPoint, depth + 1, hybridData);
}

template <typename T, qhPartitionStrategy S = qhPartitionStrategy::noPartitionByX>
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
	
	quickhullHybridRec<S, T>(std::span<point<T>>(&pts[1], &*belowPointsEndIt), rightmostPt, leftmostPt, 0, hybridData);
	quickhullHybridRec<S, T>(std::span<point<T>>(&*belowPointsEndIt + 1, pts.data() + pts.size()), leftmostPt, rightmostPt, 0, hybridData);
	
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
