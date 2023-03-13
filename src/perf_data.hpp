#pragma once

#include <chrono>

struct PerfData {
	std::chrono::high_resolution_clock::time_point startTime;
	std::chrono::high_resolution_clock::time_point endTime;
	
	virtual ~PerfData() { }
	
	virtual void begin();
	virtual void end();
	
	virtual void printStatistics();
};
