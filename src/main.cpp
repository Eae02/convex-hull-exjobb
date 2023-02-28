#include "hull_impl.hpp"
#include "slice_parallel.hpp"

#include <iostream>
#include <algorithm>
#include <string_view>
#include <chrono>
#include <cassert>
#include <charconv>
#include <iomanip>
#include <cmath>
#include <memory>
#include <span>
#include <optional>

static bool readBinary;
static bool outputPoints;

static std::optional<SolveSliceParallelArgs> solveSliceParallelArgs;

template <typename T>
void readRunAndOutput(uint64_t numPoints, std::function<void(std::vector<point<T>>&)> run) {
	auto beforeTime = std::chrono::high_resolution_clock::now();
	
	std::vector<pointd> pointsd(numPoints);
	if (readBinary) {
		std::cin.read(reinterpret_cast<char*>(pointsd.data()), pointsd.size() * sizeof(point<T>));
	} else {
		for (uint64_t i = 0; i < numPoints; i++) {
			std::string line;
			std::getline(std::cin, line);
			size_t spacePos = line.find(' ');
			assert(spacePos != std::string::npos);
			
			std::from_chars(&line[0], &line[spacePos], pointsd[i].x);
			std::from_chars(&line[spacePos + 1], &line[line.size()], pointsd[i].y);
		}
	}
	
	std::vector<point<T>>* points;
	std::vector<point<T>> pointsTVec;
	if constexpr (std::is_same_v<T, double>) {
		points = &pointsd;
	} else {
		points = &pointsTVec;
		pointsTVec.resize(pointsd.size());
		for (uint64_t i = 0; i < numPoints; i++) {
			if constexpr (std::is_integral_v<T>) {
				pointsd[i].x = std::round(pointsd[i].x);
				pointsd[i].y = std::round(pointsd[i].y);
			}
			pointsTVec[i] = point<T>(pointsd[i]);
		}
	}
	
	if (solveSliceParallelArgs) {
		run = [innerSolve=run] (std::vector<point<T>>& p) {
			solveSliceParallel<T>(p, innerSolve, *solveSliceParallelArgs);
		};
	}
	
	auto startTime = std::chrono::high_resolution_clock::now();
	run(*points);
	auto endTime = std::chrono::high_resolution_clock::now();
	
	std::cout << "on hull: " << points->size() << "\n";
	if (outputPoints) {
		std::rotate(points->begin(), std::min_element(points->begin(), points->end()), points->end());
		std::cout << std::setprecision(15) << std::fixed;
		for (const auto& point : *points) {
			if (std::isnan(point.x) || std::isnan(point.y) || std::isinf(point.x) || std::isinf(point.y)) {
				std::cerr << "output contains inf/nan!\n";
			}
			std::cout << point.x << " " << point.y << "\n";
		}
	}
	
	std::cerr << "elapsed time: " << std::chrono::duration<double, std::milli>(endTime - beforeTime).count() << "ms\n";
	std::cerr << "compute time: " << std::chrono::duration<double, std::milli>(endTime - startTime).count() << "ms\n";
}

[[noreturn]] void printImplementationNamesAndExit() {
	std::cout << "Available implementations:\n";
	for (const HullImpl& impl : *hullImplementations) {
		std::cout << " - " << impl.name << " (";
		if (impl.runInt)
			std::cout << "int";
		if (impl.runDouble) {
			if (impl.runInt)
				std::cout << ", ";
			std::cout << "double";
		}
		std::cout << ")\n";
	}
	std::exit(1);
}

int main(int argv, char** argc) {
	std::ios_base::sync_with_stdio(false);
	std::cin.tie(nullptr);
	
	std::sort(hullImplementations->begin(), hullImplementations->end(),
	          [] (const auto& a, const auto& b) { return a.name < b.name; });
	
	bool useIntVersion = false;
	outputPoints = true;
	std::string_view implName;
	for (int i = 1; i < argv; i++) {
		std::string_view arg = argc[i];
		if (arg == "-i") {
			useIntVersion = true;
		} else if (arg == "-q") {
			outputPoints = false;
		} else if (arg.starts_with("-sp")) {
			std::string_view numThreads = arg.substr(3);
			solveSliceParallelArgs = SolveSliceParallelArgs();
			if (!numThreads.empty()) {
				std::from_chars(numThreads.data(), numThreads.data() + numThreads.size(), solveSliceParallelArgs->numThreads);
			}
		} else if (arg.starts_with("-spm=")) {
			solveSliceParallelArgs->splitMethod = splitMethodFromString(arg.substr(5));
		} else if (!arg.starts_with("-")) {
			implName = arg;
		}
	}
	
	if (implName.empty()) {
		std::cout << "No implementation specified.";
		printImplementationNamesAndExit();
	}
	
	size_t implNameColonPos = implName.find(':');
	if (implNameColonPos != std::string_view::npos) {
		implArgs = implName.substr(implNameColonPos + 1);
		implName = implName.substr(0, implNameColonPos);
	}
	
	auto implIterator = std::find_if(
		hullImplementations->begin(), hullImplementations->end(),
		[&] (const HullImpl& impl) { return impl.name == implName; });
	if (implIterator == hullImplementations->end()) {
		std::cout << "Implementation not found: " << implName;
		printImplementationNamesAndExit();
	}
	if (useIntVersion && !implIterator->runInt) {
		std::cout << "Integer implementation not available for " << implName << "\n";
		return 1;
	}
	
	uint64_t numPoints = 0;
	
	char c0 = std::cin.get();
	if (c0 == 'B') {
		readBinary = true;
		std::cin.read(reinterpret_cast<char*>(&numPoints), sizeof(numPoints));
	} else if (c0 == '2') {
		std::string line;
		std::getline(std::cin, line);
		std::getline(std::cin, line);
		numPoints = std::stoi(line);
	} else {
		std::cerr << "unexpected first character " << c0 << ", expected 2 or B\n";
		return 1;
	}
	
	if (useIntVersion) {
		readRunAndOutput<int64_t>(numPoints, implIterator->runInt);
	} else {
		std::cin.precision(15);
		readRunAndOutput<double>(numPoints, implIterator->runDouble);
	}
}
