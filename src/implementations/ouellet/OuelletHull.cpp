// File is taken from Eric Ouellets project https://github.com/EricOuellet2/ConvexHull more specifically "OuelletConvexHullCpp" folder
// Small adaptions have been made to make the code work with our project/benchmarking.

// Initial benchmarking shows very good performance on square dataset (on par with qh_rec), but extremely poor on circle (x400 execution time compared to monotone_chain for 1M points). 

#include "OuelletHull.h"
#include "OuelletPoint.h"
#include <string.h>
#include <omp.h>

#include "../../hull_impl.hpp"
#include "../../point.hpp"



void runOuellet(std::vector<point<double>>& pts) {
	int count = pts.size();
	point<double>* start = &pts.front();
	ouellet_point* ouellet_start = reinterpret_cast<ouellet_point*>(start);
	int resultCount;
	OuelletHull convexHull(ouellet_start, count, false);
	ouellet_point* ouellet_output = convexHull.GetResultAsArray(resultCount);
	point<double>* output = reinterpret_cast<point<double>*>(ouellet_output);
	pts.clear();
	
	std::copy(output, output+resultCount, std::back_inserter(pts));
}


DEF_HULL_IMPL({
	.name = "ouellet",
	.runInt = nullptr,
	.runDouble = &runOuellet,
});



// **************************************************************************
extern "C" ouellet_point* ouelletHull(ouellet_point* pArrayOfPoint, int count, bool closeThePath, int& resultCount)
{
	OuelletHull convexHull(pArrayOfPoint, count, closeThePath);
	return convexHull.GetResultAsArray(resultCount);
}

// **************************************************************************
int ouelletHullForTimeCheckOnly(ouellet_point* pArrayOfPoint, int count)
{
	int resultCount;
	OuelletHull convexHull(pArrayOfPoint, count, true);
	convexHull.GetResultAsArray(resultCount);
	return resultCount;
}

