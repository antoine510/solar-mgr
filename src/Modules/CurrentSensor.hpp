#pragma once

#include "BusModule.hpp"

class CurrentSensor : public BusModule {
public:
	CurrentSensor(uint8_t moduleID, int cal = 0) : BusModule(moduleID), _calibration(cal) {}

	int GetCurrent() const {
		int current = _calibration - sendMessageWithResponse<int32_t>((uint8_t)READ_CURRENT_MA);
		if(current > 100'000) throw std::runtime_error("Invalid current value");
		return current;
	}

private:
	enum Commands : uint8_t {
		READ_CURRENT_MA = 1,
	};

	int _calibration;
};
