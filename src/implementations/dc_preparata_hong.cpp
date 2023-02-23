#include "../hull_impl.hpp"
#include "../point.hpp"

#include <algorithm>
#include <vector>

// Implementation based on the 2D Divide & Conquer algorithm by F.P. Preparata and S.J. Hong published in 1977.


// finds the indices of the two tangent points between two convex hulls A and B. sA and sB denote the starting indices, 
// lowerTangent denotes if we are finding the lower or upper tangent.
template <typename T>
std::pair<int,int> find_tangent(const std::vector<point<T>>& A, const std::vector<point<T>>& B, int sA, int sB, bool lowerTangent) {
	int step = 0; 
	if ( A[sA].y <= B[sB].y) {
		step = 1; // we are stepping forwards
	} else {
		step = -1; // we are stepping backwards
	}

	// It is important in which order we step around the A or B hull. If we do this incorrectly we might overstep and find the wrong tangent.
	bool aStepFirst = (step==1) != lowerTangent;

	int i,j;
	i = sA;
	j = sB;
	bool done = false;
	while (!done) {

		if (aStepFirst) {
			int nexti = (i+step +A.size())%A.size();
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
		int nextj = (j+step+B.size())%B.size();
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
		if (!aStepFirst) {
			int nexti = (i+step +A.size())%A.size();
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


template <typename T>
void ch(std::vector<point<T>>& S) {
	int n = S.size();
	if (n<4) { // base case
		if (n == 3) {
			// Check if the 3 points are on a straight line
			side orientation = S[2].sideOfLine(S[0],S[1]);
			if (orientation == side::on) {
					S[1] = S[2];
					S.pop_back();
					return;
			}

			// Make sure in CCW order
			if (orientation == side::right) {
				 std::swap(S[1],S[2]);
			}
		}
		return;
	}
	std::vector<point<T>> A,B;
	A.reserve(n/2); B.reserve(n-n/2);
	std::copy(S.begin(), S.begin()+n/2, std::back_inserter(A));
	std::copy(S.begin()+n/2, S.end(), std::back_inserter(B));
	ch(A);
	ch(B);
	// Uncomment to swap out tangent finding for just generally merging the hulls, is around 3x slower 
	// std::vector<std::vector<point<T>>> Pdiv;
	// Pdiv.push_back(std::move(A)); Pdiv.push_back(std::move(B));
	// S = Hull2DMerge(Pdiv);
	// return;

	// Compute tangents to merge A and B
	size_t lAi = 0; // Lowest point on A
	size_t lBi = 0; // Lowest point on B
	size_t uAi = 0; // highest point on A
	size_t uBi = 0; // Highest point on B
	for (size_t i = 0; i < A.size(); i++) {
		if (A[i].y < A[lAi].y) {
			lAi = i;
		}
		if (A[i].y > A[uAi].y) {
			uAi = i;
		}
	}
	for (size_t i = 0; i < B.size(); i++) {
		if (B[i].y < B[lBi].y) {
			lBi = i;
		}
		if (B[i].y > B[uBi].y) {
			uBi = i;
		}
	}

	auto lower_tangent = find_tangent(A,B,lAi,lBi,true);
	auto upper_tangent = find_tangent(A,B,uAi,uBi,false);
	size_t ltA,ltB,utA,utB;
	ltA = lower_tangent.first;
	ltB = lower_tangent.second;
	utA = upper_tangent.first;
	utB = upper_tangent.second;
	S.clear();
	//Build the hull
	for (size_t i = utA; i != ltA; i=(i+1)%A.size()) {
		S.push_back(A[i]);
	}
	S.push_back(A[ltA]);

	for (size_t i = ltB; i != utB; i=(i+1)%B.size()) {
		S.push_back(B[i]);
	}
	S.push_back(B[utB]);

}



template <typename T>
void runDcPreparataHong(std::vector<point<T>>& pts) {
	if (pts.size() <= 1) return;
	std::sort(pts.begin(), pts.end());
	ch(pts);
}

DEF_HULL_IMPL({
	.name = "dc_preparata_hong",
	.runInt = &runDcPreparataHong<int64_t>,
	.runDouble = &runDcPreparataHong<double>,
});