// **************************************************************************
void OuelletHull::CalcConvexHull()
{
	// Find the quadrant limits (maximum x and y)
	ouellet_point* pPt = _pPoints;

	ouellet_point q1p1 = *pPt;
	ouellet_point q1p2 = *pPt;
	ouellet_point q2p1 = *pPt;
	ouellet_point q2p2 = *pPt;
	ouellet_point q3p1 = *pPt;
	ouellet_point q3p2 = *pPt;
	ouellet_point q4p1 = *pPt;
	ouellet_point q4p2 = *pPt;

	pPt++;

	for (int n = _countOfPoint - 1; n > 0; n--) // -1 because 0 bound.
	{
		ouellet_point& pt = *pPt;

		// Right
		if (pt.x >= q1p1.x)
		{
			if (pt.x == q1p1.x)
			{
				if (pt.y > q1p1.y)
				{
					q1p1 = pt;
				}
				else
				{
					if (pt.y < q4p2.y)
					{
						q4p2 = pt;
					}
				}
			}
			else
			{
				q1p1 = pt;
				q4p2 = pt;
			}
		}

		// Left
		if (pt.x <= q2p2.x)
		{
			if (pt.x == q2p2.x)
			{
				if (pt.y > q2p2.y)
				{
					q2p2 = pt;
				}
				else
				{
					if (pt.y < q3p1.y)
					{
						q3p1 = pt;
					}
				}
			}
			else
			{
				q2p2 = pt;
				q3p1 = pt;
			}
		}

		// Top
		if (pt.y >= q1p2.y)
		{
			if (pt.y == q1p2.y)
			{
				if (pt.x < q2p1.x)
				{
					q2p1 = pt;
				}
				else
				{
					if (pt.x > q1p2.x)
					{
						q1p2 = pt;
					}
				}
			}
			else
			{
				q1p2 = pt;
				q2p1 = pt;
			}
		}

		// Bottom
		if (pt.y <= q3p2.y)
		{
			if (pt.y == q3p2.y)
			{
				if (pt.x < q3p2.x)
				{
					q3p2 = pt;
				}
				else
				{
					if (pt.x > q4p1.x)
					{
						q4p1 = pt;
					}
				}
			}
			else
			{
				q3p2 = pt;
				q4p1 = pt;
			}
		}

		pPt++;
	}

	ouellet_point q1rootPt = { q1p2.x, q1p1.y };
	ouellet_point q2rootPt = { q2p1.x, q2p2.y };
	ouellet_point q3rootPt = { q3p2.x, q3p1.y };
	ouellet_point q4rootPt = { q4p1.x, q4p2.y };

	// *************************
	// Q1 Init
	// *************************

	q1hullCapacity = _quadrantHullPointArrayInitialCapacity;
	q1pHullPoints = new ouellet_point[q1hullCapacity];

	q1pHullPoints[0] = q1p1;
	if (compare_points(q1p1, q1p2))
	{
		q1hullCount = 1;
	}
	else
	{
		q1pHullPoints[1] = q1p2;
		q1hullCount = 2;
	}

	// *************************
	// Q2 Init
	// *************************

	q2hullCapacity = _quadrantHullPointArrayInitialCapacity;
	q2pHullPoints = new ouellet_point[q2hullCapacity];

	q2pHullPoints[0] = q2p1;
	if (compare_points(q2p1, q2p2))
	{
		q2hullCount = 1;
	}
	else
	{
		q2pHullPoints[1] = q2p2;
		q2hullCount = 2;
	}

	// *************************
	// Q3 Init
	// *************************

	q3hullCapacity = _quadrantHullPointArrayInitialCapacity;
	q3pHullPoints = new ouellet_point[q3hullCapacity];

	q3pHullPoints[0] = q3p1;
	if (compare_points(q3p1, q3p2))
	{
		q3hullCount = 1;
	}
	else
	{
		q3pHullPoints[1] = q3p2;
		q3hullCount = 2;
	}

	// *************************
	// Q4 Init
	// *************************

	q4hullCapacity = _quadrantHullPointArrayInitialCapacity;
	q4pHullPoints = new ouellet_point[q4hullCapacity];

	q4pHullPoints[0] = q4p1;
	if (compare_points(q4p1, q4p2))
	{
		q4hullCount = 1;
	}
	else
	{
		q4pHullPoints[1] = q4p2;
		q4hullCount = 2;
	}
	
	// *************************
	// Start Calc	
	// *************************

	// Calc per quadrant
	int index;
	int indexLow;
	int indexHi;

	// Currently hardcoded, could be calculated or pass as argument by user, dynamic, grow as needed

	pPt = _pPoints;

	for (int n = _countOfPoint - 1; n >= 0; n--) // -1 because 0 bound.
	{
		ouellet_point& pt = *pPt;

		// ****************************************************************
		// Q1 Calc
		// ****************************************************************

		// Begin get insertion ouellet_point
		if (pt.x > q1rootPt.x && pt.y > q1rootPt.y) // Is ouellet_point is in Q1
		{
			indexLow = 0;
			indexHi = q1hullCount;

			while (indexLow < indexHi - 1)
			{
				index = ((indexHi - indexLow) >> 1) + indexLow;

				if (pt.x <= q1pHullPoints[index].x && pt.y <= q1pHullPoints[index].y)
				{
					goto currentPointNotPartOfq1Hull; // No calc needed
				}

				if (pt.x > q1pHullPoints[index].x)
				{
					indexHi = index;
					continue;
				}

				if (pt.x < q1pHullPoints[index].x)
				{
					indexLow = index;
					continue;
				}

				indexLow = index - 1;
				indexHi = index + 1;
				break;
			}

			// Here indexLow should contains the index where the ouellet_point should be inserted 
			// if calculation does not invalidate it.

			if (!right_turn(q1pHullPoints[indexLow], q1pHullPoints[indexHi], pt))
			{
				goto currentPointNotPartOfq1Hull;
			}

			// HERE: We should insert a new candidate as a Hull ouellet_point (until a new one could invalidate this one, if any).

			// indexLow is the index of the ouellet_point before the place where the new ouellet_point should be inserted as the new candidate of ConveHull ouellet_point.
			// indexHi is the index of the ouellet_point after the place where the new ouellet_point should be inserted as the new candidate of ConveHull ouellet_point.
			// But indexLow and indexHi can change because it could invalidate many points before or after.

			// Find lower bound (remove ouellet_point invalidate by the new one that come before)
			while (indexLow > 0)
			{
				if (right_turn(q1pHullPoints[indexLow - 1], pt, q1pHullPoints[indexLow]))
				{
					break; // We found the lower index limit of points to keep. The new ouellet_point should be added right after indexLow.
				}
				indexLow--;
			}

			// Find upper bound (remove ouellet_point invalidate by the new one that come after)
			int maxIndexHi = q1hullCount - 1;
			while (indexHi < maxIndexHi)
			{
				if (right_turn(pt, q1pHullPoints[indexHi + 1], q1pHullPoints[indexHi]))
				{
					break; // We found the higher index limit of points to keep. The new ouellet_point should be added right before indexHi.
				}
				indexHi++;
			}

			if (indexLow + 1 == indexHi)
			{
				InsertPoint(q1pHullPoints, indexLow + 1, pt, q1hullCount, q1hullCapacity);

				goto nextPoint;
			}
			else if (indexLow + 2 == indexHi) // Don't need to insert, just replace at index + 1
			{
				q1pHullPoints[indexLow + 1] = *pPt;
				goto nextPoint;
			}
			else
			{
				q1pHullPoints[indexLow + 1] = *pPt;
				RemoveRange(q1pHullPoints, indexLow + 2, indexHi -1, q1hullCount);
				goto nextPoint;
			}
		}

	currentPointNotPartOfq1Hull:

		// ****************************************************************
		// Q2 Calc
		// ****************************************************************

		// Begin get insertion ouellet_point
		if (pt.x < q2rootPt.x && pt.y > q2rootPt.y) // Is ouellet_point is in q2
		{
			indexLow = 0;
			indexHi = q2hullCount;

			while (indexLow < indexHi - 1)
			{
				index = ((indexHi - indexLow) >> 1) + indexLow;

				if (pt.x >= q2pHullPoints[index].x && pt.y <= q2pHullPoints[index].y)
				{
					goto currentPointNotPartOfq2Hull; // No calc needed
				}

				if (pt.x > q2pHullPoints[index].x)
				{
					indexHi = index;
					continue;
				}

				if (pt.x < q2pHullPoints[index].x)				{
					indexLow = index;
					continue;
				}

				indexLow = index - 1;
				indexHi = index + 1;
				break;
			}

			// Here indexLow should contains the index where the ouellet_point should be inserted 
			// if calculation does not invalidate it.

			if (!right_turn(q2pHullPoints[indexLow], q2pHullPoints[indexHi], pt))
			{
				goto currentPointNotPartOfq2Hull;
			}

			// HERE: We should insert a new candidate as a Hull ouellet_point (until a new one could invalidate this one, if any).

			// indexLow is the index of the ouellet_point before the place where the new ouellet_point should be inserted as the new candidate of ConveHull ouellet_point.
			// indexHi is the index of the ouellet_point after the place where the new ouellet_point should be inserted as the new candidate of ConveHull ouellet_point.
			// But indexLow and indexHi can change because it could invalidate many points before or after.

			// Find lower bound (remove ouellet_point invalidate by the new one that come before)
			while (indexLow > 0)
			{
				if (right_turn(q2pHullPoints[indexLow - 1], pt, q2pHullPoints[indexLow]))
				{
					break; // We found the lower index limit of points to keep. The new ouellet_point should be added right after indexLow.
				}
				indexLow--;
			}

			// Find upper bound (remove ouellet_point invalidate by the new one that come after)
			int maxIndexHi = q2hullCount - 1;
			while (indexHi < maxIndexHi)
			{
				if (right_turn(pt, q2pHullPoints[indexHi + 1], q2pHullPoints[indexHi]))
				{
					break; // We found the higher index limit of points to keep. The new ouellet_point should be added right before indexHi.
				}
				indexHi++;
			}

			if (indexLow + 1 == indexHi)
			{
				InsertPoint(q2pHullPoints, indexLow + 1, pt, q2hullCount, q2hullCapacity);

				goto nextPoint;
			}
			else if (indexLow + 2 == indexHi) // Don't need to insert, just replace at index + 1
			{
				q2pHullPoints[indexLow + 1] = *pPt;
				goto nextPoint;
			}
			else
			{
				q2pHullPoints[indexLow + 1] = *pPt;
				RemoveRange(q2pHullPoints, indexLow + 2, indexHi - 1, q2hullCount);
				goto nextPoint;
			}
		}

	currentPointNotPartOfq2Hull:

		// ****************************************************************
		// Q3 Calc
		// ****************************************************************

		// Begin get insertion ouellet_point
		if (pt.x < q3rootPt.x && pt.y < q3rootPt.y) // Is ouellet_point is in q3
		{
			indexLow = 0;
			indexHi = q3hullCount;

			while (indexLow < indexHi - 1)
			{
				index = ((indexHi - indexLow) >> 1) + indexLow;

				if (pt.x >= q3pHullPoints[index].x && pt.y >= q3pHullPoints[index].y)
				{
					goto currentPointNotPartOfq3Hull; // No calc needed
				}

				if (pt.x < q3pHullPoints[index].x)
				{
					indexHi = index;
					continue;
				}

				if (pt.x > q3pHullPoints[index].x)
				{
					indexLow = index;
					continue;
				}

				indexLow = index - 1;
				indexHi = index + 1;
				break;
			}

			// Here indexLow should contains the index where the ouellet_point should be inserted 
			// if calculation does not invalidate it.

			if (!right_turn(q3pHullPoints[indexLow], q3pHullPoints[indexHi], pt))
			{
				goto currentPointNotPartOfq3Hull;
			}

			// HERE: We should insert a new candidate as a Hull ouellet_point (until a new one could invalidate this one, if any).

			// indexLow is the index of the ouellet_point before the place where the new ouellet_point should be inserted as the new candidate of ConveHull ouellet_point.
			// indexHi is the index of the ouellet_point after the place where the new ouellet_point should be inserted as the new candidate of ConveHull ouellet_point.
			// But indexLow and indexHi can change because it could invalidate many points before or after.

			// Find lower bound (remove ouellet_point invalidate by the new one that come before)
			while (indexLow > 0)
			{
				if (right_turn(q3pHullPoints[indexLow - 1], pt, q3pHullPoints[indexLow]))
				{
					break; // We found the lower index limit of points to keep. The new ouellet_point should be added right after indexLow.
				}
				indexLow--;
			}

			// Find upper bound (remove ouellet_point invalidate by the new one that come after)
			int maxIndexHi = q3hullCount - 1;
			while (indexHi < maxIndexHi)
			{
				if (right_turn(pt, q3pHullPoints[indexHi + 1], q3pHullPoints[indexHi]))
				{
					break; // We found the higher index limit of points to keep. The new ouellet_point should be added right before indexHi.
				}
				indexHi++;
			}

			if (indexLow + 1 == indexHi)
			{
				InsertPoint(q3pHullPoints, indexLow + 1, pt, q3hullCount, q3hullCapacity);

				goto nextPoint;
			}
			else if (indexLow + 2 == indexHi) // Don't need to insert, just replace at index + 1
			{
				q3pHullPoints[indexLow + 1] = *pPt;
				goto nextPoint;
			}
			else
			{
				q3pHullPoints[indexLow + 1] = *pPt;
				RemoveRange(q3pHullPoints, indexLow + 2, indexHi - 1, q3hullCount);
				goto nextPoint;
			}
		}

	currentPointNotPartOfq3Hull:

		// ****************************************************************
		// Q4 Calc
		// ****************************************************************

		// Begin get insertion ouellet_point
		if (pt.x > q4rootPt.x && pt.y < q4rootPt.y) // Is ouellet_point is in q4
		{
			indexLow = 0;
			indexHi = q4hullCount;

			while (indexLow < indexHi - 1)
			{
				index = ((indexHi - indexLow) >> 1) + indexLow;

				if (pt.x <= q4pHullPoints[index].x && pt.y >= q4pHullPoints[index].y)
				{
					goto currentPointNotPartOfq4Hull; // No calc needed
				}

				if (pt.x < q4pHullPoints[index].x)
				{
					indexHi = index;
					continue;
				}

				if (pt.x > q4pHullPoints[index].x)
				{
					indexLow = index;
					continue;
				}

				indexLow = index - 1;
				indexHi = index + 1;
				break;
			}

			// Here indexLow should contains the index where the ouellet_point should be inserted 
			// if calculation does not invalidate it.

			if (!right_turn(q4pHullPoints[indexLow], q4pHullPoints[indexHi], pt))
			{
				goto currentPointNotPartOfq4Hull;
			}

			// HERE: We should insert a new candidate as a Hull ouellet_point (until a new one could invalidate this one, if any).

			// indexLow is the index of the ouellet_point before the place where the new ouellet_point should be inserted as the new candidate of ConveHull ouellet_point.
			// indexHi is the index of the ouellet_point after the place where the new ouellet_point should be inserted as the new candidate of ConveHull ouellet_point.
			// But indexLow and indexHi can change because it could invalidate many points before or after.

			// Find lower bound (remove ouellet_point invalidate by the new one that come before)
			while (indexLow > 0)
			{
				if (right_turn(q4pHullPoints[indexLow - 1], pt, q4pHullPoints[indexLow]))
				{
					break; // We found the lower index limit of points to keep. The new ouellet_point should be added right after indexLow.
				}
				indexLow--;
			}

			// Find upper bound (remove ouellet_point invalidate by the new one that come after)
			int maxIndexHi = q4hullCount - 1;
			while (indexHi < maxIndexHi)
			{
				if (right_turn(pt, q4pHullPoints[indexHi + 1], q4pHullPoints[indexHi]))
				{
					break; // We found the higher index limit of points to keep. The new ouellet_point should be added right before indexHi.
				}
				indexHi++;
			}

			if (indexLow + 1 == indexHi)
			{
				InsertPoint(q4pHullPoints, indexLow + 1, pt, q4hullCount, q4hullCapacity);

				goto nextPoint;
			}
			else if (indexLow + 2 == indexHi) // Don't need to insert, just replace at index + 1
			{
				q4pHullPoints[indexLow + 1] = *pPt;
				goto nextPoint;
			}
			else
			{
				q4pHullPoints[indexLow + 1] = *pPt;
				RemoveRange(q4pHullPoints, indexLow + 2, indexHi - 1, q4hullCount);
				goto nextPoint;
			}
		}

	currentPointNotPartOfq4Hull:
		
		// *************************************** All quadrant are done

	nextPoint:
		pPt++;
	}

}

