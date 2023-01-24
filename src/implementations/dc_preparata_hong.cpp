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
	if (lowerTangent) {
		if ( A[sA].y <= B[sB].y) {
			step = 1; // we are stepping forwards
		} else {
			step = -1;
		}
	} else {
		if ( A[sA].y <= B[sB].y) {
			step = -1; // we are stepping backwards
		} else {
			step = 1;
		}
	}

	int i,j;
	i = sA;
	j = sB;
	bool done = false;
	while (!done) {
		int nexti = (i+step +A.size())%A.size();
		// If we are building lowerTangent we want to switch from CCW, if we are building upperTanget we want to switch from CW
		if ((A[nexti].cross(B[j],A[i]) > 0 && lowerTangent) || (A[nexti].cross(B[j],A[i]) < 0 && !lowerTangent)) { 
			i = nexti;
			continue;
		} 
		// If colinear pick furthest point from B[j]
		if (A[nexti].cross(B[j],A[i]) == 0 && (A[i]-B[j]).len2() < (A[nexti]-B[j]).len2()) {
			i = nexti;
			continue;
		}
		int nextj = (j+step+B.size())%B.size();
		if ((B[nextj].cross(B[j],A[i]) > 0 && lowerTangent) || (B[nextj].cross(B[j],A[i]) < 0 && !lowerTangent)) {
			j = nextj;
			continue;
		}
		if (B[nextj].cross(B[j],A[i]) == 0 && (B[j]-A[i]).len2() < (B[nextj]-A[i]).len2()) {
			j = nextj;
			continue;
		}
		done = true;
	}
	return std::tie(i,j);
}


template <typename T>
void ch(std::vector<point<T>>& S) {
	int n = S.size();
	if (n<4) { // base case
		if (n == 3) {
			// Check if the 3 points are on a straight line
			if (S[1].cross(S[2],S[0]) == 0) {
					S[1] = S[2];
					S.pop_back();
					return;
			}

			// Make sure in CCW order
			if (S[1].cross(S[2],S[0]) < 0) {
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

	// Compute tangents to merge A and B
	int lAi = 0; // Lowest point on A
	int lBi = 0; // Lowest point on B
	int uAi = 0; // highest point on A
	int uBi = 0; // Highest point on B
	for (int i = 0; i < A.size(); i++) {
		if (A[i].y < A[lAi].y) {
			lAi = i;
		}
		if (A[i].y > A[uAi].y) {
			lAi = i;
		}
	}
	for (int i = 0; i < B.size(); i++) {
		if (B[i].y < B[lBi].y) {
			lBi = i;
		}
		if (B[i].y > B[uBi].y) {
			lBi = i;
		}
	}

	auto lower_tangent = find_tangent(A,B,lAi,lBi,true);
	auto upper_tangent = find_tangent(A,B,uAi,uBi,false);
	int ltA,ltB,utA,utB;
	ltA = lower_tangent.first;
	ltB = lower_tangent.second;
	utA = upper_tangent.first;
	utB = upper_tangent.second;
	S.clear();
	for (int i = utA; i != ltA; i=(i+1)%A.size()) {
		S.push_back(A[i]);
	}
	S.push_back(A[ltA]);

	for (int i = ltB; i != utB; i=(i+1)%B.size()) {
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
