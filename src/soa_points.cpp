#include "soa_points.hpp"

template <typename T>
size_t SOAPoints<T>::findMinIndex() const {
	size_t minPointIdx = 0;
	T minPointX = x[0];
	for (size_t i = 1; i < size(); i++) {
		if (x[i] < minPointX) {
			minPointIdx = i;
			minPointX = x[i];
		} else if (x[i] == minPointX && y[i] < y[minPointIdx]) {
			minPointIdx = i;
		}
	}
	return minPointIdx;
}

template <typename T>
size_t SOAPoints<T>::findMaxIndex() const {
	size_t maxPointIdx = 0;
	T maxPointX = x[0];
	for (size_t i = 1; i < size(); i++) {
		if (x[i] > maxPointX) {
			maxPointIdx = i;
			maxPointX = x[i];
		} else if (x[i] == maxPointX && y[i] > y[maxPointIdx]) {
			maxPointIdx = i;
		}
	}
	return maxPointIdx;
}

template struct SOAPoints<int64_t>;
template struct SOAPoints<double>;
