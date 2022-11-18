#include <iostream>
#include <thread>
#include "SolarSerial.hpp"
#include <influxdb.hpp>

static constexpr const char* serial_device = "/dev/mppts";

static constexpr const char* influxdb_org_name = "Microtonome";
static constexpr const char* influxdb_bucket = "Solaire";

using LogPeriod = std::chrono::duration<int64_t, std::ratio<30>>;

int main(int argc, char** argv) {
	using namespace std::chrono;

	try {
		auto influxdb_token = getenv("INFLUXDB_TOKEN");
		if(!influxdb_token) throw std::invalid_argument("Missing INFLUXDB_TOKEN environment variable");

		SolarSerial solar(serial_device);
		influxdb_cpp::server_info serverInfo("127.0.0.1", 8086, influxdb_org_name, influxdb_token, influxdb_bucket);

		while(true) {
			const auto currentTP = std::chrono::system_clock::now();
			const auto nextTP = std::chrono::ceil<LogPeriod>(currentTP);
			std::this_thread::sleep_until(nextTP);

			try {
				auto data = solar.ReadAll();

				influxdb_cpp::builder()
					.meas("MPPT1")
					.field("voltage", data[0].millivolts / 1000.f, 2)
					.field("current", data[0].milliamps / 1000.f, 2)
					.field("watts", data[0].deciwatts / 10.f, 1)
					.field("joules", data[0].joules)
					.meas("MPPT2")
					.field("voltage", data[1].millivolts / 1000.f, 2)
					.field("current", data[1].milliamps / 1000.f, 2)
					.field("watts", data[1].deciwatts / 10.f, 1)
					.field("joules", data[1].joules)
					.meas("MPPT3")
					.field("voltage", data[2].millivolts / 1000.f, 2)
					.field("current", data[2].milliamps / 1000.f, 2)
					.field("watts", data[2].deciwatts / 10.f, 1)
					.field("joules", data[2].joules)
					.meas("MPPT4")
					.field("voltage", data[3].millivolts / 1000.f, 2)
					.field("current", data[3].milliamps / 1000.f, 2)
					.field("watts", data[3].deciwatts / 10.f, 1)
					.field("joules", data[3].joules)
					.post_http(serverInfo);
			} catch(const std::exception& e) {
				std::cerr << e.what() << std::endl;
			}
		}
	} catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}
