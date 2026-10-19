#include <iostream>
#include <fstream>
#include <thread>
#include <nlohmann/json.hpp>
#include "Bus.hpp"
#include "Network.hpp"

using LogPeriod = std::chrono::duration<int64_t, std::ratio<60>>;

int main(int argc, char** argv) {
	try {
		const auto configJson = nlohmann::json::parse(std::ifstream("config.json"));
		const auto influxJson = configJson.at("InfluxDB");
		const auto busJson = configJson.at("Bus");

		const std::string influxAddress{influxJson.value("Address", "localhost")};
		const unsigned influxPort{influxJson.value("Port", 8086u)};
		const std::string influxOrg{influxJson.at("Org")};
		const std::string influxToken{influxJson.at("Token")};

		Bus::SetupBus(busJson.at("Device"), busJson.value("Baudrate", 9600u));
		influxdb_cpp::server_info serverInfoSolar(influxAddress, influxPort, influxOrg, influxToken, influxJson.at("SolarBucket"));
		influxdb_cpp::server_info serverInfoBattery(influxAddress, influxPort, influxOrg, influxToken, influxJson.at("BatteryBucket"));

		std::vector<MPPT> mppts;
		const auto mpptsJson = configJson.at("MPPTs");
		const int mpptCount = mpptsJson.size();
		for(const auto& mpptJson : mpptsJson) {
			auto resWeights = mpptJson.value("Connections", std::vector<int>{});
			if(resWeights.size() != mpptCount) throw std::runtime_error("Invalid weights for MPPT");
			mppts.emplace_back(mpptJson.at("Address"), mpptJson.value("VoutTune", 0), resWeights);
		}

		const auto batJson = configJson.at("ExternalBattery");
		ExternalBattery externalBattery(batJson.at("Address"));

		const auto currentJson = configJson.at("CurrentSensors");
		const auto producersJson = currentJson.at("Producers");
		const auto consumersJson = currentJson.at("Consumers");
		CurrentSensor producers(producersJson.at("Address"), producersJson.value("Scale", 1), producersJson.value("Offset", 0));
		CurrentSensor consumers(consumersJson.at("Address"), consumersJson.value("Scale", 1), consumersJson.value("Offset", 0));

		while(true) {
			const auto currentTP = std::chrono::system_clock::now();
			const auto nextTP = std::chrono::ceil<LogPeriod>(currentTP);
			std::this_thread::sleep_until(nextTP);

			std::vector<int> currents_ca;
			for(MPPT& mppt : mppts) {
				mppt.Update();
				currents_ca.push_back(mppt.HasValidData() ? mppt.GetData().iout_ca : 0);
			}
			for(MPPT& mppt : mppts) mppt.DoCurrentCompensation(currents_ca);

			sendMPTTs(serverInfoSolar, mppts);
			sendExternalBattery(serverInfoBattery, externalBattery);
			sendCurrents(serverInfoBattery, producers, consumers);
		}
	} catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}
