#include "pcm.hpp"

#ifndef HAS_PCM
std::unique_ptr<PerfData> createPCMPerfData() { return nullptr; }
#else

#include <cpucounters.h>

struct PCMPerfData : PerfData {
	pcm::SystemCounterState beforeState;
	pcm::SystemCounterState afterState;
	
	void begin() override {
		beforeState = pcm::getSystemCounterState();
		PerfData::begin();
	}
	
	void end() override {
		PerfData::end();
		afterState = pcm::getSystemCounterState();
	}
	
	void printStatistics() override {
		PerfData::printStatistics();
		
		std::cerr << "pcm measurements:\n";
		
		std::vector<std::pair<std::string_view, std::string>> measurements;
		size_t maxLabelLen = 0;
		auto addMeasurment = [&] <typename T> (std::string_view label, T(*fn)(const pcm::SystemCounterState&, const pcm::SystemCounterState&)) {
			maxLabelLen = std::max(maxLabelLen, label.size());
			std::string value = std::to_string(fn(beforeState, afterState));
			if constexpr (std::is_integral_v<T>) {
				int vlen = value.size();
				for (int i = 1; i * 3 < vlen; i++) {
					value.insert(value.begin() + (vlen - i * 3), ',');
				}
			}
			measurements.emplace_back(label, std::move(value));
		};
		
		addMeasurment("instr retired", &pcm::getInstructionsRetired<pcm::SystemCounterState>);
		addMeasurment("instr per clock", &pcm::getIPC<pcm::SystemCounterState>);
		addMeasurment("instr per clock (core)", &pcm::getCoreIPC<pcm::SystemCounterState>);
		addMeasurment("MC bytes read", &pcm::getBytesReadFromMC<pcm::SystemCounterState>);
		addMeasurment("MC bytes written", &pcm::getBytesWrittenToMC<pcm::SystemCounterState>);
		addMeasurment("L2 hits", &pcm::getL2CacheHits<pcm::SystemCounterState>);
		addMeasurment("L2 misses", &pcm::getL2CacheMisses<pcm::SystemCounterState>);
		addMeasurment("L2 hit ratio", &pcm::getL2CacheHitRatio<pcm::SystemCounterState>);
		addMeasurment("L3 hits", &pcm::getL3CacheHits<pcm::SystemCounterState>);
		addMeasurment("L3 misses", &pcm::getL3CacheMisses<pcm::SystemCounterState>);
		addMeasurment("L3 hit ratio", &pcm::getL3CacheHitRatio<pcm::SystemCounterState>);
		addMeasurment("pl bad speculation", &pcm::getBadSpeculation<pcm::SystemCounterState>);
		addMeasurment("pl backend bound", &pcm::getBackendBound<pcm::SystemCounterState>);
		addMeasurment("pl frontend bound", &pcm::getFrontendBound<pcm::SystemCounterState>);
		
		for (const auto& [label, value] : measurements) {
			std::cerr << std::string(maxLabelLen + 2 - label.size(), ' ') << label << ": " << value << "\n";
		}
	}
};

std::unique_ptr<PerfData> createPCMPerfData() {
	auto m = pcm::PCM::getInstance();
	if (m->program() != pcm::PCM::Success)
		return nullptr;
	return std::make_unique<PCMPerfData>();
}

#endif
