#include "generator.hpp"

#include <numbers>

void generatePoints(std::vector<point>& points, rand_generator& rng) {
	double radius = getArgOrDefault("r", 1000);
	std::uniform_real_distribution<double> angleDistribution(0, std::numbers::pi_v<double> * 2);
	for (point& p : points) {
		double angle = angleDistribution(rng);
		p.x = std::cos(angle) * radius;
		p.y = std::sin(angle) * radius;
	}
}
