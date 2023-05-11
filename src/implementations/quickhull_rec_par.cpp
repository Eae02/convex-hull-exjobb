#include "../hull_impl.hpp"
#include "../point.hpp"
#include "quickhull_common.hpp"

#include <algorithm>
#include <vector>
#include <span>
#include <cmath>
#include <thread>
#include <list>
#include <mutex>

struct ParallelData {
	std::mutex lock;
	std::list<std::thread> threads;
	
	template <typename CB>
	void maybeRunInParallel(bool parallel, CB callback) {
		if (parallel) {
			lock.lock();
			threads.emplace_back(callback);
			lock.unlock();
		} else {
			callback();
		}
	}
};

template <qhPartitionStrategy S, typename T>
void quickhullRecPar(
	std::span<point<T>> pts, point<T> leftHullPoint, point<T> rightHullPoint,
	int remParallelDepth, ParallelData& pdata
) {
	if (pts.empty())
		return;
	
	size_t maxPointIdx = findFurthestPointFromLine<T>(pts, leftHullPoint, rightHullPoint);
	point<T> maxPoint = pts[maxPointIdx];
	
	if (pts[maxPointIdx].sideOfLine(leftHullPoint, rightHullPoint) != side::left) {
		std::fill(pts.begin(), pts.end(), point<T>::notOnHull);
		return;
	}
	
	if (pts.size() == 1)
		return;
	
	auto [rightSubspan, leftSubspan] = quickhullPartitionPoints<S, T>(pts, leftHullPoint, rightHullPoint, maxPointIdx);
	
	auto pdataPtr = &pdata;
	pdata.maybeRunInParallel(remParallelDepth > 0, [=] {
		quickhullRecPar<S, T>(rightSubspan, maxPoint, rightHullPoint, remParallelDepth - 1, *pdataPtr);
	});
	
	quickhullRecPar<S, T>(leftSubspan, leftHullPoint, maxPoint, remParallelDepth - 1, pdata);
}

template <qhPartitionStrategy S, typename T>
void runQuickhullPar(std::vector<point<T>>& pts) {
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
	
	ParallelData pdata;
	
	int maxThreads = std::max(1, getImplArgInt("T").value_or(static_cast<int>(std::thread::hardware_concurrency())));
	int remParallelDepth = floor(log2(static_cast<double>(maxThreads)));
	
	pdata.maybeRunInParallel(remParallelDepth > 0, [&] {
		quickhullRecPar<S, T>(std::span<point<T>>(&pts[1], &*belowPointsEndIt), rightmostPt, leftmostPt, remParallelDepth - 1, pdata);
	});
	
	quickhullRecPar<S, T>(std::span<point<T>>(&*belowPointsEndIt + 1, pts.data() + pts.size()), leftmostPt, rightmostPt, remParallelDepth - 1, pdata);
	
	while (true) {
		std::unique_lock<std::mutex> lg(pdata.lock);
		if (pdata.threads.empty()) break;
		auto thread = std::move(pdata.threads.back());
		pdata.threads.pop_back();
		lg.unlock();
		thread.join();
	}
	
	removeNotOnHull(pts);
}

DEF_HULL_IMPL({
	.name = "qh_recpar_xp",
	.runInt = runQuickhullPar<qhPartitionStrategy::firstPartitionByX, int64_t>,
	.runDouble = runQuickhullPar<qhPartitionStrategy::firstPartitionByX, double>,
});

DEF_HULL_IMPL({
	.name = "qh_recpar_nxp",
	.runInt = runQuickhullPar<qhPartitionStrategy::noPartitionByX, int64_t>,
	.runDouble = runQuickhullPar<qhPartitionStrategy::noPartitionByX, double>,
});
