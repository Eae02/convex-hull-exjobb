#include "../hull_impl.hpp"
#include "../point.hpp"

#include <algorithm>
#include <vector>
#include <cmath>
#include <span>
#include <cassert>
#include <iostream>

// Implementation based on the 2D Divide & Conquer algorithm by F.P. Preparata and S.J. Hong published in 1977.

// finds the indices of the two tangent points between two convex hulls A and B. sA and sB denote the starting indices, 
// lowerTangent denotes if we are finding the lower or upper tangent.
template <typename T>
std::pair<size_t,size_t> find_tangent(std::span<point<T>> A, std::span<point<T>> B, size_t sA, size_t sB, bool lowerTangent) {
	int step = 0; 
	if ( A[sA].y <= B[sB].y) {
		step = 1; // we are stepping forwards
	} else {
		step = -1; // we are stepping backwards
	}

	// It is important in which order we step around the A or B hull. If we do this incorrectly we might overstep and find the wrong tangent.
	bool aStepFirst = (step==1) != lowerTangent;

	size_t i,j;
	i = sA;
	j = sB;
	bool done = false;
	while (!done) {
		if (aStepFirst) { // Step on A hull
			size_t nexti = (i+step +A.size())%A.size();
			side orientationA = A[nexti].sideOfLine(B[j],A[i]); // Line pointing leftwards from B to A
			// If we are building lower tangent we want to lower the line (turn left) and vice versa.
			if ((orientationA == side::left && lowerTangent) || (orientationA == side::right && !lowerTangent)) { 
				i = nexti;
				continue;
			} 
			// If colinear pick furthest point from B[j]
			if (orientationA == side::on && (A[i]-B[j]).len2() < (A[nexti]-B[j]).len2()) {
				i = nexti;
				continue;
			}
		}
		{   // Step on B hull
			size_t nextj = (j+step+B.size())%B.size();
			side orientationB = B[nextj].sideOfLine(A[i],B[j]); // Line pointing rightwards from A to B
			// If we are building lower tangent we want to lower the line (turn right) and vice versa.
			if ((orientationB == side::right && lowerTangent) || (orientationB == side::left && !lowerTangent)) {
				j = nextj; 
				continue;
			}
			if (orientationB == side::on && (B[j]-A[i]).len2() < (B[nextj]-A[i]).len2()) {
				j = nextj;
				continue;
			}
		}
		if (!aStepFirst) { // Step on A hull
			size_t nexti = (i+step +A.size())%A.size();
			side orientationA = A[nexti].sideOfLine(B[j],A[i]); // Line pointing leftwards from B to A
			// If we are building lower tangent we want to lower the line (turn left) and vice versa.
			if ((orientationA == side::left && lowerTangent) || (orientationA == side::right && !lowerTangent)) { 
				i = nexti;
				continue;
			} 
			// If colinear pick furthest point from B[j]
			if (orientationA == side::on && (A[i]-B[j]).len2() < (A[nexti]-B[j]).len2()) {
				i = nexti;
				continue;
			}
		}

		done = true;
	}
	return std::make_pair(i,j);
}

// Returns number of points on hull
template <typename T>
size_t ch(std::span<point<T>> pts) {
	size_t n = pts.size();
	if (n<4) { // base case
		if (n == 3) {
			// Check if the 3 points are on a straight line
			side orientation = pts[2].sideOfLine(pts[0],pts[1]);
			if (orientation == side::on) {
					pts[1] = pts[2];
					return 2;
			}

			// Make sure in CCW order
			if (orientation == side::right) {
				 std::swap(pts[1],pts[2]);
				 return 3;
			}
		}
		return n;
	}

	std::span<point<T>> A = pts.subspan(0,n/2);
	std::span<point<T>> B = pts.subspan(n/2,n-n/2);
	size_t szA = ch(A);
	size_t szB = ch(B);
	A = A.subspan(0,szA);
	B = B.subspan(0,szB);
	// Compute tangents to merge A and B
	size_t lAi = 0; // Lowest point on A
	size_t lBi = 0; // Lowest point on B
	size_t uAi = 0; // highest point on A
	size_t uBi = 0; // Highest point on B
	for (size_t i = 0; i < szA; i++) {
		if (A[i].y < A[lAi].y) {
			lAi = i;
		}
		if (A[i].y > A[uAi].y) {
			uAi = i;
		}
	}
	for (size_t i = 0; i < szB; i++) {
		if (B[i].y < B[lBi].y) {
			lBi = i;
		}
		if (B[i].y > B[uBi].y) {
			uBi = i;
		}
	}

	auto lower_tangent = find_tangent(A,B,lAi,lBi,true);
	auto upper_tangent = find_tangent(A,B,uAi,uBi,false);
	// Tangent indices
	size_t ltA,ltB,utA,utB;
	ltA = lower_tangent.first;
	ltB = lower_tangent.second;
	utA = upper_tangent.first;
	utB = upper_tangent.second;
	//Build the hull
	std::vector<point<T>> tmp;
	tmp.push_back(B[utB]);
	if (utA != 0){
		tmp.reserve(szA-utA+1);
		for (size_t i = utA; i < szA; i++) {
			tmp.push_back(A[i]);
		}
	}

	size_t size = ltA+1;
	for (size_t i = ltB; i != utB; i = (i+1)%szB) {
		pts[size] = B[i];
		size++;
	}

	for (auto& pt : tmp) {
		pts[size] = pt;
		size++;
	}

	return size;
	
}



template <typename T>
static void runDcPreparataHong(std::vector<point<T>>& pts) {
	if (pts.size() <= 1) return;
	std::sort(pts.begin(), pts.end());
	size_t sz = ch(std::span<point<T>>(pts.begin(), pts.end()));
	pts.resize(sz);
}

DEF_HULL_IMPL({
	.name = "dc_preparata_hong",
	.runInt = &runDcPreparataHong<int64_t>,
	.runDouble = &runDcPreparataHong<double>,
});
