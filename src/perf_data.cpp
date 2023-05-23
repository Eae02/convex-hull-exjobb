#include "perf_data.hpp"

#include <iostream>

void PerfData::begin() {
	startTime = std::chrono::high_resolution_clock::now();
}

void PerfData::end() {
	endTime = std::chrono::high_resolution_clock::now();
}

void printIntermediateTimes(std::chrono::high_resolution_clock::time_point startTime);

void PerfData::printStatistics() {
	std::cerr << "compute time: " << std::chrono::duration<double, std::milli>(endTime - startTime).count() << " ms\n";
	printIntermediateTimes(startTime);
}
