#include "../hull_impl.hpp"
#include "../point.hpp"
#include "quickhull_common.hpp"

#include <algorithm>
#include <vector>
#include <span>
#include <cmath>

namespace qh_bf {
	template <typename T>
	struct Data {
		std::vector<std::pair<uint32_t, uint32_t>> intervals;
		std::vector<std::pair<uint32_t, uint32_t>> intervalsNext;
		std::vector<std::pair<uint32_t, uint32_t>> intervalHullPoints;
		std::vector<std::pair<uint32_t, uint32_t>> intervalHullPointsNext;
		
		void initialize(std::vector<point<T>>& pts);
		
		void compactAndRemoveNotOnHull(std::vector<point<T>>& pts);
		
		void addInterval(uint32_t lo, uint32_t hi) {
			if (hi > lo) {
				intervalsNext.emplace_back(lo, hi);
			}
		}
		
		void addInterval(uint32_t lo, uint32_t hi, uint32_t hullPointR, uint32_t hullPointL) {
			if (hi > lo) {
				intervalsNext.emplace_back(lo, hi);
				intervalHullPointsNext.emplace_back(hullPointR, hullPointL);
			}
		}
		
		void swapIntervals() {
			intervalsNext.swap(intervals);
			intervalsNext.clear();
			intervalHullPointsNext.swap(intervalHullPoints);
			intervalHullPointsNext.clear();
		}
	};
	
	template <typename T>
	void Data<T>::initialize(std::vector<point<T>>& pts) {
		size_t leftmost = std::min_element(pts.begin(), pts.end()) - pts.begin();
		point<T> leftmostPt = pts[leftmost];
		std::swap(pts.front(), pts[leftmost]);
		
		size_t rightmost = std::max_element(pts.begin(), pts.end()) - pts.begin();
		point<T> rightmostPt = pts[rightmost];
		std::swap(pts.back(), pts[rightmost]);
		
		auto belowPointsEndIt = std::partition(pts.begin() + 1, pts.end() - 1, [&] (const point<T>& p) -> bool {
			return p.sideOfLine(leftmostPt, rightmostPt) == side::right;
		});
		uint32_t belowPointsEndIdx = belowPointsEndIt - pts.begin();
		
		std::swap(pts.back(), *belowPointsEndIt);
		
		addInterval(1, belowPointsEndIdx);
		addInterval(belowPointsEndIdx + 1, pts.size());
		swapIntervals();
	}
	
	template <typename T>
	void Data<T>::compactAndRemoveNotOnHull(std::vector<point<T>>& pts) {
		uint32_t numPointsKept = 0;
		uint32_t prevHi = 0;
		
		auto copyBetween = [&] (uint32_t lo, uint32_t hi) {
			if (hi > lo) {
				numPointsKept = std::copy_if(
					pts.begin() + lo, pts.begin() + hi, pts.begin() + numPointsKept,
					[&] (const auto& p) { return !p.isNotOnHull(); }) - pts.begin();
			}
		};
		
		for (auto& interval : intervals) {
			auto [ilo, ihi] = interval;
			copyBetween(prevHi, ilo);
			std::copy(pts.begin() + ilo, pts.begin() + ihi, pts.begin() + numPointsKept);
			interval.first = numPointsKept;
			numPointsKept += ihi - ilo;
			interval.second = numPointsKept;
			prevHi = ihi;
		}
		
		copyBetween(prevHi, pts.size());
		pts.resize(numPointsKept);
		
		intervalHullPoints.clear();
	}
	
	inline std::pair<uint32_t, uint32_t> getImplicitHullPointsForInterval(uint32_t ilo, uint32_t ihi, uint32_t numPoints) {
		return { ilo - 1, ihi == numPoints ? 0 : ihi };
	}
	
