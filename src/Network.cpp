#include "Network.hpp"
#include <iostream>

void sendMPTTs(const influxdb_cpp::server_info& si, const std::vector<MPPT>& mppts) {
	for(const MPPT& mppt : mppts) {
		if(!mppt.HasValidData()) continue;
		auto data = mppt.GetData();
		influxdb_cpp::builder builder;
		builder.meas("MPPT" + std::to_string(mppt.GetModuleID()))
		.field("vin", data.vin_cv / 100.f, 2)
		.field("vout", data.vout_dv / 10.f, 1)
		.field("iout", data.iout_ca / 100.f, 2)
		.field("power", data.pout_dw / 10.f, 1)
		.post_http(si);
	}
}

int externalBatteryCurrent_cA = 0;
void sendExternalBattery(const influxdb_cpp::server_info& si, const ExternalBattery& bat) {
	try {
		auto data = bat.GetData();
		externalBatteryCurrent_cA = data.current_cA;
		influxdb_cpp::builder builder;
		builder.meas("External")
		.field("v1", data.pack_voltage_mv[0] / 1000.f, 3)
		.field("v2", data.pack_voltage_mv[1] / 1000.f, 3)
		.field("v3", data.pack_voltage_mv[2] / 1000.f, 3)
		.field("v4", data.pack_voltage_mv[3] / 1000.f, 3)
		.field("v5", data.pack_voltage_mv[4] / 1000.f, 3)
		.field("v6", data.pack_voltage_mv[5] / 1000.f, 3)
		.field("temp", data.temp_dC / 10.f, 1)
		.field("i", data.current_cA / 100.f, 2)
		.field("heat", (bool)data.heater_on)
		.post_http(si);
	} catch(const std::exception& e) {
		externalBatteryCurrent_cA = 0;
		std::cerr << e.what() << std::endl;
	}
}

void sendCurrents(const influxdb_cpp::server_info& si, const CurrentSensor& producers, const CurrentSensor& consumers) {
	influxdb_cpp::builder builder;
	int prodCurrent_mA, consCurrent_mA;
	bool hasProd = false, hasCons = false;
	try {
		prodCurrent_mA = producers.GetCurrent();
		hasProd = true;
	} catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	try {
		consCurrent_mA = consumers.GetCurrent() - externalBatteryCurrent_cA * 10;
		hasCons = true;
	} catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	if(hasProd) {
		if(hasCons) {
			builder.meas("Currents")
				.field("producers", prodCurrent_mA / 1000.f, 3)
				.field("consumers", consCurrent_mA / 1000.f, 3)
				.post_http(si);
		} else {
			builder.meas("Currents")
				.field("producers", prodCurrent_mA / 1000.f, 3)
				.post_http(si);
		}
	} else {
		if(hasCons) {
			builder.meas("Currents")
				.field("consumers", consCurrent_mA / 1000.f, 3)
				.post_http(si);
		}
	}
}
