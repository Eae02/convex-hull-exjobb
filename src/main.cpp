#include "hull_impl.hpp"

#include <iostream>
#include <algorithm>
#include <string_view>
#include <chrono>
#include <cassert>
#include <charconv>

template <typename T>
void readRunAndOutput(int numPoints, const std::function<void(std::vector<point<T>>&)>& run) {
	auto beforeTime = std::chrono::high_resolution_clock::now();
	
	std::vector<point<T>> points;
	points.reserve(numPoints);
	for (int i = 0; i < numPoints; i++) {
		std::string line;
		std::getline(std::cin, line);
		size_t spacePos = line.find(' ');
		assert(spacePos != std::string::npos);
		
		T x, y;
		std::from_chars(&line[0], &line[spacePos], x);
		std::from_chars(&line[spacePos + 1], &line[line.size()], y);
		points.emplace_back(x, y);
	}
	
	auto startTime = std::chrono::high_resolution_clock::now();
	run(points);
	auto endTime = std::chrono::high_resolution_clock::now();
	
	std::rotate(points.begin(), std::min_element(points.begin(), points.end()), points.end());
	
	std::cout << points.size() << "\n";
	for (const auto& point : points) {
		std::cout << point.x << " " << point.y << "\n";
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
	std::string_view implName;
	for (int i = 1; i < argv; i++) {
		std::string_view arg = argc[i];
		if (arg == "-i") {
			useIntVersion = true;
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
	
	std::string line;
	std::getline(std::cin, line);
	int dimension = std::stoi(line);
	if (dimension != 2) {
		std::cout << "Dimension must be 2\n";
		return 1;
	}
	
	std::getline(std::cin, line);
	int numPoints = std::stoi(line);
	
	if (useIntVersion) {
		readRunAndOutput<int64_t>(numPoints, implIterator->runInt);
	} else {
		std::cin.precision(15);
		readRunAndOutput<double>(numPoints, implIterator->runDouble);
	}
}
