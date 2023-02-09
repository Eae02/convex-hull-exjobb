#include "../hull_impl.hpp"
#include "../point.hpp"
#include "simd_utils.hpp"

#include <algorithm>
#include <vector>
#include <cmath>

int findMinPoint(__m256d* ptsx, __m256d* ptsy, uint32_t vcount) {
	__m256d minX = _mm256_set1_pd(INFINITY);
	__m256d minY = _mm256_set1_pd(INFINITY);
	__m128i minIndices = _mm_set1_epi32(0);
	
	__m128i indices = _mm_setr_epi32(0, 1, 2, 3);
	const __m128i indicesInc = _mm_set1_epi32(4);
	
	for (uint32_t i = 0; i < vcount; i++) {
		auto x = ptsx[i];
		auto y = ptsy[i];
		
		auto minCmpMask = _mm256_or_pd(
			_mm256_cmp_pd(minX, x, _CMP_GT_OS), 
			(_mm256_and_pd(_mm256_cmp_pd(minX, x, _CMP_EQ_OS), _mm256_cmp_pd(minY, y, _CMP_GT_OS))));
		minX = _mm256_blendv_pd(minX, x, minCmpMask);
		minY = _mm256_blendv_pd(minY, y, minCmpMask);
		minIndices = _mm_blendv_epi8(minIndices, indices, cvtepi64_epi32_avx(minCmpMask));
		
		indices = _mm_add_epi32(indices, indicesInc);
	}
	
	alignas(__m128i) int minIndicesBuffer[4];
	_mm_store_si128(reinterpret_cast<__m128i*>(minIndicesBuffer), minIndices);
	
	std::tuple<double, double, int> minPoint(INFINITY, INFINITY, -1);
	for (int i = 0; i < 4; i++) {
		minPoint = std::min(minPoint, std::make_tuple(minX[i], minY[i], minIndicesBuffer[i]));
	}
	
	return std::get<2>(minPoint);
}

static void runJarvisWrapAvx(std::vector<pointd>& pts) {
	size_t bufferSize = ((pts.size() * 2 * sizeof(double)) + 32) & ~31;
	char* buffer = static_cast<char*>(std::aligned_alloc(32, bufferSize * 2));
	
	__m256d* ptsx = reinterpret_cast<__m256d*>(buffer);
	__m256d* ptsy = reinterpret_cast<__m256d*>(buffer + bufferSize);
	initPoints256(pts, ptsx, ptsy, NAN);
	
	int minPointIdx = findMinPoint(ptsx, ptsy, (pts.size() + 3) / 4);
	pointd firstHullPoint = pts[minPointIdx];
	
	uint32_t numPoints = pts.size();
	
	pts.clear();
	
	pointd lastHullPoint = firstHullPoint;
	
	auto addPoint = [&] (uint32_t index) {
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
		__m128i indices = _mm_setr_epi32(0, 1, 2, 3);
		const __m128i indicesInc = _mm_set1_epi32(4);
		
		__m256d lastHullPointX8 = _mm256_set1_pd(lastHullPoint.x);
		__m256d lastHullPointY8 = _mm256_set1_pd(lastHullPoint.y);
		
		__m128i nextHullPointIdx = indices;
		__m256d nextHullPointRelX = _mm256_sub_pd(ptsx[0], lastHullPointX8);
		__m256d nextHullPointRelY = _mm256_sub_pd(ptsy[0], lastHullPointY8);
		__m256d nextHullPointDist = _mm256_add_pd(abs256(nextHullPointRelX), abs256(nextHullPointRelY));
		
		for (uint32_t i = 1; i < (numPoints + 3) / 4; i++) {
			indices = _mm_add_epi32(indices, indicesInc);
			
			__m256d ptRelX = _mm256_sub_pd(ptsx[i], lastHullPointX8);
			__m256d ptRelY = _mm256_sub_pd(ptsy[i], lastHullPointY8);
			
			auto crossLHS = _mm256_mul_pd(ptRelY, nextHullPointRelX);
			auto crossRHS = _mm256_mul_pd(ptRelX, nextHullPointRelY);
			
			__m256d cmpMask = _mm256_cmp_pd(crossLHS, crossRHS, _CMP_LT_OQ);
			
			__m256d newDist = _mm256_add_pd(abs256(ptRelX), abs256(ptRelY));
			cmpMask = _mm256_or_pd(cmpMask, _mm256_and_pd(
				_mm256_cmp_pd(crossLHS, crossRHS, _CMP_EQ_OQ),
				_mm256_cmp_pd(nextHullPointDist, newDist, _CMP_LT_OQ)));
			
			nextHullPointRelX = _mm256_blendv_pd(nextHullPointRelX, ptRelX, cmpMask);
			nextHullPointRelY = _mm256_blendv_pd(nextHullPointRelY, ptRelY, cmpMask);
			nextHullPointDist = _mm256_blendv_pd(nextHullPointDist, newDist, cmpMask);
			nextHullPointIdx = _mm_blendv_epi8(nextHullPointIdx, indices, cvtepi64_epi32_avx(cmpMask));
		}
		
		alignas(__m128i) int indicesBuffer[4];
		_mm_store_si128(reinterpret_cast<__m128i*>(indicesBuffer), nextHullPointIdx);
		
		int nextHullPointI = indicesBuffer[0];
		pointd nextHullPointRel(nextHullPointRelX[0], nextHullPointRelY[0]);
		for (uint32_t i = 1; i < 4; i++) {
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
	.name = "jarvis_wrap_avx",
	.runInt = nullptr,
	.runDouble = &runJarvisWrapAvx,
});
