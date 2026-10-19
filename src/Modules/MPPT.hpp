#pragma once

#include <vector>
#include <algorithm>
#include "BusModule.hpp"

class MPPT : public BusModule {
public:
	MPPT(uint8_t moduleID, uint16_t voutTune, std::vector<int> resistanceWeights) : BusModule(moduleID), _voutTune(voutTune), _resistanceWeights(std::move(resistanceWeights)) {}

	struct SerialData {
		uint16_t vin_cv;
		uint16_t vout_dv;
		uint16_t iout_ca;
		uint16_t pout_dw;
	};

	bool HasValidData() const { return _valid; }
	const SerialData& GetData() const { return _cacheData; }
	void Update() {
		try {
			_cacheData = sendMessageWithResponse<SerialData>((uint8_t)READ_ALL, 1);
			_valid = true;
		} catch(const BusModule::NoResponseException&) {
			_valid = false;
		} catch(const std::exception& e) {
			std::cerr << "MPPT" << (int)GetModuleID() << " " << e.what() << std::endl;
			_valid = false;
		}
	}
	void DoCurrentCompensation(const std::vector<int>& currents_ca) const {
		if(!_voutTune || !_valid) return;

		int bias = 0;
		for(int i = 0; i < currents_ca.size(); ++i) bias += currents_ca[i] * _resistanceWeights[i];
		auto finalTune = _voutTune - std::clamp(bias / 100, 0, 200);

		try {
			SetVoutTune(finalTune);
		} catch(const std::exception&) {}
	}

	void SetManualPP(uint16_t mpp_dv) const { sendMessage((uint8_t)SET_MPP_MANUAL_DV, mpp_dv); }
	void SetAutomaticPP() const { sendMessage((uint8_t)SET_MPP_AUTO); }
	void EnableOutput() const { sendMessage((uint8_t)ENABLE_OUTPUT); }
	void DisableOutput() const { sendMessage((uint8_t)DISABLE_OUTPUT); }

	// [0 - 4095], tuned for 80 V output at 2048
	void SetVoutTune(uint16_t tune) const { sendMessage((uint8_t)SET_VOUT_TUNE, tune); }

private:
	enum Commands : uint8_t {
		READ_ALL = 1,
		SET_MPP_MANUAL_DV,
		SET_MPP_AUTO,
		ENABLE_OUTPUT,
		DISABLE_OUTPUT,
		SET_VOUT_TUNE
	};

	uint16_t _voutTune;
	std::vector<int> _resistanceWeights;
	SerialData _cacheData;
	bool _valid = false;
};
