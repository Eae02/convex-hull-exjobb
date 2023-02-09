#pragma once

#include "../point.hpp"

#include <immintrin.h>
#include <span>

inline __m128i cvtepi64_epi32_avx(__m256d v) {
	__m256 vf = _mm256_castpd_ps(v);
	__m128 hi = _mm256_extractf128_ps(vf, 1);
	__m128 lo = _mm256_castps256_ps128(vf);
	__m128 packed = _mm_shuffle_ps(lo, hi, _MM_SHUFFLE(2, 0, 2, 0));
	return _mm_castps_si128(packed);
}

inline void initPoints256(std::span<const pointd> pts, __m256d* ptsx, __m256d* ptsy, double pad) {
	for (size_t i = 0; i < pts.size() / 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			size_t idx = i * 4 + j;
			ptsx[i][j] = pts[idx].x;
			ptsy[i][j] = pts[idx].y;
		}
	}
	size_t lastVecIdx = pts.size() / 4;
	ptsx[lastVecIdx] = _mm256_set1_pd(pad);
	ptsy[lastVecIdx] = _mm256_set1_pd(pad);
	for (size_t i = lastVecIdx * 4; i < pts.size(); i++) {
		ptsx[lastVecIdx][i % 4] = pts[i].x;
		ptsy[lastVecIdx][i % 4] = pts[i].y;
	}
}

inline void initPoints512(std::span<const pointd> pts, __m512d* ptsx, __m512d* ptsy, double pad) {
	for (size_t i = 0; i < pts.size() / 8; i++) {
		for (size_t j = 0; j < 8; j++) {
			size_t idx = i * 8 + j;
			ptsx[i][j] = pts[idx].x;
			ptsy[i][j] = pts[idx].y;
		}
	}
	size_t lastVecIdx = pts.size() / 8;
	ptsx[lastVecIdx] = _mm512_set1_pd(pad);
	ptsy[lastVecIdx] = _mm512_set1_pd(pad);
	for (size_t i = lastVecIdx * 8; i < pts.size(); i++) {
		ptsx[lastVecIdx][i % 8] = pts[i].x;
		ptsy[lastVecIdx][i % 8] = pts[i].y;
	}
}

inline __m256d abs256(__m256d val) {
	static const __m256d mask = _mm256_castsi256_pd(_mm256_set1_epi64x((1LLU << 63LLU) - 1));
	return _mm256_and_pd(mask, val);
}
