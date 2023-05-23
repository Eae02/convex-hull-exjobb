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
	points[0].x = -2*radius, points[0].y = -radius; // (-2r,-r)
	points[1].x = 2*radius, points[1].y = -radius; //(2r,-r)
	points[2].x = 0, points[2].y = 2*radius; //(0,2r)
}
