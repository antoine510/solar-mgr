#pragma once

#include "BusModule.hpp"

class CurrentSensor : public BusModule {
public:
	CurrentSensor(uint8_t moduleID, float calFac, int calOffset, bool crcCheck) : BusModule(moduleID, crcCheck), _calFac(calFac), _calOffset(calOffset) {}

	int GetCurrent() const {
		int current = sendMessageWithResponse<int32_t>((uint8_t)READ_CURRENT_MA) * _calFac + _calOffset;
		if(current < -100'000 || current > 100'000) throw std::runtime_error("Invalid current value");
		return current;
	}

private:
	enum Commands : uint8_t {
		READ_CURRENT_MA = 1,
	};

	float _calFac;
	int _calOffset;
};
