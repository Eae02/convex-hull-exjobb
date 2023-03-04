#include "../hull_impl.hpp"
#include "../point.hpp"
#include "simd_utils.hpp"

#include <algorithm>
#include <vector>
#include <span>
#include <cmath>
#include <thread>
#include <iostream>
#include <iomanip>
#include <list>
#include <mutex>
#include <cassert>

namespace avx2 {

struct points {
	__m256d* x;
	__m256d* y;
	uint32_t vcount;
	uint8_t skipFirst;
	uint8_t skipLast;
	
	template <typename CallbackT>
	void forEach(CallbackT callback) {
		if (vcount == 0)
			return;
		uint8_t firstMask = ((1 << skipFirst) - 1) ^ 0xF;
		uint8_t lastMask = (0x10 >> skipLast) - 1;
		if (vcount == 1) {
			callback(0, firstMask & lastMask);
		} else {
			callback(0, firstMask);
			for (size_t i = 1; i < vcount - 1; i++) {
				callback(i, 0xF);
			}
			callback(vcount - 1, lastMask);
		}
	}
	
	points subspan(uint32_t first, uint32_t count) const {
		uint32_t ofirst = first + (uint32_t)skipFirst;
		uint32_t oend = ofirst + count;
		uint32_t vfirst = ofirst / 4;
		uint32_t vend = (oend + 3) / 4;
		return points {
			.x = x + vfirst,
			.y = y + vfirst,
			.vcount = vend - vfirst,
			.skipFirst = (uint8_t)(ofirst % 4),
			.skipLast = (uint8_t)((vend * 4) - oend)
		};
	}
	
	uint32_t count() const {
		return vcount * 4 - (uint32_t)skipFirst - (uint32_t)skipLast;
	}
	
	std::pair<double*, double*> getDoublePointers() {
		return { reinterpret_cast<double*>(x) + skipFirst, reinterpret_cast<double*>(y) + skipFirst };
	}
	
	pointd at(uint32_t i) const {
		size_t oi = i + (uint32_t)skipFirst;
		return { x[oi/4][oi%4], y[oi/4][oi%4] };
	}
	
	void set(size_t i, pointd p) {
		size_t oi = i + (uint32_t)skipFirst;
		x[oi/4][oi%4] = p.x;
		y[oi/4][oi%4] = p.y;
	}
	
