#pragma once

#include <cstdint>
#include <ostream>
#include <tuple>
#include <cmath>
#include <limits>

enum class side {
	left,
	right,
	on
};

template <typename T>
struct point {
	T x, y;
	
	point() : x(0), y(0) { }
	point(T xx, T yy) : x(xx), y(yy) { }
	
	template <typename U> point(point<U> o) : x(static_cast<T>(o.x)), y(static_cast<T>(o.y)) { }
	point operator+(point o) const { return { x+o.x, y+o.y }; }
	point operator-(point o) const { return { x-o.x, y-o.y }; }
	point operator*(T o) const { return { x*o, y*o }; }
	point operator/(T o) const { return { x/o, y/o }; }
	bool operator==(point o) const { return x==o.x && y==o.y; };
	bool operator<(point o) const { return std::tie(x, y) < std::tie(o.x, o.y); };
	bool operator>(point o) const { return std::tie(x, y) > std::tie(o.x, o.y); };
	T dot(point o) const { return x*o.x + y*o.y; }
	T cross(point b) const { return x*b.y - y*b.x; }
	T cross(point b, point o) const { return (*this - o).cross(b - o); }
	T len2() const { return x*x + y*y; }
	T lenmh() const { return std::abs(x) + std::abs(y); }
	point rotated90CW() const { return point(y, -x); };
	point rotated90CCW() const { return point(-y, x); };
	
	side sideOfLine(point lineStart, point lineEnd, T epsilon = 0) const {
		auto c = cross(lineEnd, lineStart);
		if (c > epsilon)
			return side::right;
		if (c < -epsilon)
			return side::left;
		return side::on;
	}
	
	static const point<T> notOnHull;
	
	bool isNotOnHull() const {
		if constexpr (std::numeric_limits<T>::has_quiet_NaN) {
			return std::isnan(x);
		} else {
			return x == std::numeric_limits<T>::max();
		}
	}
};
using pointi = point<int64_t>;
using pointd = point<double>;

template <typename T>
inline T _getNotOnHullValue() {
	if constexpr (std::numeric_limits<T>::has_quiet_NaN) {
		return std::numeric_limits<T>::quiet_NaN();
	} else {
		return std::numeric_limits<T>::max();
	}
}
template <typename T>
const point<T> point<T>::notOnHull = { _getNotOnHullValue<T>(), _getNotOnHullValue<T>() };

static_assert(sizeof(pointi) == 2 * sizeof(int64_t));
static_assert(sizeof(pointd) == 2 * sizeof(double));

template <typename T>
std::ostream& operator<<(std::ostream& stream, const point<T>& p) {
	return stream << "(" << p.x << " " << p.y << ")";
}
