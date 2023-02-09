#include "../hull_impl.hpp"
#include "../point.hpp"

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

#include <immintrin.h>

struct points {
	__m512d* x;
	__m512d* y;
	size_t count;
	
	points trimSuffix(size_t trim) const {
		return points { x, y, count - trim };
	}
	
	void print(int prefix = 0) const {
		return;
		std::cerr << std::string(prefix * 2, ' ');
		for (size_t i = 0; i < count; i++) {
			std::cerr << "(" << std::fixed << std::setprecision(0) << x[i/8][i%8] << " " << y[i/8][i%8] << ") ";
		}
		std::cerr << "\n";
	}
	
	void copyPoint(size_t src, size_t dst) {
		x[dst / 8][dst % 8] = x[src / 8][src % 8];
		y[dst / 8][dst % 8] = y[src / 8][src % 8];
	}
};

size_t partitionByLine(const points& ptsIn, points& ptsOut, pointd lineStart, pointd lineEnd) {
	assert(ptsOut.count >= ptsIn.count);
	ptsOut.count = ptsIn.count;
	
	const auto lineStartX8 = _mm512_set1_pd(lineStart.x);
	const auto lineStartY8 = _mm512_set1_pd(lineStart.y);
	const auto lineDeltaX8 = _mm512_set1_pd(lineEnd.x - lineStart.x);
	const auto lineDeltaY8 = _mm512_set1_pd(lineEnd.y - lineStart.y);
	
	size_t numRight = 0;
	double* outXR = reinterpret_cast<double*>(ptsOut.x);
	double* outYR = reinterpret_cast<double*>(ptsOut.y);
	double* outXL = outXR + ptsOut.count;
	double* outYL = outYR + ptsOut.count;
	
	auto step = [&] (size_t i, uint8_t mask) {
		__m512d x = ptsIn.x[i];
		__m512d y = ptsIn.y[i];
		
		auto relx = _mm512_sub_pd(x, lineStartX8);
		auto rely = _mm512_sub_pd(y, lineStartY8);
		
		uint8_t maskR = mask & _mm512_cmplt_pd_mask(
			_mm512_mul_pd(rely, lineDeltaX8),
			_mm512_mul_pd(relx, lineDeltaY8)
		);
		uint8_t maskL = mask & ~maskR;
		
		size_t numL = __builtin_popcount(maskL);
		size_t numR = __builtin_popcount(maskR);
		outXL -= numL;
		outYL -= numL;
		
		_mm512_mask_compressstoreu_pd(outXR, maskR, x);
		_mm512_mask_compressstoreu_pd(outYR, maskR, y);
		_mm512_mask_compressstoreu_pd(outXL, maskL, x);
		_mm512_mask_compressstoreu_pd(outYL, maskL, y);
		
		outXR += numR;
		outYR += numR;
		numRight += numR;
	};
	
	for (size_t i = 0; i < ptsIn.count / 8; i++) {
		step(i, 0xFF);
	}
	if (ptsIn.count % 8) {
		step(ptsIn.count / 8, (1 << (ptsIn.count % 8)) - 1);
	}
	
	return numRight;
}

int findMaxPointIndex(const points& pts, pointd normal) {
	const auto normalX8 = _mm512_set1_pd(normal.x);
	const auto normalY8 = _mm512_set1_pd(normal.y);
	
	__m256i indices = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
	const __m256i indicesInc = _mm256_set1_epi32(8);
	
	auto maxDotValues = _mm512_set1_pd(-INFINITY);
	auto maxIndices = _mm256_set1_epi32(0);
	
	auto step = [&] (size_t i, uint8_t mask) {
		auto mulx = _mm512_mul_pd(pts.x[i], normalX8);
		auto muly = _mm512_mul_pd(pts.y[i], normalY8);
		auto dot = _mm512_add_pd(mulx, muly);
		uint8_t cmpMask = mask & _mm512_cmplt_pd_mask(maxDotValues, dot);
		maxDotValues = _mm512_mask_blend_pd(cmpMask, maxDotValues, dot);
		maxIndices = _mm256_mask_blend_epi32(cmpMask, maxIndices, indices);
		indices = _mm256_add_epi32(indices, indicesInc);
	};
	
	for (size_t i = 0; i < pts.count / 8; i++) {
		step(i, 0xFF);
	}
	if (pts.count % 8) {
		step(pts.count / 8, (1 << (pts.count % 8)) - 1);
	}
	
	alignas(32) int maxIndicesBuffer[8];
	_mm256_store_epi32(maxIndicesBuffer, maxIndices);
	
	std::pair<double, int> maxPoint(-INFINITY, -1);
	for (int i = 0; i < 8; i++) {
		maxPoint = std::max(maxPoint, std::make_pair(maxDotValues[i], maxIndicesBuffer[i]));
	}
	
	return maxPoint.second;
}

