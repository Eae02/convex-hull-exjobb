#include "perf_data.hpp"

#include <iostream>

void PerfData::begin() {
	startTime = std::chrono::high_resolution_clock::now();
}

void PerfData::end() {
	endTime = std::chrono::high_resolution_clock::now();
}

void PerfData::printStatistics() {
	std::cerr << "compute time: " << std::chrono::duration<double, std::milli>(endTime - startTime).count() << "ms\n";
}
