#pragma once

#include <vector>
#include <algorithm>
#include <fstream>
#include <random>
#include <charconv>
#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <string_view>

using rand_generator = std::mt19937;

struct point {
	double x;
	double y;
};
static_assert(sizeof(point) == 16);

std::unordered_map<std::string_view, int> args;

int getArgOrDefault(std::string_view name, int def = 0) {
	auto it = args.find(name);
	return it == args.end() ? def : it->second;
}

void generatePoints(std::vector<point>& points, rand_generator& rng);

void writeOutput(std::ostream& stream, const std::vector<point>& points) {
	if (getArgOrDefault("bin")) {
		stream.put('B');
		uint64_t numPoints = points.size();
		stream.write(reinterpret_cast<const char*>(&numPoints), sizeof(numPoints));
		stream.write(reinterpret_cast<const char*>(points.data()), points.size() * sizeof(point));
	} else {
		stream << "2\n" << points.size() << "\n" << std::fixed << std::setprecision(getArgOrDefault("prec", 10));
		for (const point& point : points) {
			stream << point.x << " " << point.y << "\n";
		}
	}
}

int main(int argc, char** argv) {
	std::ios_base::sync_with_stdio(false);
	std::cin.tie(nullptr);
	
	const char* outPath = nullptr;
	
	for (int i = 1; i < argc; i++) {
		std::string_view arg(argv[i]);
		size_t eqPos = arg.find("=");
		if (eqPos != std::string_view::npos) {
			std::string_view argName = arg.substr(0, eqPos);
			std::string_view argValue = arg.substr(eqPos + 1);
			if (argName == "out") {
				outPath = argValue.data();
			} else {
				int argValueInt = 1;
				if (std::from_chars(argValue.data(), argValue.data() + argValue.size(), argValueInt).ec != std::errc()) {
					std::cerr << "invalid value for argument " << argName << ": " << argValue << "\n";
					return 1;
				}
				args[argName] = argValueInt;
			}
		} else {
			args[arg] = 1;
		}
	}
	
	int n = getArgOrDefault("n", 10);
	int64_t seed = getArgOrDefault("seed", -1);
	if (seed == -1) {
		std::random_device rd;
		seed = rd();
		std::cerr << "using seed " << seed << "\n";
	}
	
	rand_generator rng(seed);
	std::vector<point> points(n);
	generatePoints(points, rng);
	
	if (outPath != nullptr) {
		std::ofstream outStream(std::string(outPath), std::ios::binary);
		writeOutput(outStream, points);
	} else {
		writeOutput(std::cout, points);
	}
}
