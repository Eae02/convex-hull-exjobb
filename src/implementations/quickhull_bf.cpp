#include "../hull_impl.hpp"
#include "../point.hpp"

#include <algorithm>
#include <vector>
#include <span>
#include <cmath>

namespace qh_bf {
	struct Interval_ImplicitHullPoints {
		uint32_t lo;
		uint32_t hi;
		
		Interval_ImplicitHullPoints(uint32_t _lo, uint32_t _hi)
			: lo(_lo), hi(_hi) { }
		Interval_ImplicitHullPoints(uint32_t _lo, uint32_t _hi, uint32_t, uint32_t)
			: lo(_lo), hi(_hi) { }
		
		template <typename T>
		std::pair<point<T>, point<T>> getHullPoints(const std::vector<point<T>>& points) const {
			return { points[lo - 1], points[hi == points.size() ? 0 : hi] };
		}
	};
	
	struct Interval_ExplicitHullPoints {
		uint32_t lo;
		uint32_t hi;
		uint32_t hullPointR;
		uint32_t hullPointL;
		
		Interval_ExplicitHullPoints(uint32_t _lo, uint32_t _hi, uint32_t _hullPointR, uint32_t _hullPointL)
			: lo(_lo), hi(_hi), hullPointR(_hullPointR), hullPointL(_hullPointL) { }
		
		template <typename T>
		std::pair<point<T>, point<T>> getHullPoints(const std::vector<point<T>>& points) const {
			return { points[hullPointR], points[hullPointL] };
		}
	};
	
	template <typename T, typename Interval>
	struct Data {
		std::vector<Interval> intervals;
		std::vector<Interval> intervalsNext;
		
		void initialize(std::vector<point<T>>& pts);
		
		std::pair<size_t, point<T>> findFurthestPoint(
			std::span<const point<T>> pts, point<T> rightHullPoint, point<T> leftHullPoint) const;
		
		void addInterval(Interval interval) {
			if (interval.hi > interval.lo)
				intervalsNext.push_back(interval);
		}
		
		void swapIntervals() {
			intervalsNext.swap(intervals);
			intervalsNext.clear();
		}
	};
	
	template <typename T, typename Interval>
	void Data<T, Interval>::initialize(std::vector<point<T>>& pts) {
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
		
		addInterval(Interval(1, belowPointsEndIdx, 0, belowPointsEndIdx));
		addInterval(Interval(belowPointsEndIdx + 1, pts.size(), belowPointsEndIdx, 0));
		swapIntervals();
	}
	
	template <typename T, typename Interval>
	std::pair<size_t, point<T>> Data<T, Interval>::findFurthestPoint(
		std::span<const point<T>> pts,
		point<T> rightHullPoint,
		point<T> leftHullPoint
	) const {
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
	void run_alwaysCompact(std::vector<point<T>>& pts) {
		Data<T, Interval_ImplicitHullPoints> data;
		data.initialize(pts);
		
		std::vector<point<T>> pointsLTmp;
		
		while (!data.intervals.empty()) {
			uint32_t nextOutIdx = data.intervals[0].lo;
			
			for (size_t ii = 0; ii < data.intervals.size(); ii++) {
				uint32_t ilo = data.intervals[ii].lo;
				uint32_t ihi = data.intervals[ii].hi;
				auto [rightHullPoint, leftHullPoint] = data.intervals[ii].getHullPoints(pts);
				auto [maxPointIdx, maxPoint] = data.findFurthestPoint(
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
						
						data.addInterval({ rightIntvLo, rightIntvHi });
						data.addInterval({ leftIntvLo, leftIntvHi });
					}
				}
				
				uint32_t nextIntvLo = ii == data.intervals.size() - 1 ? pts.size() : data.intervals[ii + 1].lo;
				if (nextOutIdx != ihi)
					std::copy(pts.begin() + ihi, pts.begin() + nextIntvLo, pts.begin() + nextOutIdx);
				nextOutIdx += nextIntvLo - ihi;
			}
			data.swapIntervals();
			pts.resize(nextOutIdx);
		}
	}
	
