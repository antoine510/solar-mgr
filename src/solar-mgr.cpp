#include <iostream>
#include <thread>
#include <array>
#include "Bus.hpp"
#include "CurrentSensor.hpp"
#include "MPPT.hpp"
#include <influxdb.hpp>

static constexpr const char* serial_device = "/dev/mppts";

static constexpr const char* influxdb_org_name = "Microtonome";
static constexpr const char* influxdb_bucket_solar = "Solaire";
static constexpr const char* influxdb_bucket_battery = "Batterie";

using LogPeriod = std::chrono::duration<int64_t, std::ratio<60>>;

int main(int argc, char** argv) {
	using namespace std::chrono;

	try {
		auto influxdb_token = getenv("INFLUXDB_TOKEN");
		if(!influxdb_token) throw std::invalid_argument("Missing INFLUXDB_TOKEN environment variable");

		Bus::SetupBus(serial_device, 9500); // Temporary temperature hack atmega internal oscillator callibration needed
		influxdb_cpp::server_info serverInfoSolar("127.0.0.1", 8086, influxdb_org_name, influxdb_token, influxdb_bucket_solar);
		influxdb_cpp::server_info serverInfoBattery("127.0.0.1", 8086, influxdb_org_name, influxdb_token, influxdb_bucket_battery);

		std::array<MPPT, 4> mppts{0x01, 0x02, 0x03, 0x04};
		CurrentSensor producers(0x65, 8), consumers(0x66, -15);

		while(true) {
			const auto currentTP = std::chrono::system_clock::now();
			const auto nextTP = std::chrono::ceil<LogPeriod>(currentTP);
			std::this_thread::sleep_until(nextTP);

			influxdb_cpp::builder builder;
			bool hasData = false;

			for(MPPT& mppt : mppts) {
				try {
					auto data = mppt.GetData();
					builder.meas("MPPT" + std::to_string(mppt.GetModuleID()))
					.field("vin", data.vin_cv / 100.f, 2)
					.field("vout", data.vout_dv / 10.f, 1)
					.field("iout", data.iout_ca / 100.f, 2)
					.field("power", data.eout_j / 60.f, 1);
					hasData = true;
				} catch(const std::exception& e) {}
			}
			if(hasData) reinterpret_cast<influxdb_cpp::detail::ts_caller&>(builder).post_http(serverInfoSolar);

			try {
				influxdb_cpp::builder()
					.meas("Currents")
					.field("producers", producers.GetCurrent() / 1000.f, 3)
					.field("consumers", consumers.GetCurrent() / 1000.f, 3)
					.post_http(serverInfoBattery);

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
