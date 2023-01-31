#include "generator.hpp"

#include <numbers>

void generatePoints(std::vector<point>& points, rand_generator& rng) {
	double min = getArgOrDefault("min", -100);
	double max = getArgOrDefault("max", 100);
	std::uniform_real_distribution<double> distribution(min, max);
	for (point& p : points) {
		p.x = distribution(rng);
		p.y = distribution(rng);
	}
}