	template <typename T, typename ShouldCompactFn>
	void run_sometimesCompact(std::vector<point<T>>& pts, ShouldCompactFn shouldCompact) {
		std::vector<uint32_t> numRemovedBefore;
		
		int numNotCompacted = 0;
		int depth = 0;
		
		Data<T, Interval_ExplicitHullPoints> data;
		data.initialize(pts);
		
		while (!data.intervals.empty()) {
			for (const Interval_ExplicitHullPoints& interval : data.intervals) {
				uint32_t ilo = interval.lo;
				uint32_t ihi = interval.hi;
				std::span<point<T>> iptsSpan(pts.data() + ilo, pts.data() + ihi);
				auto [rightHullPoint, leftHullPoint] = interval.getHullPoints(pts);
				auto [maxPointIdx, maxPoint] = data.findFurthestPoint(iptsSpan, rightHullPoint, leftHullPoint);
				
				//If the max point is not outside the line between the left and right hull point,
				// remove all points in this interval
				if (iptsSpan[maxPointIdx].sideOfLine(leftHullPoint, rightHullPoint) != side::left) {
					numNotCompacted += iptsSpan.size();
					std::fill(iptsSpan.begin(), iptsSpan.end(), point<T>::notOnHull);
					continue;
				}
				
				//If there is only a single point in this interval, keep this point as a hull point
				if (ihi == ilo + 1)
					continue;
				
				bool upperHull = leftHullPoint < rightHullPoint;
				
				point<T> lineDeltaR = maxPoint - rightHullPoint;
				point<T> lineDeltaL = leftHullPoint - maxPoint;
				
				//Remove the max point from the range by replacing it with the first point
				iptsSpan[maxPointIdx] = iptsSpan[0];
				uint32_t rightPointsEnd = 1;
				uint32_t leftPointsBegin = iptsSpan.size();
				
				//Partitions points so that:
				// * Points to the right of the max point and outside the line between rightHullPoint and maxPoint are moved to the start
				// * Points to the left of the max point and outside the line between maxPoint and leftHullPoint are moved to the end
				// * Other points (at the center of the interval) are set to nan
				for (uint32_t i = 1; i < leftPointsBegin;) {
					bool right = (iptsSpan[i].x < maxPoint.x) ^ upperHull;
					if ((iptsSpan[i] - maxPoint).cross(right ? lineDeltaR : lineDeltaL) > 0) {
						if (right)
							std::swap(iptsSpan[rightPointsEnd++], iptsSpan[i++]);
						else
							std::swap(iptsSpan[--leftPointsBegin], iptsSpan[i]);
					} else {
						iptsSpan[i++] = point<T>::notOnHull;
						numNotCompacted++;
					}
				}
				
				//Moves the last point in the right interval to the first point (that was previously removed),
				// and places the max point in the correct position between the two intervals.
				rightPointsEnd--;
				iptsSpan[0] = iptsSpan[rightPointsEnd];
				iptsSpan[rightPointsEnd] = maxPoint;
				
				rightPointsEnd += ilo;
				leftPointsBegin += ilo;
				data.addInterval({ ilo, rightPointsEnd, interval.hullPointR, rightPointsEnd });
				data.addInterval({ leftPointsBegin, ihi, rightPointsEnd, interval.hullPointL });
			}
			data.swapIntervals();
			
			if (shouldCompact(numNotCompacted, depth) && numNotCompacted > 0) {
				uint32_t numKept = 0;
				while (!pts[numKept].isNotOnHull())
					numKept++;
				
				if (numRemovedBefore.empty()) {
					numRemovedBefore.resize(pts.size() + 1, 0);
				} else {
					std::fill_n(numRemovedBefore.begin(), numKept + 1, 0);
				}
				
				for (uint32_t i = numKept + 1; i < pts.size(); i++) {
					numRemovedBefore[i] = i - numKept;
					if (!pts[i].isNotOnHull())
						pts[numKept++] = pts[i];
				}
				numRemovedBefore[pts.size()] = pts.size() - numKept;
				
				pts.resize(numKept);
				
				for (Interval_ExplicitHullPoints& interval : data.intervals) {
					interval.lo -= numRemovedBefore[interval.lo];
					interval.hi -= numRemovedBefore[interval.hi];
					interval.hullPointR -= numRemovedBefore[interval.hullPointR];
					interval.hullPointL -= numRemovedBefore[interval.hullPointL];
				}
				
				numNotCompacted = 0;
			}
			
			depth++;
		}
		if (numNotCompacted) {
			removeNotOnHull(pts);
		}
	}
	
	template <typename T>
	void run(std::vector<point<T>>& pts) {
		if (implArgs == "A" || implArgs == "") {
			run_alwaysCompact(pts);
		} else if (implArgs == "N") {
			run_sometimesCompact(pts, [] (int, int) { return false; });
		} else if (auto pointsThresholdOpt = getImplArgInt("P")) {
			run_sometimesCompact(pts, [&] (int numNotCompacted, int) { return numNotCompacted >= *pointsThresholdOpt; });
		} else if (auto depthThresholdGOpt = getImplArgInt("D>")) {
			run_sometimesCompact(pts, [&] (int, int depth) { return depth > *depthThresholdGOpt; });
		} else if (auto depthThresholdLOpt = getImplArgInt("D<")) {
			run_sometimesCompact(pts, [&] (int, int depth) { return depth < *depthThresholdLOpt; });
		}
	}
}

DEF_HULL_IMPL({
	.name = "qh_bf",
	.runInt = qh_bf::run<int64_t>,
	.runDouble = qh_bf::run<double>,
});