	void print(int prefix = 0) const {
		std::cerr << std::string(prefix * 2, ' ');
		for (size_t i = 0; i < count(); i++) {
			std::cerr << at(i) << " ";
		}
		std::cerr << "\n";
	}
};

__m256d bitmapToVecmask(int m) {
	static const __m256i vshift_count = _mm256_set_epi64x(60, 61, 62, 63);
	__m256i bcast = _mm256_set1_epi64x(m);
	return _mm256_castsi256_pd(_mm256_sllv_epi64(bcast, vshift_count));
}

template <bool KeepLeft>
size_t partitionByLine(points pts, pointd lineStart, pointd lineEnd) {
	const auto lineStartX4 = _mm256_set1_pd(lineStart.x);
	const auto lineStartY4 = _mm256_set1_pd(lineStart.y);
	const auto lineDeltaX4 = _mm256_set1_pd(lineEnd.x - lineStart.x);
	const auto lineDeltaY4 = _mm256_set1_pd(lineEnd.y - lineStart.y);
	
	size_t numRight = 0;
	
	auto [ptsxd, ptsyd] = pts.getDoublePointers();
	
	pts.forEach([&] (size_t vi, uint32_t activeCompMask) {
		uint32_t mask = static_cast<uint32_t>(_mm256_movemask_pd(_mm256_cmp_pd(
			_mm256_mul_pd(_mm256_sub_pd(pts.y[vi], lineStartY4), lineDeltaX4),
			_mm256_mul_pd(_mm256_sub_pd(pts.x[vi], lineStartX4), lineDeltaY4),
			_CMP_LT_OQ
		))) & activeCompMask;
		
		for (uint32_t j = 0; j < 4; j++) {
			if (mask & ((uint32_t)1 << j)) {
				if (KeepLeft) {
					std::swap(ptsxd[numRight], pts.x[vi][j]);
					std::swap(ptsyd[numRight], pts.y[vi][j]);
				} else {
					ptsxd[numRight] = pts.x[vi][j];
					ptsyd[numRight] = pts.y[vi][j];
				}
				numRight++;
			}
		}
	});
	
	return numRight;
}

int findMaxPointIndex(points pts, pointd offsetPoint, pointd normal) {
	const auto normalX4 = _mm256_set1_pd(normal.x);
	const auto normalY4 = _mm256_set1_pd(normal.y);
	const auto offsetX4 = _mm256_set1_pd(offsetPoint.x);
	const auto offsetY4 = _mm256_set1_pd(offsetPoint.y);
	
	__m256i indices = _mm256_setr_epi64x(0, 1, 2, 3);
	const __m256i indicesInc = _mm256_set1_epi64x(4);
	
	__m256d maxDotValues = _mm256_set1_pd(-INFINITY);
	__m256i maxIndices = _mm256_set1_epi64x(0);
	
	pts.forEach([&] (size_t vi, uint32_t activeCompMask) {
		auto mulx = _mm256_mul_pd(_mm256_sub_pd(pts.x[vi], offsetX4), normalX4);
		auto dot = _mm256_fmadd_pd(_mm256_sub_pd(pts.y[vi], offsetY4), normalY4, mulx);
		
		__m256d cmpResult = _mm256_cmp_pd(maxDotValues, dot, _CMP_LT_OQ);
		cmpResult = _mm256_and_pd(cmpResult, bitmapToVecmask(activeCompMask));
		
		maxDotValues = _mm256_blendv_pd(maxDotValues, dot, cmpResult);
		maxIndices = _mm256_castpd_si256(_mm256_blendv_pd(_mm256_castsi256_pd(maxIndices), _mm256_castsi256_pd(indices), cmpResult));
		indices = _mm256_add_epi64(indices, indicesInc);
	});
	
	alignas(__m256i) int64_t maxIndicesBuffer[4];
	_mm256_store_si256(reinterpret_cast<__m256i*>(maxIndicesBuffer), maxIndices);
	
	std::pair<double, int64_t> maxPoint(-INFINITY, -1);
	for (int i = 0; i < 4; i++) {
		maxPoint = std::max(maxPoint, std::make_pair(maxDotValues[i], maxIndicesBuffer[i]));
	}
	
	return (int)maxPoint.second - (int)pts.skipFirst;
}

void quickhullAvxRec(points pts, pointd leftHullPoint, pointd rightHullPoint, std::vector<pointd>& output, bool isUpperHull) {
	if (pts.count() <= 1) {
		if (pts.count() == 1) {
			output.push_back(pts.at(0));
		}
		return;
	}
	
	const pointd normal = (rightHullPoint - leftHullPoint).rotated90CCW();
	size_t maxPointIndex = findMaxPointIndex(pts, leftHullPoint, normal);
	
	pointd maxPoint = pts.at(maxPointIndex);
	pts.set(maxPointIndex, pts.at(pts.count() - 1));
	pts = pts.subspan(0, pts.count() - 1);
	
	const __m256d maxPointX4 = _mm256_set1_pd(maxPoint.x);
	
	auto [ptsxd, ptsyd] = pts.getDoublePointers();
	uint32_t numRight = 0;
	pts.forEach([&] (size_t vi, uint32_t activeCompMask) {
		__m256d isRightMask256 = _mm256_cmp_pd(pts.x[vi], maxPointX4, _CMP_LT_OQ);
		for (size_t c = 0; c < 4; c++) {
			if (((bool)isRightMask256[c] != isUpperHull) && (activeCompMask & (1 << c))) {
				std::swap(ptsxd[numRight], pts.x[vi][c]);
				std::swap(ptsyd[numRight], pts.y[vi][c]);
				numRight++;
			}
		}
	});
	
	points pointsR = pts.subspan(0, numRight);
	points pointsL = pts.subspan(numRight, pts.count() - numRight);
	
	size_t numR = partitionByLine<false>(pointsR, rightHullPoint, maxPoint);
	size_t numL = partitionByLine<false>(pointsL, maxPoint, leftHullPoint);
	
	quickhullAvxRec(pointsR.subspan(0, numR), maxPoint, rightHullPoint, output, isUpperHull);
	
	output.push_back(maxPoint);
	
	quickhullAvxRec(pointsL.subspan(0, numL), leftHullPoint, maxPoint, output, isUpperHull);
}

std::pair<int, int> findMinMax(__m256d* ptsx, __m256d* ptsy, size_t vecCount) {
	__m256d maxX = _mm256_set1_pd(-INFINITY);
	__m256d maxY = _mm256_set1_pd(-INFINITY);
	__m128i maxIndices = _mm_set1_epi32(0);
	__m256d minX = _mm256_set1_pd(INFINITY);
	__m256d minY = _mm256_set1_pd(INFINITY);
	__m128i minIndices = _mm_set1_epi32(0);
	
	__m128i indices = _mm_setr_epi32(0, 1, 2, 3);
	const __m128i indicesInc = _mm_set1_epi32(4);
	
	for (size_t i = 0; i < vecCount; i++) {
		__m256d x = ptsx[i];
		__m256d y = ptsy[i];
		
		auto maxCmpMask = _mm256_or_pd(
			_mm256_cmp_pd(maxX, x, _CMP_LT_OS),
			(_mm256_and_pd(_mm256_cmp_pd(maxX, x, _CMP_EQ_OS), _mm256_cmp_pd(maxY, y, _CMP_LT_OS))));
		maxX = _mm256_blendv_pd(maxX, x, maxCmpMask);
		maxY = _mm256_blendv_pd(maxY, y, maxCmpMask);
		maxIndices = _mm_blendv_epi8(maxIndices, indices, cvtepi64_epi32_avx(maxCmpMask));
		
		auto minCmpMask = _mm256_or_pd(
			_mm256_cmp_pd(minX, x, _CMP_GT_OS), 
			(_mm256_and_pd(_mm256_cmp_pd(minX, x, _CMP_EQ_OS), _mm256_cmp_pd(minY, y, _CMP_GT_OS))));
		minX = _mm256_blendv_pd(minX, x, minCmpMask);
		minY = _mm256_blendv_pd(minY, y, minCmpMask);
		minIndices = _mm_blendv_epi8(minIndices, indices, cvtepi64_epi32_avx(minCmpMask));
		
		indices = _mm_add_epi32(indices, indicesInc);
	}
	
	alignas(__m128i) int minIndicesBuffer[4], maxIndicesBuffer[4];
	_mm_store_si128(reinterpret_cast<__m128i*>(minIndicesBuffer), minIndices);
	_mm_store_si128(reinterpret_cast<__m128i*>(maxIndicesBuffer), maxIndices);
	
	std::tuple<double, double, int> maxPoint(-INFINITY, -INFINITY, -1);
	std::tuple<double, double, int> minPoint(INFINITY, INFINITY, -1);
	for (int i = 0; i < 4; i++) {
		minPoint = std::min(minPoint, std::make_tuple(minX[i], minY[i], minIndicesBuffer[i]));
		maxPoint = std::max(maxPoint, std::make_tuple(maxX[i], maxY[i], maxIndicesBuffer[i]));
	}
	
	return { std::get<2>(minPoint), std::get<2>(maxPoint) };
}

void runQuickhullAvx(std::vector<pointd>& pts) {
	size_t bufferSize = ((pts.size() * 2 * sizeof(double)) + 128) & ~31;
	char* buffer = static_cast<char*>(std::aligned_alloc(32, bufferSize * 2));
	
	__m256d* ptsx = reinterpret_cast<__m256d*>(buffer);
	__m256d* ptsy = reinterpret_cast<__m256d*>(buffer + bufferSize);
	
	initPoints256(pts, ptsx, ptsy, NAN);
	
	auto [leftmostIdx, rightmostIdx] = findMinMax(ptsx, ptsy, (pts.size() + 3) / 4);
	
	pointd leftmostPt = pts[leftmostIdx];
	pointd rightmostPt = pts[rightmostIdx];
	
	size_t numPoints = pts.size();
	auto popPoint = [&] (size_t idx) {
		numPoints--;
		ptsx[idx/4][idx%4] = ptsx[numPoints/4][numPoints%4];
		ptsy[idx/4][idx%4] = ptsy[numPoints/4][numPoints%4];
	};
	
	popPoint(std::max(leftmostIdx, rightmostIdx));
	popPoint(std::min(leftmostIdx, rightmostIdx));
	
	uint32_t vCount = (numPoints + 3) / 4;
	points pointsSpan = points {
		.x = ptsx,
		.y = ptsy,
		.vcount = vCount,
		.skipFirst = 0,
		.skipLast = (uint8_t)(vCount * 4 - numPoints)
	};
	
	size_t numPointsBelow = partitionByLine<true>(pointsSpan, leftmostPt, rightmostPt);
	
	pts.clear();
	pts.push_back(leftmostPt);
	
	quickhullAvxRec(pointsSpan.subspan(0, numPointsBelow), rightmostPt, leftmostPt, pts, false);
	
	pts.push_back(rightmostPt);
	
	quickhullAvxRec(pointsSpan.subspan(numPointsBelow, numPoints - numPointsBelow), leftmostPt, rightmostPt, pts, true);
	
	std::free(buffer);
}

DEF_HULL_IMPL({
	.name = "qh_avx",
	.runInt = nullptr,
	.runDouble = runQuickhullAvx,
});
}
