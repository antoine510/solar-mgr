#include "Network.hpp"
#include <iostream>

void sendMPTTs(const influxdb_cpp::server_info& si, const std::vector<MPPT>& mppts) {
	for(const MPPT& mppt : mppts) {
		try {
			auto data = mppt.GetData();
			influxdb_cpp::builder builder;
			builder.meas("MPPT" + std::to_string(mppt.GetModuleID()))
			.field("vin", data.vin_cv / 100.f, 2)
			.field("vout", data.vout_dv / 10.f, 1)
			.field("iout", data.iout_ca / 100.f, 2)
			.field("power", data.eout_j / 60.f, 1)
			.post_http(si);
        } catch(const BusModule::NoResponseException&) {
        } catch(const std::exception& e) {
			std::cerr << "MPPT" << (int)mppt.GetModuleID() << " " << e.what() << std::endl;
		}
	}
}

void sendCurrents(const influxdb_cpp::server_info& si, const CurrentSensor& producers, const CurrentSensor& consumers) {
	influxdb_cpp::builder builder;
	int prodCurrent, consCurrent;
	bool hasProd = false, hasCons = false;
	try {
		prodCurrent = producers.GetCurrent();
		hasProd = true;
	} catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	try {
		consCurrent = consumers.GetCurrent();
		hasCons = true;
	} catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	if(hasProd) {
		if(hasCons) {
			builder.meas("Currents")
				.field("producers", prodCurrent / 1000.f, 3)
				.field("consumers", consCurrent / 1000.f, 3)
				.post_http(si);
		} else {
			builder.meas("Currents")
				.field("producers", prodCurrent / 1000.f, 3)
				.post_http(si);
		}
	} else {
		if(hasCons) {
			builder.meas("Currents")
				.field("consumers", consCurrent / 1000.f, 3)
				.post_http(si);
		}
	}
}
