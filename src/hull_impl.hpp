#pragma once

#include <string_view>
#include <vector>
#include <functional>

#include "point.hpp"

template <typename T>
using HullSolveFunction = std::function<void(std::vector<point<T>>&)>;

struct HullImpl {
	std::string_view name;
	HullSolveFunction<int64_t> runInt;
	HullSolveFunction<double> runDouble;
};

extern std::vector<HullImpl>* hullImplementations;

int _defHullImpl(HullImpl impl);

#define STR_CONCAT_IMPL(x, y) x##y
#define STR_CONCAT(x, y) STR_CONCAT_IMPL(x, y)

#define DEF_HULL_IMPL static int STR_CONCAT(_hullImpl_, __LINE__) = _defHullImpl
