#include "hull_impl.hpp"
#include "point.hpp"

#include <vector>

enum class SplitMethod {
	DirectionsExtremePoint,
	AngleFromMean,
	AngleFromBoxCenter
};

SplitMethod splitMethodFromString(std::string_view name);

struct SolveSliceParallelArgs {
	size_t numThreads = 0;
	SplitMethod splitMethod = SplitMethod::DirectionsExtremePoint;
};

template <typename T>
void solveSliceParallel(std::vector<point<T>>& points, const HullSolveFunction<T>& innerSolve, SolveSliceParallelArgs args);
