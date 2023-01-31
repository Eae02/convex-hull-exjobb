#include "impl1.hpp"
#include "../hull_impl.hpp"
#include "../point.hpp"

#include <algorithm>
#include <vector>

template <typename T>
void runImpl1(std::vector<point<T>>& pts) {
	if (pts.size() <= 1) return;
	std::sort(pts.begin(), pts.end());
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
	.runInt = &runImpl1<int64_t>,
	.runDouble = &runImpl1<double>,
});
