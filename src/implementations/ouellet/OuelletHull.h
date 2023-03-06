// File is taken from Eric Ouellets project https://github.com/EricOuellet2/ConvexHull more specifically "OuelletConvexHullCpp" folder
// Small adaptions have been made to make the code work with our project/benchmarking.


#pragma once

#include <math.h>
#include "OuelletPoint.h"

class OuelletHull
{
private:
	static const int _quadrantHullPointArrayInitialCapacity = 1000;
	static const int _quadrantHullPointArrayGrowSize = 1000;

	ouellet_point* _pPoints;
	int _countOfPoint;
	bool _shouldCloseTheGraph;

	ouellet_point* q1pHullPoints;
	ouellet_point* q1pHullLast;
	int q1hullCapacity;
	int q1hullCount = 0;

	ouellet_point* q2pHullPoints;
	ouellet_point* q2pHullLast;
	int q2hullCapacity;
	int q2hullCount = 0;

	ouellet_point* q3pHullPoints;
	ouellet_point* q3pHullLast;
	int q3hullCapacity;
	int q3hullCount = 0;

	ouellet_point* q4pHullPoints;
	ouellet_point* q4pHullLast;
	int q4hullCapacity;
	int q4hullCount = 0;

	void CalcConvexHull();

	inline static void InsertPoint(ouellet_point*& pPoint, int index, ouellet_point& pt, int& count, int& capacity);
	inline static void RemoveRange(ouellet_point* pPoint, int indexStart, int indexEnd, int &count);

public:
	OuelletHull(ouellet_point* points, int countOfPoint, bool shouldCloseTheGraph = true);
	~OuelletHull();
	ouellet_point* GetResultAsArray(int& count);
};

int ouelletHullForTimeCheckOnly(ouellet_point* pArrayOfPoint, int count);

extern "C" 
{
	ouellet_point* ouelletHull(ouellet_point* pArrayOfPoint, int count, bool closeThePath, int& resultCount);
//	array<ManagedPoint>^ ouelletHullManaged(ouellet_point* pArrayOfPoint, int count);
}
