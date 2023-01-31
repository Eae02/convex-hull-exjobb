#include "../hull_impl.hpp"
#include "../point.hpp"

#include <algorithm>
#include <vector>
#include <span>
#include <cmath>
#include <thread>

template <typename T>
struct PointNotOnHull {
	static const point<T> value;
};
template <> const point<double> PointNotOnHull<double>::value = { NAN, NAN };
template <> const point<int64_t> PointNotOnHull<int64_t>::value = { INT64_MAX, INT64_MAX };

struct ParallelData {
	std::vector<std::thread> threads;
};

template <typename T>
void quickhullRec(
	std::span<point<T>> pts, point<T> leftHullPoint, point<T> rightHullPoint,
	int remParallelDepth, ParallelData* pdata
) {
	if (pts.size() <= 1)
		return;
	
	point<T> normal = (rightHullPoint - leftHullPoint).rotated90CCW();
	
	size_t maxPointIdx = 0;
	T maxPointDot = normal.dot(pts[0]);
	for (size_t i = 1; i < pts.size(); i++) {
		T dotProduct = normal.dot(pts[i]);
		if (dotProduct > maxPointDot) {
			maxPointIdx = i;
			maxPointDot = dotProduct;
		}
	}
	
	std::swap(pts[maxPointIdx], pts.back());
	auto maxPoint = pts.back();
	
	auto rightPointsEndIt = std::partition(pts.begin(), pts.end() - 1, [&] (const point<T>& p) -> bool {
		return p.sideOfLine(rightHullPoint, maxPoint) == side::right;
	});
	
	std::swap(*rightPointsEndIt, pts.back());
	
	auto leftPointsEndIt = std::partition(rightPointsEndIt + 1, pts.end(), [&] (const point<T>& p) -> bool {
		return p.sideOfLine(maxPoint, leftHullPoint) == side::right;
	});
	
	size_t numPointsRight = rightPointsEndIt - pts.begin();
	size_t numPointsLeft = (leftPointsEndIt - rightPointsEndIt) - 1;
	
	std::fill(leftPointsEndIt, pts.end(), PointNotOnHull<T>::value);
	
	auto rightSubspan = pts.subspan(0, numPointsRight);
	if (pdata && remParallelDepth > 0) {
		pdata->threads.emplace_back([=] {
			quickhullRec<T>(rightSubspan, maxPoint, rightHullPoint, remParallelDepth - 1, pdata);
		});
	} else {
		quickhullRec<T>(rightSubspan, maxPoint, rightHullPoint, remParallelDepth - 1, pdata);
	}
	
	quickhullRec<T>(pts.subspan(numPointsRight + 1, numPointsLeft), leftHullPoint, maxPoint, remParallelDepth - 1, pdata);
}

template <typename T>
void runQuickhull(std::vector<point<T>>& pts, bool parallel) {
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
	
	std::unique_ptr<ParallelData> pdata;
	int remParallelDepth = 0;
	
	if (parallel) {
		while ((1 << remParallelDepth) <= std::thread::hardware_concurrency())
			remParallelDepth++;
	}
	
	quickhullRec<T>(std::span<point<T>>(&pts[1], &*belowPointsEndIt), rightmostPt, leftmostPt, remParallelDepth, pdata.get());
	quickhullRec<T>(std::span<point<T>>(&*belowPointsEndIt + 1, pts.data() + pts.size()), leftmostPt, rightmostPt, remParallelDepth, pdata.get());
	
	if (pdata) {
		for (std::thread& thread : pdata->threads) {
			thread.join();
		}
	}
	
	pts.erase(std::remove_if(pts.begin(), pts.end(), [&] (const point<T>& p) { return std::isnan(p.x); }), pts.end());
}

DEF_HULL_IMPL({
	.name = "qh_rec",
	.runInt = std::bind(runQuickhull<int64_t>, std::placeholders::_1, false),
	.runDouble = std::bind(runQuickhull<double>, std::placeholders::_1, false),
});

DEF_HULL_IMPL({
	.name = "qh_recpar",
	.runInt = std::bind(runQuickhull<int64_t>, std::placeholders::_1, true),
	.runDouble = std::bind(runQuickhull<double>, std::placeholders::_1, true),
});
