#include "slice_parallel.hpp"

#include <barrier>
#include <thread>
#include <span>
#include <list>
#include <algorithm>
#include <cmath>

std::pair<size_t, size_t> partitionRange(size_t n, size_t numThreads, size_t threadIndex) {
	size_t perThread = n / numThreads;
	size_t numWithExtra = n % numThreads;
	size_t first =
		(perThread + 1) * std::min(threadIndex, numWithExtra) +
		((threadIndex > numWithExtra) ? perThread * (threadIndex - numWithExtra) : 0);
	size_t last = first + perThread + (threadIndex < numWithExtra);
	return { std::min(first, n), std::min(last, n) };
}

template <typename T>
struct SliceParallelSolver {
	std::span<point<T>> points;
	HullSolveFunction<T> innerSolve;
	SolveSliceParallelArgs args;
	std::barrier<> barrier;
	
	explicit SliceParallelSolver(const SolveSliceParallelArgs& _args)
		: args(_args), barrier(_args.numThreads) { }
	
	virtual ~SliceParallelSolver() { }
	
	virtual void threadTarget(size_t threadIndex) = 0;
	virtual std::vector<point<T>> finish() = 0;
};

template <typename T>
struct SliceParallelSolver_SplitByDirection : SliceParallelSolver<T> {
	std::vector<size_t> maxPointIndices;
	std::vector<std::vector<point<T>>> pointsInSlices;
	
	explicit SliceParallelSolver_SplitByDirection(const SolveSliceParallelArgs& _args)
		: SliceParallelSolver<T>(_args), maxPointIndices(_args.numThreads), pointsInSlices(_args.numThreads) { }
	
	void threadTarget(size_t threadIndex) {
		double angle = M_PI * 2 * (double)threadIndex / (double)this->args.numThreads - M_PI;
		pointd direction(std::cos(angle), std::sin(angle));
		
		std::tuple<double, point<T>, size_t> maxValue(-INFINITY, point<T>(), 0);
		for (size_t i = 0; i < this->points.size(); i++) {
			double dot = direction.dot(this->points[i]);
			maxValue = std::max(maxValue, std::make_tuple(dot, this->points[i], i));
		}
		
		maxPointIndices[threadIndex] = std::get<2>(maxValue);
		
		this->barrier.arrive_and_wait();
		
		size_t maxPointIndexR = maxPointIndices[threadIndex];
		size_t maxPointIndexL = maxPointIndices[(threadIndex + 1) % this->args.numThreads];
		if (maxPointIndexL == maxPointIndexR)
			return;
		
		point<T> maxPointR = this->points[maxPointIndexR];
		point<T> maxPointL = this->points[maxPointIndexL];
		
		std::vector<point<T>> pointsInSlice;
		for (const point<T> point : this->points) {
			if (point.sideOfLine(maxPointR, maxPointL) == side::right) {
				pointsInSlice.push_back(point);
			}
		}
		pointsInSlice.push_back(maxPointL);
		pointsInSlice.push_back(maxPointR);
		
		this->innerSolve(pointsInSlice);
		
		std::rotate(pointsInSlice.begin(), std::find(pointsInSlice.begin(), pointsInSlice.end(), maxPointR), pointsInSlice.end());
		pointsInSlice.pop_back();
		
		pointsInSlices[threadIndex] = std::move(pointsInSlice);
	}
	
	std::vector<point<T>> finish() {
		std::vector<point<T>> result;
		for (size_t i = 0; i < this->args.numThreads; i++) {
			result.insert(result.end(), pointsInSlices[i].begin(), pointsInSlices[i].end());
		}
		return result;
	}
};

template <typename T>
void solveSliceParallel(std::vector<point<T>>& points, const HullSolveFunction<T>& innerSolve, SolveSliceParallelArgs args) {
	if (args.numThreads == 0) {
		args.numThreads = std::thread::hardware_concurrency();
	}
	if (args.numThreads == 1) {
		args.numThreads = 2;
	}
	
	std::unique_ptr<SliceParallelSolver<T>> solver;
	
	switch (args.splitMethod) {
	case SplitMethod::DirectionsExtremePoint:
		solver = std::make_unique<SliceParallelSolver_SplitByDirection<T>>(args);
		break;
	default:
		std::abort();
	}
	
	solver->points = points;
	solver->innerSolve = innerSolve;
	
	std::list<std::thread> threads;
	for (size_t ti = 0; ti < args.numThreads; ti++) {
		threads.emplace_back([ti, _solver=solver.get()] { _solver->threadTarget(ti); });
	}
	
	for (std::thread& thread : threads) {
		thread.join();
	}
	
	std::vector<point<T>> result = solver->finish();
	points.swap(result);
}

template void solveSliceParallel<double>(std::vector<point<double>>& points, const HullSolveFunction<double>& innerSolve, SolveSliceParallelArgs args);
template void solveSliceParallel<int64_t>(std::vector<point<int64_t>>& points, const HullSolveFunction<int64_t>& innerSolve, SolveSliceParallelArgs args);

SplitMethod splitMethodFromString(std::string_view name) {
	if (name == "dirExtremePoint")
		return SplitMethod::DirectionsExtremePoint;
	std::abort();
}
