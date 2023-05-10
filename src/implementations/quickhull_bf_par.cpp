#ifdef HAS_TBB
#include "../hull_impl.hpp"
#include "../point.hpp"

#include <algorithm>
#include <memory>
#include <thread>
#include <vector>
#include <barrier>
#include <cassert>
#include <execution>
#include <atomic>
#include <span>
#include <cmath>
#include <iostream>

#include <boost/iterator/counting_iterator.hpp>

auto makeCountingIterator(uint32_t v) {
	return boost::counting_iterator<uint32_t>(v);
}

template <typename execution_policy>
void quickhullParallel(std::vector<pointd>& pts, bool removePoints) {
	execution_policy ep;
	
	auto [itMin, itMax] = std::minmax_element(ep, pts.begin(), pts.end());
	
	pointd allMinPt = *itMin;
	pointd allMaxPt = *itMax;
	
	//Moves points so that points below the line come first
	auto itFirstAbove = std::partition(ep, pts.begin(), pts.end(), [&] (const pointd& p) {
		return p.sideOfLine(allMinPt, allMaxPt, 0.00001) != side::left;
	});
	uint32_t numBelow = itFirstAbove - pts.begin();
	
	//Sorts points by X. Increasing below the first line, and decreasing above
	std::sort(ep, pts.begin(), itFirstAbove, [] (const auto& a, const auto& b) { return a < b; });
	std::sort(ep, itFirstAbove, pts.end(), [] (const auto& a, const auto& b) { return a > b; });
	
	uint32_t allMaxPtIdx = numBelow - 1;
	assert(pts[0] == allMinPt);
	assert(pts[allMaxPtIdx] == allMaxPt);
	
	std::vector<std::pair<uint32_t, uint32_t>> adjHullPoints(pts.size()); // (right, left)
	std::fill(adjHullPoints.begin(), adjHullPoints.begin() + numBelow, std::make_pair(0U, allMaxPtIdx));
	std::fill(adjHullPoints.begin() + numBelow, adjHullPoints.end(), std::make_pair(allMaxPtIdx, 0U));
	adjHullPoints[0] = { 0, 0 };
	adjHullPoints[allMaxPtIdx] = { allMaxPtIdx, allMaxPtIdx };
	
	std::unique_ptr<std::tuple<uint32_t, double, uint32_t>[]> distsPrefixMax(new std::tuple<uint32_t, double, uint32_t>[pts.size()]);
	
	std::unique_ptr<uint32_t[]> numRemovePrefixSum(new uint32_t[pts.size() + 1]);
	numRemovePrefixSum[0] = 0;
	
	std::atomic_bool anyPointActive;
	uint32_t numPoints = pts.size();
	
	auto removePointsInHull = [&] () {
		std::transform_inclusive_scan(ep,
			adjHullPoints.begin(), adjHullPoints.begin() + numPoints,
			&numRemovePrefixSum[1], std::plus<>(),
			[&] (const auto& adj) { return static_cast<uint32_t>(adj.first == UINT32_MAX); }
		);
		
		std::for_each(ep,
			adjHullPoints.begin(), adjHullPoints.begin() + numPoints,
			[&] (auto& adj) {
				if (adj.first != UINT32_MAX) {
					adj.first -= numRemovePrefixSum[adj.first];
					adj.second -= numRemovePrefixSum[adj.second];
				}
			}
		);
		
		auto newPtsEndIt = std::stable_partition(
			ep, pts.begin(), pts.begin() + numPoints,
			[] (const auto& p) { return !std::isnan(p.x); });
		
		assert((numPoints - numRemovePrefixSum[numPoints]) == (newPtsEndIt - pts.begin()));
		
		std::stable_partition(
			ep, adjHullPoints.begin(), adjHullPoints.begin() + numPoints,
			[] (const auto& p) { return p.first != UINT32_MAX; });
		
		numPoints = newPtsEndIt - pts.begin();
	};
	
	do {
		std::transform_inclusive_scan(ep,
			makeCountingIterator(0), makeCountingIterator(numPoints), &distsPrefixMax[0],
			[&] (const auto& a, const auto& b) { return std::max(a, b); },
			[&] (uint32_t idx) {
				auto [hullPtR, hullPtL] = adjHullPoints[idx];
				double dist = -INFINITY;
				if (hullPtR != hullPtL) {
					pointd normal = (pts[hullPtR] - pts[hullPtL]).rotated90CCW();
					dist = (pts[idx] - pts[hullPtL]).dot(normal);
				}
				return std::make_tuple(hullPtR == UINT32_MAX ? 0 : hullPtR, dist, idx);
			}
		);
		
		anyPointActive = false;
		
		std::for_each(ep,
			makeCountingIterator(0), makeCountingIterator(numPoints),
			[&] (uint32_t idx) {
				auto [hullPtR, hullPtL] = adjHullPoints[idx];
				if (hullPtR == hullPtL)
					return;
				
				uint32_t rangeEndIdx = hullPtL ? (hullPtL - 1) : (numPoints - 1);
				uint32_t newHullPtIdx = std::get<2>(distsPrefixMax[rangeEndIdx]);
				if (idx < newHullPtIdx) { // this point is to the right of the new hull point
					hullPtL = newHullPtIdx;
				} else if (idx > newHullPtIdx) { // this point is to the left of the new hull point
					hullPtR = newHullPtIdx;
				} else { // this point is the new hull point
					adjHullPoints[idx] = { idx, idx };
					return;
				}
				
				if (pts[idx].sideOfLine(pts[hullPtL], pts[hullPtR]) == side::left) {
					// outside hull, the point should be kept
					adjHullPoints[idx] = { hullPtR, hullPtL };
					anyPointActive = true;
				} else {
					// inside hull, remove this point
					adjHullPoints[idx] = { UINT32_MAX, UINT32_MAX };
					pts[idx] = pointd(NAN, NAN);
				}
			}
		);
		
		if (removePoints)
			removePointsInHull();
	} while (anyPointActive);
	
	if (!removePoints)
		removePointsInHull();
	
	pts.erase(pts.begin() + numPoints, pts.end());
}

DEF_HULL_IMPL({
	.name = "qhp_bf",
	.runInt = nullptr,
	.runDouble = std::bind(quickhullParallel<std::execution::parallel_policy>, std::placeholders::_1, true),
});

DEF_HULL_IMPL({
	.name = "qhp_bf_seq",
	.runInt = nullptr,
	.runDouble = std::bind(quickhullParallel<std::execution::sequenced_policy>, std::placeholders::_1, true),
});

DEF_HULL_IMPL({
	.name = "qhp_bf_nr",
	.runInt = nullptr,
	.runDouble = std::bind(quickhullParallel<std::execution::parallel_policy>, std::placeholders::_1, false),
});

#endif
