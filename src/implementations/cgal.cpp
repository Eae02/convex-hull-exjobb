#include "../hull_impl.hpp"
#include "../point.hpp"

#include <algorithm>
#include <vector>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/ch_jarvis.h>
#include <CGAL/ch_graham_andrew.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point_2;

static_assert(std::is_same_v<std::decay_t<decltype(Point_2().x())>, double>);
static_assert(sizeof(pointd) == sizeof(Point_2));

template <typename InIterator = const Point_2*, typename OutIterator = std::insert_iterator<std::vector<Point_2>>>
using CGAL_CH_Func = OutIterator(*)(InIterator, InIterator, OutIterator);

std::function<void(std::vector<pointd>&)> createCGALRunFunction(CGAL_CH_Func<> cgalFunc) {
	return [=] (std::vector<pointd>& pts) {
		auto ptsPointer = reinterpret_cast<const Point_2*>(pts.data());
		
		std::vector<Point_2> output;
		cgalFunc(ptsPointer, ptsPointer + pts.size(), std::inserter(output, output.end()));
		
		pts.clear();
		for (const auto& p : output)
			pts.emplace_back(p.x(), p.y());
	};
}

DEF_HULL_IMPL({
	.name = "cgal_jarvis",
	.runInt = nullptr,
	.runDouble = createCGALRunFunction(&CGAL::ch_jarvis)
});

DEF_HULL_IMPL({
	.name = "cgal_graham",
	.runInt = nullptr,
	.runDouble = createCGALRunFunction(&CGAL::ch_graham_andrew)
});
