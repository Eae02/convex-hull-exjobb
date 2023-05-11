#pragma once

#include "../point.hpp"

#include <span>

template <typename T>
size_t findFurthestPointFromLine(std::span<const point<T>> pts, point<T> lineStart, point<T> lineEnd) {
	point<T> normal = (lineEnd - lineStart).rotated90CCW();
	
	size_t maxPointIdx = 0;
	T maxPointDot = normal.dot(pts[0] - lineStart);
	point<T> maxPoint = pts[0];
	for (size_t i = 1; i < pts.size(); i++) {
		T dotProduct = normal.dot(pts[i] - lineStart);
		if (std::tie(dotProduct, pts[i]) > std::tie(maxPointDot, maxPoint)) {
			maxPointIdx = i;
			maxPointDot = dotProduct;
			maxPoint = pts[i];
		}
	}
	
	return maxPointIdx;
}

enum class qhPartitionStrategy {
	noPartitionByX,
	firstPartitionByX,
	singleScan
};

template <qhPartitionStrategy S, typename T, bool WriteNotOnHull = true>
std::array<std::span<point<T>>, 2> quickhullPartitionPoints(
	std::span<point<T>> pts,
	point<T> leftHullPoint,
	point<T> rightHullPoint,
	size_t& midHullPointIdx
) {
	point<T> maxPoint = pts[midHullPointIdx];
	std::swap(pts[midHullPointIdx], pts.back());
	
	size_t numPointsRight, numPointsNotLeft;
	
	if constexpr (S == qhPartitionStrategy::singleScan) {
		numPointsRight = 0;
		while (numPointsRight < pts.size() - 1 && pts[numPointsRight].sideOfLine(rightHullPoint, maxPoint) == side::right) {
			numPointsRight++;
		}
		
		numPointsNotLeft = pts.size() - 1;
		for (size_t i = numPointsRight; i < numPointsNotLeft;) {
			if (pts[i].sideOfLine(rightHullPoint, maxPoint) == side::right) {
				std::swap(pts[numPointsRight++], pts[i++]);
			} else if (pts[i].sideOfLine(maxPoint, leftHullPoint) == side::right) {
				std::swap(pts[--numPointsNotLeft], pts[i]);
			} else {
				if constexpr (WriteNotOnHull) {
					pts[i] = point<T>::notOnHull;
				}
				i++;
			}
		}
		
		std::swap(pts[numPointsNotLeft], pts.back());
		midHullPointIdx = numPointsNotLeft;
		
		return { pts.subspan(0, numPointsRight), pts.subspan(numPointsNotLeft + 1) };
	}
	
	if constexpr (S == qhPartitionStrategy::firstPartitionByX) {
		bool upperHull = leftHullPoint < rightHullPoint;
		
		auto rightPointsEndIt = std::partition(pts.begin(), pts.end() - 1, [&] (const point<T>& p) -> bool {
			return (p.x < maxPoint.x) ^ upperHull;
		});
		
		auto rightPointsValidEndIt = std::partition(pts.begin(), rightPointsEndIt, [&] (const point<T>& p) -> bool {
			return p.sideOfLine(rightHullPoint, maxPoint) == side::right;
		});
		
		if constexpr (WriteNotOnHull) {
			std::fill(rightPointsValidEndIt, rightPointsEndIt, point<T>::notOnHull);
		}
		
		numPointsNotLeft = rightPointsEndIt - pts.begin();
		numPointsRight = rightPointsValidEndIt - pts.begin();
	} else {
		auto rightPointsEndIt = std::partition(pts.begin(), pts.end() - 1, [&] (const point<T>& p) -> bool {
			return p.sideOfLine(rightHullPoint, maxPoint) == side::right;
		});
		
		numPointsNotLeft = numPointsRight = rightPointsEndIt - pts.begin();
	}
	
	std::swap(pts[numPointsNotLeft], pts.back());
	midHullPointIdx = numPointsNotLeft;
	
	auto leftPointsBeginIt = pts.begin() + numPointsNotLeft + 1;
	auto leftPointsEndIt = std::partition(leftPointsBeginIt, pts.end(), [&] (const point<T>& p) -> bool {
		return p.sideOfLine(maxPoint, leftHullPoint) == side::right;
	});
	
	if constexpr (WriteNotOnHull) {
		std::fill(leftPointsEndIt, pts.end(), point<T>::notOnHull);
	}
	
	size_t numPointsLeft = leftPointsEndIt - leftPointsBeginIt;
	
	return { pts.subspan(0, numPointsRight), pts.subspan(numPointsNotLeft + 1, numPointsLeft) };
}
