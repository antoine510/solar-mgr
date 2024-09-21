#pragma once

#include "BusModule.hpp"

class MPPT : public BusModule {
public:
	MPPT(uint8_t moduleID) : BusModule(moduleID) {}

	struct SerialData {
		uint16_t vin_cv;
		uint16_t vout_dv;
		uint16_t iout_ca;
		uint16_t eout_j;
	};

	SerialData GetData() const { return sendMessageWithResponse<SerialData>((uint8_t)READ_ALL); }
	void SetManualPP(uint16_t mpp_dv) const { sendMessage((uint8_t)SET_MPP_MANUAL_DV, mpp_dv); }
	void SetAutomaticPP() const { sendMessage((uint8_t)SET_MPP_AUTO); }
	void EnableOutput() const { sendMessage((uint8_t)ENABLE_OUTPUT); }
	void DisableOutput() const { sendMessage((uint8_t)DISABLE_OUTPUT); }

private:
	enum Commands : uint8_t {
		READ_ALL = 1,
		SET_MPP_MANUAL_DV = 1,
		SET_MPP_AUTO = 1,
		ENABLE_OUTPUT = 1,
		DISABLE_OUTPUT = 1,
	};
};
