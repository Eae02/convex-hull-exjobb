#include "impl1.hpp"
#include "../hull_impl.hpp"
#include "../point.hpp"

#include <algorithm>
#include <vector>
#include <execution>

template <typename T>
void runImpl1(std::vector<point<T>>& pts, bool parallelSort) {
	if (pts.size() <= 1)
		return;
	
	if (parallelSort) {
		std::sort(std::execution::par, pts.begin(), pts.end());
	} else {
		std::sort(pts.begin(), pts.end());
	}
	
	std::vector<point<T>> h;
	auto f = [&] (size_t s) {
		for (auto p : pts) {
			while (h.size() >= s + 2 && h.back().cross(p, h[h.size() - 2]) <= 0)
				h.pop_back();
			h.push_back(p);
		}
		h.pop_back();
	};
	f(0);
	reverse(pts.begin(), pts.end());
	f(h.size());
	if (h.size() == 2 && h[0] == h[1]) h.pop_back();
	pts = h;
}

DEF_HULL_IMPL({
	.name = "impl1",
	.runInt = std::bind(&runImpl1<int64_t>, std::placeholders::_1, false),
	.runDouble = std::bind(&runImpl1<double>, std::placeholders::_1, false),
});

DEF_HULL_IMPL({
	.name = "impl1_par",
	.runInt = std::bind(&runImpl1<int64_t>, std::placeholders::_1, true),
	.runDouble = std::bind(&runImpl1<double>, std::placeholders::_1, true),
});
