#pragma once

#include "BusModule.hpp"

class ExternalBattery : public BusModule {
public:
	ExternalBattery(uint8_t moduleID) : BusModule(moduleID) {}

	struct BatteryData {
		int16_t pack_voltage_mv[6];
		int16_t temp_dC;
		int16_t current_count;
		uint8_t heater_on;
	};

	BatteryData GetData() const {
		return sendMessageWithResponse<BatteryData>((uint8_t)READ_ALL);
	}

private:
	enum Commands : uint8_t {
		READ_ALL = 1,
	};
};
