#pragma once

#include <string_view>
#include <vector>
#include <functional>

#include "point.hpp"

struct HullImpl {
	std::string_view name;
	std::function<void(std::vector<pointi>&)> runInt;
	std::function<void(std::vector<pointd>&)> runDouble;
};

extern std::vector<HullImpl>* hullImplementations;

int _defHullImpl(HullImpl impl);

#define STR_CONCAT_IMPL(x, y) x##y
#define STR_CONCAT(x, y) STR_CONCAT_IMPL(x, y)

#define DEF_HULL_IMPL static int STR_CONCAT(_hullImpl_, __LINE__) = _defHullImpl
