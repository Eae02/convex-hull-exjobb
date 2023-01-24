#pragma once

#include <cstdint>
#include <tuple>

template <typename T>
struct point {
	T x, y;
	
	point() : x(0), y(0) { }
	point(T xx, T yy) : x(xx), y(yy) { }
	
	template <typename U> point(point<U> o) : x(o.x), y(o.y) { }
	point operator+(point o) const { return { x+o.x, y+o.y }; }
	point operator-(point o) const { return { x-o.x, y-o.y }; }
	point operator*(T o) const { return { x*o, y*o }; }
	point operator/(T o) const { return { x/o, y/o }; }
	bool operator==(point o) const { return x==o.x && y==o.y; };
	bool operator<(point o) const { return std::tie(x, y) < std::tie(o.x, o.y); };
	T dot(point o) const { return x*o.x + y*o.y; }
	T cross(point b) const { return x*b.y - y*b.x; }
	T cross(point b, point o) const { return (*this - o).cross(b - o); }
	T len2() const { return x*x + y*y; }
	point rotated90CW() const { return point(y, -x); };
	point rotated90CCW() const { return point(-y, x); };
};
using pointi = point<int64_t>;
using pointd = point<double>;

static_assert(sizeof(pointi) == 2 * sizeof(int64_t));
static_assert(sizeof(pointd) == 2 * sizeof(double));