// **************************************************************************
void OuelletHull::InsertPoint(ouellet_point*& pPoint, int index, ouellet_point& pt, int& count, int& capacity)
{
	// make some room to insert the ouellet_point. make sure to not reach capacity and/or adjust it
	if (count >= capacity)
	{
		// Should make some room
		//int newCapacity = capacity + _quadrantHullPointArrayGrowSize; // Very bad in the worse case. Fallback to regular way of growing list capacity
		int newCapacity = capacity * 2;
		ouellet_point* newPointArray = new ouellet_point[newCapacity];
		memmove(newPointArray, pPoint, capacity * sizeof(ouellet_point));
		delete pPoint;
		pPoint = newPointArray;
		capacity = newCapacity;
	}
	
	memmove(&(pPoint[index + 1]), &(pPoint[index]), (count - index) * sizeof(ouellet_point));

	// Insert ouellet_point at index 
	pPoint[index] = pt;
	count++;
}

// **************************************************************************
/// Remove every item in from index start to indexEnd inclusive 
void OuelletHull::RemoveRange(ouellet_point* pPoint, int indexStart, int indexEnd, int &count)
{
	memmove(&(pPoint[indexStart]), &(pPoint[indexEnd + 1]), (count - indexEnd) * sizeof(ouellet_point));
	count -= (indexEnd - indexStart + 1);
}

