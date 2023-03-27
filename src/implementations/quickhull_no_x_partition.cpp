#include "../hull_impl.hpp"
#include "../point.hpp"

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
};

template <typename T>
void quickhullRecNoXPartition(std::span<point<T>> pts, point<T> leftHullPoint, point<T> rightHullPoint) {
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
	
	std::swap(pts[maxPointIdx], pts.back());
	
	auto rightPointsEndIt = std::partition(pts.begin(), pts.end() - 1, [&] (const point<T>& p) -> bool {
		return p.sideOfLine(rightHullPoint, maxPoint) == side::right;
	});

	std::swap(*rightPointsEndIt, pts.back());
	
	auto leftPointsEndIt = std::partition(rightPointsEndIt + 1, pts.end(), [&] (const point<T>& p) -> bool {
		return p.sideOfLine(maxPoint, leftHullPoint) == side::right;
	});

	size_t numPointsRight = rightPointsEndIt - pts.begin();
	size_t numPointsLeft = (leftPointsEndIt - rightPointsEndIt) - 1;

	std::fill(leftPointsEndIt, pts.end(), point<T>::notOnHull);

	quickhullRecNoXPartition<T>(pts.subspan(0, numPointsRight), maxPoint, rightHullPoint);
	quickhullRecNoXPartition<T>(pts.subspan(numPointsRight + 1, numPointsLeft), leftHullPoint, maxPoint);
}

template <typename T>
void runQuickhullNoXPartition(std::vector<point<T>>& pts) {
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
	
	quickhullRecNoXPartition<T>(std::span<point<T>>(&pts[1], &*belowPointsEndIt), rightmostPt, leftmostPt);
	quickhullRecNoXPartition<T>(std::span<point<T>>(&*belowPointsEndIt + 1, pts.data() + pts.size()), leftmostPt, rightmostPt);
	
	removeNotOnHull(pts);
}

DEF_HULL_IMPL({
	.name = "qh_rec_nxp",
	.runInt = &runQuickhullNoXPartition<int64_t>,
	.runDouble = &runQuickhullNoXPartition<double>,
});
