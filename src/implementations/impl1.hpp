#pragma once

#include <vector>
#include "../point.hpp"

template <typename T>
void runImpl1(std::vector<point<T>>& pts, bool parallelSort = false);
