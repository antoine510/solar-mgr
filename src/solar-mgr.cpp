#include <iostream>
#include <fstream>
#include <thread>
#include <nlohmann/json.hpp>
#include "Bus.hpp"
#include "Network.hpp"

//static constexpr const char* serial_device = "/dev/rs485-bus";

//static constexpr const char* influxdb_org_name = "Microtonome";
//static constexpr const char* influxdb_bucket_solar = "Solaire";
//static constexpr const char* influxdb_bucket_battery = "Batterie";

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

		//auto influxdb_token = getenv("INFLUXDB_TOKEN");
		//if(!influxdb_token) throw std::invalid_argument("Missing INFLUXDB_TOKEN environment variable");

		Bus::SetupBus(busJson.at("Device"), busJson.value("Baudrate", 9600u));
		influxdb_cpp::server_info serverInfoSolar(influxAddress, influxPort, influxOrg, influxToken, influxJson.at("SolarBucket"));
		influxdb_cpp::server_info serverInfoBattery(influxAddress, influxPort, influxOrg, influxToken, influxJson.at("BatteryBucket"));

		std::vector<MPPT> mppts;
		const auto mpptsJson = configJson.at("MPPTs");
		for(const auto& mpptJson : mpptsJson) {
			mppts.emplace_back(mpptJson.at("Address"), mpptJson.value("VoutTune", 0));
		}

		const auto currentJson = configJson.at("CurrentSensors");
		const auto producersJson = currentJson.at("Producers");
		const auto consumersJson = currentJson.at("Consumers");
		CurrentSensor producers(producersJson.at("Address"), producersJson.value("Scale", 1), producersJson.value("Offset", 0));
		CurrentSensor consumers(consumersJson.at("Address"), consumersJson.value("Scale", 1), consumersJson.value("Offset", 0));

		//CurrentSensor producers(0x65, -3.0534351145f, -365, true), consumers(0x66, -3.1f, -117, true);

		while(true) {
			const auto currentTP = std::chrono::system_clock::now();
			const auto nextTP = std::chrono::ceil<LogPeriod>(currentTP);
			std::this_thread::sleep_until(nextTP);

			for(MPPT& mppt : mppts) mppt.Update();
//			try {
//				mppts[1].SetVoutTune(1107u);
//			} catch(const std::exception&) {}

			sendMPTTs(serverInfoSolar, mppts);
			sendCurrents(serverInfoBattery, producers, consumers);
		}
	} catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}
