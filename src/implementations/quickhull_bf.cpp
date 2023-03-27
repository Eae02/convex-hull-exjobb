#include "../hull_impl.hpp"
#include "../point.hpp"

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
	std::pair<size_t, point<T>> findFurthestPoint(
		std::span<const point<T>> pts,
		point<T> rightHullPoint,
		point<T> leftHullPoint
	) {
		point<T> normal = (rightHullPoint - leftHullPoint).rotated90CCW();
		size_t maxPointIdx = 0;
		T maxPointDot = std::numeric_limits<T>::min();
		point<T> maxPoint;
		for (size_t i = 0; i < pts.size(); i++) {
			T dotProduct = normal.dot(pts[i] - leftHullPoint);
			if (std::tie(dotProduct, pts[i]) > std::tie(maxPointDot, maxPoint)) {
				maxPointIdx = i;
				maxPointDot = dotProduct;
				maxPoint = pts[i];
			}
		}
		return { maxPointIdx, maxPoint };
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
	
	template <typename T>
	void run_alwaysCompact(std::vector<point<T>>& pts, Data<T>& data) {
		std::vector<point<T>> pointsLTmp;
		while (!data.intervals.empty()) {
			uint32_t nextOutIdx = data.intervals[0].first;
			
			for (size_t ii = 0; ii < data.intervals.size(); ii++) {
				auto [ilo, ihi] = data.intervals[ii];
				auto [rightHullPointIdx, leftHullPointIdx] = getImplicitHullPointsForInterval(ilo, ihi, pts.size());
				auto rightHullPoint = pts[rightHullPointIdx];
				auto leftHullPoint = pts[leftHullPointIdx];
				
				auto [maxPointIdx, maxPoint] = findFurthestPoint<T>(
					std::span<point<T>>(pts.data() + ilo, pts.data() + ihi),
					rightHullPoint, leftHullPoint);
				maxPointIdx += ilo;
				
				if (pts[maxPointIdx].sideOfLine(leftHullPoint, rightHullPoint) == side::left) {
					if (ihi == ilo + 1) {
						pts[nextOutIdx++] = pts[ilo];
					} else {
						pointsLTmp.clear();
						pointsLTmp.push_back(maxPoint);
						
						pts[maxPointIdx] = pts[ilo];
						ilo++;
						
						bool upperHull = leftHullPoint < rightHullPoint;
						
						point<T> lineDeltaR = maxPoint - rightHullPoint;
						point<T> lineDeltaL = leftHullPoint - maxPoint;
						
						uint32_t rightIntvLo = nextOutIdx;
						for (uint32_t i = ilo; i < ihi; i++) {
							bool right = (pts[i].x < maxPoint.x) ^ upperHull;
							if ((pts[i] - maxPoint).cross(right ? lineDeltaR : lineDeltaL) > 0) {
								if (right)
									pts[nextOutIdx++] = pts[i];
								else
									pointsLTmp.push_back(pts[i]);
							}
						}
						uint32_t rightIntvHi = nextOutIdx;
						
						std::copy(pointsLTmp.begin(), pointsLTmp.end(), pts.begin() + nextOutIdx);
						uint32_t leftIntvLo = nextOutIdx + 1;
						nextOutIdx += pointsLTmp.size();
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
	
	template <typename T>
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
			
			auto [maxPointIdx, maxPoint] = findFurthestPoint<T>(
				std::span<point<T>>(pts.data() + ilo, pts.data() + ihi),
				rightHullPoint, leftHullPoint);
			maxPointIdx += ilo;
			
			//If the max point is not outside the line between the left and right hull point,
			// remove all points in this interval
			if (pts[maxPointIdx].sideOfLine(leftHullPoint, rightHullPoint) != side::left) {
				numNotCompacted += ihi - ilo;
				std::fill(pts.begin() + ilo, pts.begin() + ihi, point<T>::notOnHull);
				continue;
			}
			
			//If there is only a single point in this interval, keep this point as a hull point
			if (ihi == ilo + 1)
				continue;
			
			bool upperHull = leftHullPoint < rightHullPoint;
			
			std::swap(pts[maxPointIdx], pts[ihi - 1]);
			
			auto rightPointsEndIt = std::partition(pts.begin() + ilo, pts.begin() + ihi - 1, [&] (const point<T>& p) -> bool {
				return (p.x < maxPoint.x) ^ upperHull;
			});
			auto rightPointsValidEndIt = std::remove_if(pts.begin() + ilo, rightPointsEndIt, [&] (const point<T>& p) -> bool {
				return p.sideOfLine(rightHullPoint, maxPoint) != side::right;
			});
			std::swap(*rightPointsEndIt, pts[ihi - 1]);
			uint32_t newMaxPointIdx = rightPointsEndIt - pts.begin();
			std::fill(rightPointsValidEndIt, rightPointsEndIt, point<T>::notOnHull);
			
			auto leftPointsEndIt = std::remove_if(rightPointsEndIt + 1, pts.begin() + ihi, [&] (const point<T>& p) -> bool {
				return p.sideOfLine(maxPoint, leftHullPoint) != side::right;
			});
			std::fill(leftPointsEndIt, pts.begin() + ihi, point<T>::notOnHull);
			
			uint32_t pointsRightHi = rightPointsValidEndIt - pts.begin();
			uint32_t pointsLeftLo = newMaxPointIdx + 1;
			uint32_t pointsLeftHi = leftPointsEndIt - pts.begin();
			
			data.addInterval(ilo, pointsRightHi, rightHullPointIdx, newMaxPointIdx);
			data.addInterval(pointsLeftLo, pointsLeftHi, newMaxPointIdx, leftHullPointIdx);
			
			numNotCompacted += ihi - pointsRightHi - (pointsLeftHi - pointsLeftLo) - 1;
		}
		data.swapIntervals();
		return numNotCompacted;
	}
	
	template <typename T>
	void run_pointThresholdCompact(std::vector<point<T>>& pts, int compactPointThreshold) {
		int numNotCompacted = 0;
		
		Data<T> data;
		data.initialize(pts);
		
		while (!data.intervals.empty()) {
			numNotCompacted += runSingleStepWithoutCompaction(pts, data);
			if (numNotCompacted > compactPointThreshold) {
				data.compactAndRemoveNotOnHull(pts);
				numNotCompacted = 0;
			}
		}
		if (numNotCompacted > 0) {
			removeNotOnHull(pts);
		}
	}
	
	template <typename T>
	void run_depthThresholdCompact(std::vector<point<T>>& pts, int compactDepthThreshold) {
		int numNotCompacted = 0;
		
		Data<T> data;
		data.initialize(pts);
		
		for (int depth = 0; depth < compactDepthThreshold && !data.intervals.empty(); depth++) {
			numNotCompacted += runSingleStepWithoutCompaction(pts, data);
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
		
		run_alwaysCompact(pts, data);
	}
	
	template <typename T>
	void run(std::vector<point<T>>& pts) {
		if (implArgs == "N") {
			run_pointThresholdCompact(pts, std::numeric_limits<int>::max());
		} else if (auto pointsThresholdOpt = getImplArgInt("P")) {
			run_pointThresholdCompact(pts, std::max(*pointsThresholdOpt, 1));
		} else {
			int depthThreshold = getImplArgInt("D").value_or(0);
			if (depthThreshold > 0) {
				run_depthThresholdCompact(pts, depthThreshold);
			} else {
				Data<T> data;
				data.initialize(pts);
				run_alwaysCompact(pts, data);
			}
		}
	}
}

DEF_HULL_IMPL({
	.name = "qh_bf",
	.runInt = qh_bf::run<int64_t>,
	.runDouble = qh_bf::run<double>,
});
