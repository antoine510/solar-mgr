#pragma once

#include "BusModule.hpp"

class MPPT : public BusModule {
public:
	MPPT(uint8_t moduleID, uint16_t voutTune) : BusModule(moduleID), _voutTune(voutTune) {}

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
			if(_voutTune && !_voutTuneApplied) {
				try {
					SetVoutTune(_voutTune);
					_voutTuneApplied = true;
				} catch(const std::exception&) {}
			}
		} catch(const BusModule::NoResponseException&) {
			_valid = false;
			_voutTuneApplied = false;
		} catch(const std::exception& e) {
			std::cerr << "MPPT" << (int)GetModuleID() << " " << e.what() << std::endl;
			_valid = false;
			_voutTuneApplied = false;
		}
	}
	void SetManualPP(uint16_t mpp_dv) const { sendMessage((uint8_t)SET_MPP_MANUAL_DV, mpp_dv); }
	void SetAutomaticPP() const { sendMessage((uint8_t)SET_MPP_AUTO); }
	void EnableOutput() const { sendMessage((uint8_t)ENABLE_OUTPUT); }
	void DisableOutput() const { sendMessage((uint8_t)DISABLE_OUTPUT); }
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
	bool _voutTuneApplied = false;
	SerialData _cacheData;
	bool _valid = false;
};
