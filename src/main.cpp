#include "hull_impl.hpp"

#include <iostream>
#include <algorithm>
#include <string_view>
#include <chrono>
#include <cassert>
#include <charconv>
#include <iomanip>

static bool readBinary;
static bool outputPoints;

template <typename T>
void readRunAndOutput(uint64_t numPoints, const std::function<void(std::vector<point<T>>&)>& run) {
	auto beforeTime = std::chrono::high_resolution_clock::now();
	
	std::vector<point<T>> points(numPoints);
	
	if (readBinary) {
		std::cin.read(reinterpret_cast<char*>(points.data()), points.size() * sizeof(point<T>));
	} else {
		for (int i = 0; i < numPoints; i++) {
			std::string line;
			std::getline(std::cin, line);
			size_t spacePos = line.find(' ');
			assert(spacePos != std::string::npos);
			
			std::from_chars(&line[0], &line[spacePos], points[i].x);
			std::from_chars(&line[spacePos + 1], &line[line.size()], points[i].y);
		}
	}
	
	auto startTime = std::chrono::high_resolution_clock::now();
	run(points);
	auto endTime = std::chrono::high_resolution_clock::now();
	
	std::rotate(points.begin(), std::min_element(points.begin(), points.end()), points.end());
	
	std::cout << "on hull: " << points.size() << "\n";
	if (outputPoints) {
		std::cout << std::setprecision(15) << std::fixed;
		for (const auto& point : points) {
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
		} else if (!arg.starts_with("-")) {
			implName = arg;
		}
	}
	
	if (implName.empty()) {
		std::cout << "No implementation specified.";
		printImplementationNamesAndExit();
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
