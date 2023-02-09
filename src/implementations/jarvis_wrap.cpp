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

DEF_HULL_IMPL({
	.name = "jarvis_wrap",
	.runInt = &runJarvisWrap<int64_t>,
	.runDouble = &runJarvisWrap<double>,
});
