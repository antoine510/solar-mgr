#pragma once

#include "BusModule.hpp"

class ExternalBattery : public BusModule {
public:
	ExternalBattery(uint8_t moduleID) : BusModule(moduleID) {}

	struct [[gnu::packed]] BatteryData {
		int16_t pack_voltage_mv[6];
		int16_t temp_dC;
		int16_t current_cA;
		uint8_t heater_on;
	};

	BatteryData GetData() const {
		auto data = sendMessageWithResponse<BatteryData>((uint8_t)READ_ALL);
		data.current_cA += 8;	// Hacky calibration
		return data;
	}

private:
	enum Commands : uint8_t {
		READ_ALL = 1,
	};
};
