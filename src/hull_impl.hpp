#pragma once

#include <string_view>
#include <vector>
#include <functional>
#include <optional>

#include "point.hpp"
#include "soa_points.hpp"

template <typename T>
using HullSolveFunction = std::function<void(std::vector<point<T>>&)>;

template <typename T>
using HullSolveFunctionSOA = std::function<size_t(SOAPoints<T>)>;

struct HullImpl {
	std::string_view name;
	HullSolveFunction<int64_t> runInt;
	HullSolveFunction<double> runDouble;
	HullSolveFunctionSOA<int64_t> runIntSoa;
	HullSolveFunctionSOA<double> runDoubleSoa;
	size_t soaAlignment;
};

extern std::vector<HullImpl>* hullImplementations;

extern std::string_view implArgs;

std::optional<int> getImplArgInt(std::string_view argPrefix);

int _defHullImpl(HullImpl impl);

#define STR_CONCAT_IMPL(x, y) x##y
#define STR_CONCAT(x, y) STR_CONCAT_IMPL(x, y)

#define DEF_HULL_IMPL static int STR_CONCAT(_hullImpl_, __LINE__) = _defHullImpl
