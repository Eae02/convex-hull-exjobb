#include "../../hull_impl.hpp"
#include "../../point.hpp"

#include <algorithm>

template <typename T>
void runXSort(std::vector<point<T>>& pts) {
    std::sort(pts.begin(), pts.end(), [] (point<T> a, point<T> b) {return a.x < b.x;});
}

template <typename T>
void runLexSort(std::vector<point<T>>& pts) {
    std::sort(pts.begin(), pts.end(), [] (point<T> a, point<T> b) {return a < b;});
}

template <typename T>
void runAngleSort(std::vector<point<T>>& pts) {
    size_t XminIdx = std::min_element(pts.begin(), pts.end()) - pts.begin();
    point<T> o = pts[XminIdx];
    std::sort(pts.begin(), pts.end(), [&o] (point<T> a, point<T> b) {side turn = a.sideOfLine(a,b); if (turn == side::on) {return a.lenmh() < b.lenmh(); } else return turn == side::right;});
}

DEF_HULL_IMPL({
	.name = "bench_xsort",
	.runInt = &runXSort<int64_t>,
	.runDouble = &runXSort<double>,
});

DEF_HULL_IMPL({
	.name = "bench_lexsort",
	.runInt = &runLexSort<int64_t>,
	.runDouble = &runLexSort<double>,
});

DEF_HULL_IMPL({
	.name = "bench_anglesort",
	.runInt = &runAngleSort<int64_t>,
	.runDouble = &runAngleSort<double>,
});

