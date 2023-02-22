#include "generator.hpp"

#include <numbers>

void generatePoints(std::vector<point>& points, rand_generator& rng) {
	double radius = getArgOrDefault("radius", 100);
	std::uniform_real_distribution<double> distribution(-radius, radius);
	
	
	for (point& p : points) {
		double x,y;
		do { // Loop until generating a point in the disk
			x = distribution(rng);
			y = distribution(rng);
		} while (x*x + y*y > radius*radius);
		p.x = x;
		p.y = y;
	}
}
