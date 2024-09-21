#pragma once

#include "BusModule.hpp"

class CurrentSensor : public BusModule {
public:
	CurrentSensor(uint8_t moduleID, int cal = 0) : BusModule(moduleID), _calibration(cal) {}

	int GetCurrent() const {
		try {
			return _calibration - sendMessageWithResponse<int32_t>((uint8_t)READ_CURRENT_MA);
		} catch (const std::runtime_error& e) {
			return 0;
		}
	}

private:
	enum Commands : uint8_t {
		READ_CURRENT_MA = 1,
	};

	int _calibration;
};
