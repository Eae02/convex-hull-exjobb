#pragma once

#include <span>

#include "point.hpp"

template <typename T>
struct SOAPoints {
	std::span<T> x;
	std::span<T> y;
	
	size_t size() const { return x.size(); }
	
	SOAPoints<T> subspan(size_t offset, size_t count = std::dynamic_extent) const {
		return { x.subspan(offset, count), y.subspan(offset, count) };
	}
	
	point<T> operator[](size_t i) const {
		return { x[i], y[i] };
	}
	
	void swapPoints(size_t i, size_t j) {
		std::swap(x[i], x[j]);
		std::swap(y[i], y[j]);
	}
	
	template <typename F>
	size_t partition(F f) {
		size_t numTrue = 0;
		while (numTrue < size() && f(numTrue))
			numTrue++;
		for (size_t i = numTrue + 1; i < size(); i++) {
			if (f(i))
				swapPoints(i, numTrue++);
		}
		return numTrue;
	}
	
	size_t findMinIndex() const;
	size_t findMaxIndex() const;
};
