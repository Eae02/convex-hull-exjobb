#pragma once

#include "../point.hpp"

#include <span>

template <typename T>
void solveOneMonotoneChain(std::span<point<T>> pts, point<T> leftHullPoint, point<T> rightHullPoint);
