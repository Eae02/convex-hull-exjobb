#pragma once

#include "perf_data.hpp"

#include <memory>

std::unique_ptr<PerfData> createPCMPerfData();
