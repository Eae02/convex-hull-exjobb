#include "../hull_impl.hpp"
#include "../point.hpp"

#include <algorithm>
#include <vector>
#include <span>
#include <cmath>

template <typename T>
void runQuickhullBF(std::vector<point<T>>& pts) {
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
	
	std::vector<std::pair<uint32_t, uint32_t>> intvs, intvs2;
	intvs.emplace_back(1, belowPointsEndIdx);
	intvs.emplace_back(belowPointsEndIdx + 1, pts.size());
	
	std::vector<point<T>> pointsLTmp;
	
	while (!intvs.empty()) {
		uint32_t nextOutIdx = intvs[0].first;
		
		for (size_t ii = 0; ii < intvs.size(); ii++) {
			auto [ilo, ihi] = intvs[ii];
			auto rightHullPoint = pts[ilo - 1];
			auto leftHullPoint = pts[ihi == pts.size() ? 0 : ihi];
			point<T> normal = (rightHullPoint - leftHullPoint).rotated90CCW();
			
			size_t maxPointIdx = 0;
			T maxPointDot = std::numeric_limits<T>::min();
			point<T> maxPoint;
			for (uint32_t i = ilo; i < ihi; i++) {
				T dotProduct = normal.dot(pts[i] - leftHullPoint);
				if (std::tie(dotProduct, pts[i]) > std::tie(maxPointDot, maxPoint)) {
					maxPointIdx = i;
					maxPointDot = dotProduct;
					maxPoint = pts[i];
				}
			}
			
			if (pts[maxPointIdx].sideOfLine(leftHullPoint, rightHullPoint) == side::left) {
				if (ihi == ilo + 1) {
					pts[nextOutIdx++] = pts[ilo];
				} else {
					pointsLTmp.clear();
					pointsLTmp.push_back(maxPoint);
					
					pts[maxPointIdx] = pts[ilo];
					ilo++;
					
					bool upperHull = leftHullPoint < rightHullPoint;
					
					auto lineDeltaR = maxPoint - rightHullPoint;
					auto lineDeltaL = leftHullPoint - maxPoint;
					
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
					
					if (rightIntvHi > rightIntvLo) {
						intvs2.emplace_back(rightIntvLo, rightIntvHi);
					}
					if (leftIntvHi > leftIntvLo) {
						intvs2.emplace_back(leftIntvLo, leftIntvHi);
					}
				}
			}
			
			uint32_t nextIntvLo = ii == intvs.size() - 1 ? pts.size() : intvs[ii + 1].first;
			if (nextOutIdx != ihi)
				std::copy(pts.begin() + ihi, pts.begin() + nextIntvLo, pts.begin() + nextOutIdx);
			nextOutIdx += nextIntvLo - ihi;
		}
		intvs2.swap(intvs);
		intvs2.clear();
		
		pts.resize(nextOutIdx);
	}
}

DEF_HULL_IMPL({
	.name = "qh_bf",
	.runInt = runQuickhullBF<int64_t>,
	.runDouble = runQuickhullBF<double>,
});
