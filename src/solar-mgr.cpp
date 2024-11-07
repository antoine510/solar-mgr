#include <iostream>
#include <thread>
#include "Bus.hpp"
#include "Network.hpp"

static constexpr const char* serial_device = "/dev/rs485-bus";

static constexpr const char* influxdb_org_name = "Microtonome";
static constexpr const char* influxdb_bucket_solar = "Solaire";
static constexpr const char* influxdb_bucket_battery = "Batterie";

using LogPeriod = std::chrono::duration<int64_t, std::ratio<60>>;

int main(int argc, char** argv) {
	using namespace std::chrono;

	try {
		auto influxdb_token = getenv("INFLUXDB_TOKEN");
		if(!influxdb_token) throw std::invalid_argument("Missing INFLUXDB_TOKEN environment variable");

		Bus::SetupBus(serial_device, 9600);
		influxdb_cpp::server_info serverInfoSolar("127.0.0.1", 8086, influxdb_org_name, influxdb_token, influxdb_bucket_solar);
		influxdb_cpp::server_info serverInfoBattery("127.0.0.1", 8086, influxdb_org_name, influxdb_token, influxdb_bucket_battery);

		std::vector<MPPT> mppts{0x01};
		CurrentSensor producers(0x65, 8), consumers(0x66, -15);

		while(true) {
			const auto currentTP = std::chrono::system_clock::now();
			const auto nextTP = std::chrono::ceil<LogPeriod>(currentTP);
			std::this_thread::sleep_until(nextTP);

			sendMPTTs(serverInfoSolar, mppts);
			sendCurrents(serverInfoBattery, producers, consumers);
		}
	} catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}
