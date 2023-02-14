#include "generator.hpp"

#include <numbers>

void generatePoints(std::vector<point>& points, rand_generator& rng) {
	double min = getArgOrDefault("min", -100);
	double max = getArgOrDefault("max", 100);
	std::uniform_real_distribution<double> distribution(min, max);
	
	double rotation = std::uniform_real_distribution<double>(0, M_PI * 2)(rng);
	double cosR = std::cos(rotation);
	double sinR = std::sin(rotation);
	
	for (point& p : points) {
		double x = distribution(rng);
		double y = distribution(rng);
		p.x = cosR * x - sinR * y;
		p.y = sinR * y + cosR * x;
	}
}
