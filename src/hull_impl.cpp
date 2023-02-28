#include "hull_impl.hpp"

#include <charconv>

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
