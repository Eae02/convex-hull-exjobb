#include "hull_impl.hpp"

std::vector<HullImpl>* hullImplementations;

int _defHullImpl(HullImpl impl) {
	if (hullImplementations == nullptr) {
		hullImplementations = new std::vector<HullImpl>;
	}
	hullImplementations->push_back(std::move(impl));
	return 0;
}
