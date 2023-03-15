#include "../hull_impl.hpp"
#include "../point.hpp"

#include <algorithm>
#include <vector>
#include <span>
#include <cmath>

template <typename T>
static void quickhullRec(std::span<point<T>> pts, point<T> leftHullPoint, point<T> rightHullPoint) {
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
	
	auto rightPointsEndIt = std::stable_partition(pts.begin(), pts.begin() + maxPointIdx, [&] (const point<T>& p) -> bool {
		return p.sideOfLine(rightHullPoint, maxPoint) == side::right;
	});

	auto leftPointsBeginIt = pts.begin() + maxPointIdx + 1;
	auto leftPointsEndIt = std::stable_partition(leftPointsBeginIt, pts.end(), [&] (const point<T>& p) -> bool {
		return p.sideOfLine(maxPoint, leftHullPoint) == side::right;
	});

	size_t numPointsRight = rightPointsEndIt - pts.begin();
	size_t numPointsLeft = leftPointsEndIt - leftPointsBeginIt;

	std::fill(rightPointsEndIt, pts.begin() + maxPointIdx, point<T>::notOnHull);
	std::fill(leftPointsEndIt, pts.end(), point<T>::notOnHull);

	quickhullRec<T>(pts.subspan(0, numPointsRight), maxPoint, rightHullPoint);
	quickhullRec<T>(pts.subspan(maxPointIdx + 1, numPointsLeft), leftHullPoint, maxPoint);
}

template <typename T>
void runQuickhullEarlySortX(std::vector<point<T>>& pts) {
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
	
	std::span<point<T>> belowSpan(&pts[1], &*belowPointsEndIt);
	std::span<point<T>> aboveSpan(&*belowPointsEndIt + 1, pts.data() + pts.size());
	
	sort(belowSpan.begin(), belowSpan.end(), [] (const auto& a, const auto& b) { return a.x < b.x; });
	sort(aboveSpan.begin(), aboveSpan.end(), [] (const auto& a, const auto& b) { return a.x > b.x; });
	
	quickhullRec<T>(belowSpan, rightmostPt, leftmostPt);
	quickhullRec<T>(aboveSpan, leftmostPt, rightmostPt);
	
	pts.erase(std::remove_if(pts.begin(), pts.end(), [&] (const point<T>& p) { return p.isNotOnHull(); }), pts.end());
}

DEF_HULL_IMPL({
	.name = "qh_rec_esx",
	.runInt = &runQuickhullEarlySortX<int64_t>,
	.runDouble = &runQuickhullEarlySortX<double>,
});