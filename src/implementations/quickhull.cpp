#include "../hull_impl.hpp"
#include "../point.hpp"

#include <algorithm>
#include <vector>
#include <span>
#include <cmath>
#include <thread>
#include <list>
#include <mutex>

template <typename T>
struct PointNotOnHull {
	static const point<T> value;
};
template <> const point<double> PointNotOnHull<double>::value = { NAN, NAN };
template <> const point<int64_t> PointNotOnHull<int64_t>::value = { INT64_MAX, INT64_MAX };

struct ParallelData {
	std::mutex lock;
	std::list<std::thread> threads;
};

template <typename T>
void quickhullRec(
	std::span<point<T>> pts, point<T> leftHullPoint, point<T> rightHullPoint,
	int remParallelDepth, ParallelData* pdata, bool upperHull
) {
	if (pts.empty())
		return;
	
	point<T> normal = (rightHullPoint - leftHullPoint).rotated90CCW();
	
	size_t maxPointIdx = 0;
	T maxPointDot = normal.dot(pts[0]);
	point<T> maxPoint = pts[0];
	for (size_t i = 1; i < pts.size(); i++) {
		T dotProduct = normal.dot(pts[i]);
		if (std::tie(dotProduct, pts[i]) > std::tie(maxPointDot, maxPoint)) {
			maxPointIdx = i;
			maxPointDot = dotProduct;
			maxPoint = pts[i];
		}
	}
	
	if (pts[maxPointIdx].sideOfLine(leftHullPoint, rightHullPoint) != side::left) {
		std::fill(pts.begin(), pts.end(), PointNotOnHull<T>::value);
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
	std::fill(rightPointsValidEndIt, rightPointsEndIt, PointNotOnHull<T>::value);

	auto leftPointsEndIt = std::partition(rightPointsEndIt + 1, pts.end(), [&] (const point<T>& p) -> bool {
		return p.sideOfLine(maxPoint, leftHullPoint) == side::right;
	});

	size_t numPointsRight = rightPointsEndIt - pts.begin();
	size_t numPointsValidRight = rightPointsValidEndIt - pts.begin();
	size_t numPointsLeft = (leftPointsEndIt - rightPointsEndIt) - 1;
    
	std::fill(leftPointsEndIt, pts.end(), PointNotOnHull<T>::value);

	auto rightSubspan = pts.subspan(0, numPointsValidRight);
	if (pdata && remParallelDepth > 0) {
		pdata->lock.lock();
		pdata->threads.emplace_back([=] {
			quickhullRec<T>(rightSubspan, maxPoint, rightHullPoint, remParallelDepth - 1, pdata, upperHull);
		});
		pdata->lock.unlock();
	} else {
		quickhullRec<T>(rightSubspan, maxPoint, rightHullPoint, remParallelDepth - 1, pdata, upperHull);
	}
	
	quickhullRec<T>(pts.subspan(numPointsRight + 1, numPointsLeft), leftHullPoint, maxPoint, remParallelDepth - 1, pdata, upperHull);
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
		pdata = std::make_unique<ParallelData>();
		while ((1 << remParallelDepth) <= static_cast<int>(std::thread::hardware_concurrency()))
			remParallelDepth++;
	}
	
	quickhullRec<T>(std::span<point<T>>(&pts[1], &*belowPointsEndIt), rightmostPt, leftmostPt, remParallelDepth, pdata.get(), false);
	quickhullRec<T>(std::span<point<T>>(&*belowPointsEndIt + 1, pts.data() + pts.size()), leftmostPt, rightmostPt, remParallelDepth, pdata.get(), true);
	
	if (pdata) {
		while (true) {
			std::unique_lock<std::mutex> lg(pdata->lock);
			if (pdata->threads.empty()) break;
			auto thread = std::move(pdata->threads.back());
			pdata->threads.pop_back();
			lg.unlock();
			thread.join();
		}
	}
	
	pts.erase(std::remove_if(pts.begin(), pts.end(), [&] (const point<T>& p) {
		if constexpr (std::is_floating_point_v<T>) {
			return std::isnan(p.x);
		} else {
			return p == PointNotOnHull<T>::value;
		}
	}), pts.end());
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