void quickhullAvxRec(
	points pts, points tmppts, pointd leftHullPoint, pointd rightHullPoint, std::vector<pointd>& output, int depth = 1
) {
	if (pts.count <= 1) {
		if (pts.count == 1 && !std::isinf(pts.x[0][0])) {
			output.emplace_back(pts.x[0][0], pts.y[0][0]);
		}
		return;
	}
	
	const pointd normal = (rightHullPoint - leftHullPoint).rotated90CCW();
	size_t maxPointIndex = findMaxPointIndex(pts, normal);
	
	pointd maxPoint(pts.x[maxPointIndex / 8][maxPointIndex % 8], pts.y[maxPointIndex / 8][maxPointIndex % 8]);
	
	pts.copyPoint(pts.count - 1, maxPointIndex);
	pts.count--;
	
	size_t numPointsRight = partitionByLine(pts, tmppts, rightHullPoint, maxPoint);
	
	tmppts.print(depth);
	
	size_t borderVecIdx = numPointsRight / 8;
	size_t numRightLast = numPointsRight % 8;
	uint8_t borderVecRMask = (1 << numRightLast) - 1;
	auto lastVecX = tmppts.x[borderVecIdx];
	auto lastVecY = tmppts.y[borderVecIdx];
	
	quickhullAvxRec({ tmppts.x, tmppts.y, numPointsRight }, pts, maxPoint, rightHullPoint, output, depth + 1);
	
	output.push_back(maxPoint);
	
	tmppts.x[borderVecIdx] = _mm512_mask_blend_pd(borderVecRMask, lastVecX, _mm512_set1_pd(maxPoint.x - normal.x));
	tmppts.y[borderVecIdx] = _mm512_mask_blend_pd(borderVecRMask, lastVecY, _mm512_set1_pd(maxPoint.y - normal.y));
	
	size_t numPointsLeft = partitionByLine(points {
		.x = tmppts.x + borderVecIdx,
		.y = tmppts.y + borderVecIdx,
		.count = tmppts.count - borderVecIdx * 8
	}, pts, maxPoint, leftHullPoint);
	
	quickhullAvxRec({ pts.x, pts.y, numPointsLeft }, tmppts, leftHullPoint, maxPoint, output, depth + 1);
}

std::pair<int, int> findMinMax(const points& pts) {
	auto maxX = _mm512_set1_pd(-INFINITY);
	auto maxY = _mm512_set1_pd(-INFINITY);
	auto maxIndices = _mm256_set1_epi32(0);
	auto minX = _mm512_set1_pd(INFINITY);
	auto minY = _mm512_set1_pd(INFINITY);
	auto minIndices = _mm256_set1_epi32(0);
	
	__m256i indices = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
	const __m256i indicesInc = _mm256_set1_epi32(8);
	
	auto step = [&] (size_t i, uint8_t mask) {
		auto x = pts.x[i];
		auto y = pts.y[i];
		
		uint8_t maxCmpMask =
			mask & (_mm512_cmplt_pd_mask(maxX, x) | (_mm512_cmpeq_pd_mask(maxX, x) & _mm512_cmplt_pd_mask(maxY, y)));
		maxX = _mm512_mask_blend_pd(maxCmpMask, maxX, x);
		maxY = _mm512_mask_blend_pd(maxCmpMask, maxY, y);
		maxIndices = _mm256_mask_blend_epi32(maxCmpMask, maxIndices, indices);
		
		uint8_t minCmpMask =
			mask & (_mm512_cmplt_pd_mask(x, minX) | (_mm512_cmpeq_pd_mask(x, minX) & _mm512_cmplt_pd_mask(y, minY)));
		minX = _mm512_mask_blend_pd(minCmpMask, minX, x);
		minY = _mm512_mask_blend_pd(minCmpMask, minY, y);
		minIndices = _mm256_mask_blend_epi32(minCmpMask, minIndices, indices);
		
		indices = _mm256_add_epi32(indices, indicesInc);
	};
	
	for (size_t i = 0; i < pts.count / 8; i++) {
		step(i, 0xFF);
	}
	if (pts.count % 8) {
		step(pts.count / 8, (1 << (pts.count % 8)) - 1);
	}
	
	alignas(32) int minIndicesBuffer[8];
	_mm256_store_epi32(minIndicesBuffer, minIndices);
	alignas(32) int maxIndicesBuffer[8];
	_mm256_store_epi32(maxIndicesBuffer, maxIndices);
	
	std::tuple<double, double, int> maxPoint(-INFINITY, -INFINITY, -1);
	std::tuple<double, double, int> minPoint(INFINITY, INFINITY, -1);
	for (int i = 0; i < 8; i++) {
		minPoint = std::min(minPoint, std::make_tuple(minX[i], minY[i], minIndicesBuffer[i]));
		maxPoint = std::max(maxPoint, std::make_tuple(maxX[i], maxY[i], maxIndicesBuffer[i]));
	}
	
	return { std::get<2>(minPoint), std::get<2>(maxPoint) };
}