	template <qhPartitionStrategy S, typename T>
	void run_alwaysCompact(std::vector<point<T>>& pts, Data<T>& data) {
		while (!data.intervals.empty()) {
			uint32_t nextOutIdx = data.intervals[0].first;
			
			for (size_t ii = 0; ii < data.intervals.size(); ii++) {
				auto [ilo, ihi] = data.intervals[ii];
				auto [rightHullPointIdx, leftHullPointIdx] = getImplicitHullPointsForInterval(ilo, ihi, pts.size());
				auto rightHullPoint = pts[rightHullPointIdx];
				auto leftHullPoint = pts[leftHullPointIdx];
				
				std::span<point<T>> ptsSpan(pts.data() + ilo, pts.data() + ihi);
				
				size_t maxPointIdx = findFurthestPointFromLine<T>(ptsSpan, leftHullPoint, rightHullPoint);
				
				if (ptsSpan[maxPointIdx].sideOfLine(leftHullPoint, rightHullPoint) == side::left) {
					if (ihi == ilo + 1) {
						pts[nextOutIdx++] = pts[ilo];
					} else {
						auto [rightSubspan, leftSubspan] = quickhullPartitionPoints<S, T, false>(ptsSpan, leftHullPoint, rightHullPoint, maxPointIdx);
						
						uint32_t rightIntvLo = nextOutIdx;
						std::copy(rightSubspan.begin(), rightSubspan.end(), pts.begin() + nextOutIdx);
						nextOutIdx += rightSubspan.size();
						uint32_t rightIntvHi = nextOutIdx;
						
						pts[nextOutIdx++] = ptsSpan[maxPointIdx];
						
						uint32_t leftIntvLo = nextOutIdx;
						std::copy(leftSubspan.begin(), leftSubspan.end(), pts.begin() + nextOutIdx);
						nextOutIdx += leftSubspan.size();
						uint32_t leftIntvHi = nextOutIdx;
						
						data.addInterval(rightIntvLo, rightIntvHi);
						data.addInterval(leftIntvLo, leftIntvHi);
					}
				}
				
				uint32_t nextIntvLo = ii == data.intervals.size() - 1 ? pts.size() : data.intervals[ii + 1].first;
				if (nextOutIdx != ihi)
					std::copy(pts.begin() + ihi, pts.begin() + nextIntvLo, pts.begin() + nextOutIdx);
				nextOutIdx += nextIntvLo - ihi;
			}
			data.swapIntervals();
			pts.resize(nextOutIdx);
		}
	}
	
	template <qhPartitionStrategy S, typename T>
	int runSingleStepWithoutCompaction(std::vector<point<T>>& pts, Data<T>& data) {
		int numNotCompacted = 0;
		for (size_t ii = 0; ii < data.intervals.size(); ii++) {
			auto [ilo, ihi] = data.intervals[ii];
			
			uint32_t rightHullPointIdx, leftHullPointIdx;
			if (data.intervalHullPoints.empty()) {
				std::tie(rightHullPointIdx, leftHullPointIdx) = getImplicitHullPointsForInterval(ilo, ihi, pts.size());
			} else {
				std::tie(rightHullPointIdx, leftHullPointIdx) = data.intervalHullPoints[ii];
			}
			point<T> rightHullPoint = pts[rightHullPointIdx];
			point<T> leftHullPoint = pts[leftHullPointIdx];
			
			std::span<point<T>> ptsSpan(pts.data() + ilo, pts.data() + ihi);
			
			size_t maxPointIdx = findFurthestPointFromLine<T>(ptsSpan, leftHullPoint, rightHullPoint);
			
			//If the max point is not outside the line between the left and right hull point,
			// remove all points in this interval
			if (pts[ilo + maxPointIdx].sideOfLine(leftHullPoint, rightHullPoint) != side::left) {
				numNotCompacted += ihi - ilo;
				std::fill(pts.begin() + ilo, pts.begin() + ihi, point<T>::notOnHull);
				continue;
			}
			
			//If there is only a single point in this interval, keep this point as a hull point
			if (ihi == ilo + 1)
				continue;
			
			auto [rightSubspan, leftSubspan] = quickhullPartitionPoints<S>(ptsSpan, leftHullPoint, rightHullPoint, maxPointIdx);
			
			uint32_t rightSubspanILo = rightSubspan.data() - pts.data();
			uint32_t leftSubspanILo = leftSubspan.data() - pts.data();
			
			data.addInterval(rightSubspanILo, rightSubspanILo + rightSubspan.size(), rightHullPointIdx, ilo + maxPointIdx);
			data.addInterval(leftSubspanILo, leftSubspanILo + leftSubspan.size(), ilo + maxPointIdx, leftHullPointIdx);
			
			numNotCompacted += ptsSpan.size() - rightSubspan.size() - leftSubspan.size() - 1;
		}
		data.swapIntervals();
		return numNotCompacted;
	}
	
