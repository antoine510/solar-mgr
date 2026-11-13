#pragma once

#include "BusModule.hpp"

class CurrentSensor : public BusModule {
public:
	struct SerialData {
		int16_t current_cA;
		int16_t temp_dC;
	};

	CurrentSensor(uint8_t moduleID, bool reversed) : BusModule(moduleID), _reversed(reversed) {}

	SerialData GetData() const {
		SerialData data = sendMessageWithResponse<SerialData>((uint8_t)READ_ALL);
		if(_reversed) data.current_cA = -data.current_cA;
		return data;
	}

private:
	enum Commands : uint8_t {
		READ_ALL = 1
	};
	bool _reversed;
};
