#include "../hull_impl.hpp"
#include "../point.hpp"

#include <algorithm>
#include <vector>

template <typename T>
void runJarvisWrap(std::vector<point<T>>& pts) {
	auto itMinPoint = std::min_element(pts.begin(), pts.end());
	std::swap(*itMinPoint, pts.front());
	size_t numHullPts = 1;
	
	point<T> firstHullPoint = pts.front();
	point<T> lastHullPoint = firstHullPoint;
	while (numHullPts < pts.size()) {
		size_t nextHullPointIdx = numHullPts;
		for (size_t i = numHullPts + 1; i < pts.size(); i++) {
			auto side = pts[i].sideOfLine(lastHullPoint, pts[nextHullPointIdx]);
			if (side == side::right ||
				(side == side::on && (pts[i] - lastHullPoint).lenmh() > (pts[nextHullPointIdx] - lastHullPoint).lenmh())
				) {
				nextHullPointIdx = i;
			}
		}
		if (numHullPts > 1 && firstHullPoint.sideOfLine(lastHullPoint, pts[nextHullPointIdx]) != side::left)
			break;
		lastHullPoint = pts[nextHullPointIdx];
		std::swap(pts[nextHullPointIdx], pts[numHullPts]);
		numHullPts++;
	}
	
	pts.resize(numHullPts);
}

template <typename T>
size_t runJarvisWrapSOA(SOAPoints<T> pts) {
	size_t minPointIdx = pts.findMinIndex();
	std::swap(pts.x[minPointIdx], pts.x[0]);
	std::swap(pts.y[minPointIdx], pts.y[0]);
	size_t numHullPts = 1;
	
	point<T> firstHullPoint = pts[0];
	point<T> lastHullPoint = firstHullPoint;
	while (numHullPts < pts.size()) {
		size_t nextHullPointIdx = numHullPts;
		for (size_t i = numHullPts + 1; i < pts.size(); i++) {
			auto side = pts[i].sideOfLine(lastHullPoint, pts[nextHullPointIdx]);
			if (side == side::right ||
				(side == side::on && (pts[i] - lastHullPoint).lenmh() > (pts[nextHullPointIdx] - lastHullPoint).lenmh())
				) {
				nextHullPointIdx = i;
			}
		}
		if (numHullPts > 1 && firstHullPoint.sideOfLine(lastHullPoint, pts[nextHullPointIdx]) != side::left)
			break;
		lastHullPoint = pts[nextHullPointIdx];
		pts.swapPoints(nextHullPointIdx, numHullPts);
		numHullPts++;
	}
	
	return numHullPts;
}

void runJarvisWrapAvx(std::vector<pointd>& pts);
void runJarvisWrapAvx512(std::vector<pointd>& pts);

DEF_HULL_IMPL({
	.name = "jarvis_wrap",
	.runInt = &runJarvisWrap<int64_t>,
	.runDouble = &runJarvisWrap<double>,
});

DEF_HULL_IMPL({
	.name = "jarvis_wrap_soa",
	.runIntSoa = &runJarvisWrapSOA<int64_t>,
	.runDoubleSoa = &runJarvisWrapSOA<double>,
});

DEF_HULL_IMPL({
	.name = "jarvis_wrap_avx",
	.runInt = nullptr,
	.runDouble = &runJarvisWrapAvx,
});

DEF_HULL_IMPL({
	.name = "jarvis_wrap_avx512",
	.runInt = nullptr,
	.runDouble = &runJarvisWrapAvx512,
});