void runQuickhullAvx(std::vector<pointd>& pts) {
	size_t bufferSize = ((pts.size() * 2 * sizeof(double)) + 128) & ~63;
	char* buffer = static_cast<char*>(std::aligned_alloc(64, bufferSize * 4));
	
	__m512d* ptsx = reinterpret_cast<__m512d*>(buffer);
	__m512d* ptsy = reinterpret_cast<__m512d*>(buffer + bufferSize);
	__m512d* ptstmpx = reinterpret_cast<__m512d*>(buffer + bufferSize * 2);
	__m512d* ptstmpy = reinterpret_cast<__m512d*>(buffer + bufferSize * 3);
	
	for (size_t i = 0; i < pts.size() / 8; i++) {
		for (size_t j = 0; j < 8; j++) {
			size_t idx = i * 8 + j;
			ptsx[i][j] = pts[idx].x;
			ptsy[i][j] = pts[idx].y;
		}
	}
	size_t lastVecIdx = pts.size() / 8;
	ptsx[lastVecIdx] = _mm512_set1_pd(-INFINITY);
	ptsy[lastVecIdx] = _mm512_set1_pd(-INFINITY);
	for (size_t i = lastVecIdx * 8; i < pts.size(); i++) {
		ptsx[lastVecIdx][i % 8] = pts[i].x;
		ptsy[lastVecIdx][i % 8] = pts[i].y;
	}
	
	points ptsSpan = { ptsx, ptsy, pts.size() };
	points ptsTmpSpan = { ptstmpx, ptstmpy, pts.size() };
	
	auto [leftmostIdx, rightmostIdx] = findMinMax(ptsSpan);
	
	pointd leftmostPt = pts[leftmostIdx];
	pointd rightmostPt = pts[rightmostIdx];
	
	ptsSpan.copyPoint(pts.size() - 1, std::max(leftmostIdx, rightmostIdx));
	ptsSpan.copyPoint(pts.size() - 2, std::min(leftmostIdx, rightmostIdx));
	ptsSpan.count -= 2;
	
	size_t numPointsBelow = partitionByLine(ptsSpan, ptsTmpSpan, leftmostPt, rightmostPt);
	
	size_t borderVecIdx = numPointsBelow / 8;
	size_t numBelowLast = numPointsBelow % 8;
	uint8_t borderVecBMask = (1 << numBelowLast) - 1;
	auto lastVecX = ptsTmpSpan.x[borderVecIdx];
	auto lastVecY = ptsTmpSpan.y[borderVecIdx];
	
	size_t numPointsAbove = ptsTmpSpan.count - numPointsBelow + numBelowLast;
	
	pts.clear();
	pts.push_back(leftmostPt);
	
	quickhullAvxRec({ ptsTmpSpan.x, ptsTmpSpan.y, numPointsBelow }, ptsSpan, rightmostPt, leftmostPt, pts);
	
	pts.push_back(rightmostPt);
	
	ptsTmpSpan.x[borderVecIdx] = _mm512_mask_blend_pd(borderVecBMask, lastVecX, _mm512_set1_pd(-INFINITY));
	ptsTmpSpan.y[borderVecIdx] = _mm512_mask_blend_pd(borderVecBMask, lastVecY, _mm512_set1_pd(-INFINITY));
	
	quickhullAvxRec({ ptsTmpSpan.x + borderVecIdx, ptsTmpSpan.y + borderVecIdx, numPointsAbove }, ptsSpan, leftmostPt, rightmostPt, pts);
	
	std::free(buffer);
}

DEF_HULL_IMPL({
	.name = "qh_avx512",
	.runInt = nullptr,
	.runDouble = runQuickhullAvx,
});
