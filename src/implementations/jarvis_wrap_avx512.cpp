#include "../hull_impl.hpp"
#include "../point.hpp"
#include "simd_utils.hpp"

#include <algorithm>
#include <vector>
#include <cmath>
#include <cassert>

int findMinPoint(__m512d* ptsx, __m512d* ptsy, uint32_t vcount) {
	auto minX = _mm512_set1_pd(INFINITY);
	auto minY = _mm512_set1_pd(INFINITY);
	auto minIndices = _mm256_set1_epi32(0);
	
	__m256i indices = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
	const __m256i indicesInc = _mm256_set1_epi32(8);
	
	for (uint32_t i = 0; i < vcount; i++) {
		auto x = ptsx[i];
		auto y = ptsy[i];
		
		uint8_t minCmpMask = _mm512_cmplt_pd_mask(x, minX) | (_mm512_cmpeq_pd_mask(x, minX) & _mm512_cmplt_pd_mask(y, minY));
		minX = _mm512_mask_blend_pd(minCmpMask, minX, x);
		minY = _mm512_mask_blend_pd(minCmpMask, minY, y);
		minIndices = _mm256_mask_blend_epi32(minCmpMask, minIndices, indices);
		
		indices = _mm256_add_epi32(indices, indicesInc);
	}
	
	alignas(32) int minIndicesBuffer[8];
	_mm256_store_epi32(minIndicesBuffer, minIndices);
	
	std::tuple<double, double, int> minPoint(INFINITY, INFINITY, -1);
	for (int i = 0; i < 8; i++) {
		minPoint = std::min(minPoint, std::make_tuple(minX[i], minY[i], minIndicesBuffer[i]));
	}
	
	return std::get<2>(minPoint);
}

static void runJarvisWrapAvx512(std::vector<pointd>& pts) {
	size_t bufferSize = ((pts.size() * 2 * sizeof(double)) + 64) & ~63;
	char* buffer = static_cast<char*>(std::aligned_alloc(64, bufferSize * 2));
	
	__m512d* ptsx = reinterpret_cast<__m512d*>(buffer);
	__m512d* ptsy = reinterpret_cast<__m512d*>(buffer + bufferSize);
	initPoints512(pts, ptsx, ptsy, NAN);
	
	int minPointIdx = findMinPoint(ptsx, ptsy, (pts.size() + 7) / 8);
	pointd firstHullPoint = pts[minPointIdx];
	
	uint32_t numPoints = pts.size();
	
	pts.clear();
	
	pointd lastHullPoint = firstHullPoint;
	
	auto addPoint = [&] (uint32_t index) {
		assert(index < numPoints);
		
		double* nextPointPtrX = reinterpret_cast<double*>(ptsx) + index;
		double* nextPointPtrY = reinterpret_cast<double*>(ptsy) + index;
		
		pointd newHullPoint(*nextPointPtrX, *nextPointPtrY);
		if (pts.size() > 1 && firstHullPoint.sideOfLine(lastHullPoint, newHullPoint) != side::left)
			return false;
		pts.push_back(newHullPoint);
		lastHullPoint = newHullPoint;
		
		numPoints--;
		*nextPointPtrX = reinterpret_cast<double*>(ptsx)[numPoints];
		*nextPointPtrY = reinterpret_cast<double*>(ptsy)[numPoints];
		reinterpret_cast<double*>(ptsx)[numPoints] = NAN;
		reinterpret_cast<double*>(ptsy)[numPoints] = NAN;
		
		return true;
	};
	
	addPoint(minPointIdx);
	
	while (numPoints > 0) {
		__m256i indices = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
		const __m256i indicesInc = _mm256_set1_epi32(8);
		
		__m512d lastHullPointX8 = _mm512_set1_pd(lastHullPoint.x);
		__m512d lastHullPointY8 = _mm512_set1_pd(lastHullPoint.y);
		
		__m256i nextHullPointIdx = indices;
		__m512d nextHullPointRelX = _mm512_sub_pd(ptsx[0], lastHullPointX8);
		__m512d nextHullPointRelY = _mm512_sub_pd(ptsy[0], lastHullPointY8);
		__m512d nextHullPointDist = _mm512_add_pd(_mm512_abs_pd(nextHullPointRelX), _mm512_abs_pd(nextHullPointRelY));
		
		for (uint32_t i = 1; i < (numPoints + 7) / 8; i++) {
			indices = _mm256_add_epi32(indices, indicesInc);
			
			__m512d ptRelX = _mm512_sub_pd(ptsx[i], lastHullPointX8);
			__m512d ptRelY = _mm512_sub_pd(ptsy[i], lastHullPointY8);
			
			__m512d crossLHS = _mm512_mul_pd(ptRelY, nextHullPointRelX);
			__m512d crossRHS = _mm512_mul_pd(ptRelX, nextHullPointRelY);
			
			uint8_t cmpMask = _mm512_cmplt_pd_mask(crossLHS, crossRHS);
			
			__m512d newDist = _mm512_add_pd(_mm512_abs_pd(ptRelX), _mm512_abs_pd(ptRelY));
			cmpMask |= _mm512_cmpeq_pd_mask(crossLHS, crossRHS) & _mm512_cmplt_pd_mask(nextHullPointDist, newDist);
			
			nextHullPointRelX = _mm512_mask_blend_pd(cmpMask, nextHullPointRelX, ptRelX);
			nextHullPointRelY = _mm512_mask_blend_pd(cmpMask, nextHullPointRelY, ptRelY);
			nextHullPointDist = _mm512_mask_blend_pd(cmpMask, nextHullPointDist, newDist);
			nextHullPointIdx = _mm256_mask_blend_epi32(cmpMask, nextHullPointIdx, indices);
		}
		
		alignas(32) int indicesBuffer[8];
		_mm256_store_epi32(indicesBuffer, nextHullPointIdx);
		
		int nextHullPointI = indicesBuffer[0];
		pointd nextHullPointRel(nextHullPointRelX[0], nextHullPointRelY[0]);
		for (uint32_t i = 1; i < 8; i++) {
			pointd p(nextHullPointRelX[i], nextHullPointRelY[i]);
			double cross = nextHullPointRel.cross(p);
			if (cross < 0 || (cross == 0 && p.lenmh() > nextHullPointRel.lenmh())) {
				nextHullPointRel = p;
				nextHullPointI = indicesBuffer[i];
			}
		}
		
		if (!addPoint(nextHullPointI))
			break;
	}
	
	std::free(buffer);
}

DEF_HULL_IMPL({
	.name = "jarvis_wrap_avx512",
	.runInt = nullptr,
	.runDouble = &runJarvisWrapAvx512,
});
