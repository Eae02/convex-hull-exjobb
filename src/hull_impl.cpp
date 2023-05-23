#include "hull_impl.hpp"

#include <charconv>
#include <chrono>
#include <iostream>

std::vector<HullImpl>* hullImplementations;

std::string_view implArgs;

int _defHullImpl(HullImpl impl) {
	if (hullImplementations == nullptr) {
		hullImplementations = new std::vector<HullImpl>;
	}
	hullImplementations->push_back(std::move(impl));
	return 0;
}

std::optional<int> getImplArgInt(std::string_view argPrefix) {
	size_t pos = implArgs.find(argPrefix);
	if (pos != std::string_view::npos) {
		int value;
		if (std::from_chars(implArgs.data() + pos + argPrefix.size(), implArgs.data() + implArgs.size(), value).ec == std::errc())
			return value;
	}
	return {};
}

static std::vector<std::pair<std::string_view, std::chrono::high_resolution_clock::time_point>> intermediateTimes;

void addIntermediateTime(std::string_view name) {
	intermediateTimes.emplace_back(name, std::chrono::high_resolution_clock::now());
}

void printIntermediateTimes(std::chrono::high_resolution_clock::time_point startTime) {
	if (intermediateTimes.empty())
		return;
	std::cerr << "intermediate times:\n";
	for (auto [name, time] : intermediateTimes) {
		std::cerr << "| " << name << ": " << std::chrono::duration<double, std::milli>(time - startTime).count() << " ms\n";
	}
}
