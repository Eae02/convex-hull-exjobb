#include "../hull_impl.hpp"
#include "../point.hpp"
#include "quickhull_common.hpp"

#include <algorithm>
#include <vector>
#include <span>
#include <cmath>

template <qhPartitionStrategy S, typename T>
static void quickhullRec(std::span<point<T>> pts, point<T> leftHullPoint, point<T> rightHullPoint) {
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
	
	quickhullRec<S, T>(rightSubspan, maxPoint, rightHullPoint);
	quickhullRec<S, T>(leftSubspan, leftHullPoint, maxPoint);
}

template <qhPartitionStrategy S, typename T>
void runQuickhull(std::vector<point<T>>& pts) {
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
	
	quickhullRec<S, T>(std::span<point<T>>(&pts[1], &*belowPointsEndIt), rightmostPt, leftmostPt);
	quickhullRec<S, T>(std::span<point<T>>(&*belowPointsEndIt + 1, pts.data() + pts.size()), leftmostPt, rightmostPt);
	
	removeNotOnHull(pts);
}

#ifndef NO_AVX
void runQuickhullAvx2(std::vector<pointd>& pts);
void runQuickhullAvx512(std::vector<pointd>& pts);

DEF_HULL_IMPL({
	.name = "qh_avx",
	.runInt = nullptr,
	.runDouble = runQuickhullAvx2,
});

DEF_HULL_IMPL({
	.name = "qh_avx512",
	.runInt = nullptr,
	.runDouble = runQuickhullAvx512,
});
#endif

DEF_HULL_IMPL({
	.name = "qh_rec_nxp",
	.runInt = runQuickhull<qhPartitionStrategy::noPartitionByX, int64_t>,
	.runDouble = runQuickhull<qhPartitionStrategy::noPartitionByX, double>,
});

DEF_HULL_IMPL({
	.name = "qh_rec_xp",
	.runInt = runQuickhull<qhPartitionStrategy::firstPartitionByX, int64_t>,
	.runDouble = runQuickhull<qhPartitionStrategy::firstPartitionByX, double>,
});

DEF_HULL_IMPL({
	.name = "qh_rec_ss",
	.runInt = runQuickhull<qhPartitionStrategy::singleScan, int64_t>,
	.runDouble = runQuickhull<qhPartitionStrategy::singleScan, double>,
});