	template <qhPartitionStrategy S, typename T>
	void run_pointThresholdCompact(std::vector<point<T>>& pts, int compactPointThreshold) {
		int numNotCompacted = 0;
		
		Data<T> data;
		data.initialize(pts);
		
		while (!data.intervals.empty()) {
			numNotCompacted += runSingleStepWithoutCompaction<S>(pts, data);
			if (numNotCompacted > compactPointThreshold) {
				data.compactAndRemoveNotOnHull(pts);
				numNotCompacted = 0;
			}
		}
		if (numNotCompacted > 0) {
			removeNotOnHull(pts);
		}
	}
	
	template <qhPartitionStrategy S, typename T>
	void run_depthThresholdCompact(std::vector<point<T>>& pts, int compactDepthThreshold) {
		int numNotCompacted = 0;
		
		Data<T> data;
		data.initialize(pts);
		
		for (int depth = 0; depth < compactDepthThreshold && !data.intervals.empty(); depth++) {
			numNotCompacted += runSingleStepWithoutCompaction<S>(pts, data);
		}
		
		if (data.intervals.empty()) {
			if (numNotCompacted > 0)
				removeNotOnHull(pts);
			return;
		}
		
		data.intervalHullPoints.clear();
		if (numNotCompacted > 0) {
			data.compactAndRemoveNotOnHull(pts);
		}
		
		run_alwaysCompact<S>(pts, data);
	}
	
	template <qhPartitionStrategy S, typename T>
	void run(std::vector<point<T>>& pts) {
		if (implArgs == "N") {
			run_pointThresholdCompact<S>(pts, std::numeric_limits<int>::max());
		} else if (int pointsThresholdOpt = getImplArgInt("P").value_or(0)) {
			run_pointThresholdCompact<S>(pts, pointsThresholdOpt);
		} else {
			int depthThreshold = getImplArgInt("D").value_or(0);
			if (depthThreshold > 0) {
				run_depthThresholdCompact<S>(pts, depthThreshold);
			} else {
				Data<T> data;
				data.initialize(pts);
				run_alwaysCompact<S>(pts, data);
			}
		}
	}
}

DEF_HULL_IMPL({
	.name = "qh_bf_nxp",
	.runInt = qh_bf::run<qhPartitionStrategy::noPartitionByX, int64_t>,
	.runDouble = qh_bf::run<qhPartitionStrategy::noPartitionByX,double>,
});

DEF_HULL_IMPL({
	.name = "qh_bf_xp",
	.runInt = qh_bf::run<qhPartitionStrategy::firstPartitionByX, int64_t>,
	.runDouble = qh_bf::run<qhPartitionStrategy::firstPartitionByX,double>,
});

DEF_HULL_IMPL({
	.name = "qh_bf_ss",
	.runInt = qh_bf::run<qhPartitionStrategy::singleScan, int64_t>,
	.runDouble = qh_bf::run<qhPartitionStrategy::singleScan,double>,
});
