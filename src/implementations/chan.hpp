#pragma once

#include <vector>
#include "../point.hpp"

template <typename T>
bool Hull2DMerge(const std::vector<std::vector<point<T>>>& Pdiv, int H, std::vector<point<T>>& result);
