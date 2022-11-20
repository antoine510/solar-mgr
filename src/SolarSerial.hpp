#pragma once

#include <string>
#include <vector>
#include <array>
#include <stdexcept>
#include "MPPTData.h"

#ifndef WINDOWS
#include <netinet/in.h>
#endif

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif


class SolarSerial {
public:
	static constexpr const int MPPTCount = 4;

	SolarSerial(const std::string& path, int baudrate = 115200);
	~SolarSerial() noexcept;

	std::array<MPPTData, MPPTCount> ReadAll();
	void SetMaxWiper(int mpptID, uint8_t max);
	void SetOutputEnabled(int mpptID, bool en);
private:
	class ReadTimeoutException : public std::runtime_error {
	public:
		ReadTimeoutException() : std::runtime_error("Read timed-out") {}
	};

	std::vector<uint8_t> read() const;
	void write(const uint8_t* cmd_buf, size_t sz) const;

	static constexpr const uint8_t cmd_magic[] = {0x4f, 0xc7, 0xb2, 0x9a};
	enum CommandID : uint8_t {
		READ_ALL = 0x01,
		SET_MAX_WIPER = 0x02,		// Max wiper is next byte
		SET_OUTPUT_DISABLED = 0x04,
		SET_OUTPUT_ENABLED = 0x05
	};

	PACK(
		struct CommandHeader {
			uint8_t magic1 = 0x4f;
			uint8_t magic2 = 0xc7;
			uint8_t magic3 = 0xb2;
			uint8_t magic4 = 0x9a;
			uint8_t identity;
			CommandID command;
		};
	)

	int _fd = -1;
};