// **************************************************************************
OuelletHull::OuelletHull(ouellet_point* points, int countOfPoint, bool shouldCloseTheGraph)
{
	_pPoints = points;
	_countOfPoint = countOfPoint;
	_shouldCloseTheGraph = shouldCloseTheGraph;

	CalcConvexHull();
}

// **************************************************************************
OuelletHull::~OuelletHull()
{
	delete q1pHullPoints;
	delete q2pHullPoints;
	delete q3pHullPoints;
	delete q4pHullPoints;
}

// **************************************************************************
ouellet_point* OuelletHull::GetResultAsArray(int& hullPointCount)
{
	hullPointCount = 0;
	if (this->_countOfPoint == 0)
	{
		return NULL;
	}

	unsigned int indexQ1Start;
	unsigned int indexQ2Start;
	int indexQ3Start;
	int indexQ4Start;
	int indexQ1End;
	int indexQ2End;
	int indexQ3End;
	int indexQ4End;

	indexQ1Start = 0;
	indexQ1End = q1hullCount - 1;
	ouellet_point pointLast = q1pHullPoints[indexQ1End];

	if (q2hullCount == 1)
	{
		if (compare_points(*q2pHullPoints, pointLast)) // 
		{
			indexQ2Start = 1;
			indexQ2End = 0;
		}
		else
		{
			indexQ2Start = 0;
			indexQ2End = 0;
			pointLast = *q2pHullPoints;
		}
	}
	else
	{
		if (compare_points(*q2pHullPoints, pointLast))
		{
			indexQ2Start = 1;
		}
		else
		{
			indexQ2Start = 0;
		}
		indexQ2End = q2hullCount - 1;
		pointLast = q2pHullPoints[indexQ2End];
	}

	if (q3hullCount == 1)
	{
		if (compare_points(*q3pHullPoints, pointLast))
		{
			indexQ3Start = 1;
			indexQ3End = 0;
		}
		else
		{
			indexQ3Start = 0;
			indexQ3End = 0;
			pointLast = *q3pHullPoints;
		}
	}
	else
	{
		if (compare_points(*q3pHullPoints, pointLast))
		{
			indexQ3Start = 1;
		}
		else
		{
			indexQ3Start = 0;
		}
		indexQ3End = q3hullCount - 1;
		pointLast = q3pHullPoints[indexQ3End];
	}

	if (q4hullCount == 1)
	{
		if (compare_points(*q4pHullPoints, pointLast))
		{
			indexQ4Start = 1;
			indexQ4End = 0;
		}
		else
		{
			indexQ4Start = 0;
			indexQ4End = 0;
			pointLast = *q4pHullPoints;
		}
	}
	else
	{
		if (compare_points(*q4pHullPoints, pointLast))
		{
			indexQ4Start = 1;
		}
		else
		{
			indexQ4Start = 0;
		}

		indexQ4End = q4hullCount - 1;
		pointLast = q4pHullPoints[indexQ4End];
	}

	if (compare_points(q1pHullPoints[indexQ1Start], pointLast))
	{
		indexQ1Start++;
	}

	int countOfFinalHullPoint = (indexQ1End - indexQ1Start) +
		(indexQ2End - indexQ2Start) +
		(indexQ3End - indexQ3Start) +
		(indexQ4End - indexQ4Start) + 4;

	if (countOfFinalHullPoint <= 1) // Case where there is only one ouellet_point or many of only the same ouellet_point. Auto closed if required.
	{
		return new ouellet_point[1]{ pointLast };
	}

	if (countOfFinalHullPoint > 1 && _shouldCloseTheGraph)
	{
		countOfFinalHullPoint++;
	}

	ouellet_point* results = new ouellet_point[countOfFinalHullPoint];

	int resIndex = 0;

	for (int n = indexQ1Start; n <= indexQ1End; n++)
	{
		results[resIndex] = q1pHullPoints[n];
		resIndex++;
	}

	for (int n = indexQ2Start; n <= indexQ2End; n++)
	{
		results[resIndex] = q2pHullPoints[n];
		resIndex++;
	}

	for (int n = indexQ3Start; n <= indexQ3End; n++)
	{
		results[resIndex] = q3pHullPoints[n];
		resIndex++;
	}

	for (int n = indexQ4Start; n <= indexQ4End; n++)
	{
		results[resIndex] = q4pHullPoints[n];
		resIndex++;
	}

	if (countOfFinalHullPoint > 1 && _shouldCloseTheGraph)
	{
		results[resIndex] = results[0];
	}

	hullPointCount = countOfFinalHullPoint;

	return results;
}

// **************************************************************************
