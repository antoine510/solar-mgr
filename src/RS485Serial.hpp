#pragma once

#include <string>
#include <vector>
#include <array>
#include <stdexcept>
#include "MPPTData.h"

#include <netinet/in.h>

#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))


class RS485Serial {
public:
	static constexpr int MPPTCount = 4;
	static constexpr uint8_t MPPTStartAddress = 0x01;
	static constexpr uint8_t currentProducersAddress = 0x65;
	static constexpr uint8_t currentConsumersAddress = 0x66;

	RS485Serial(const std::string& path, int baudrate);
	~RS485Serial() noexcept;

	int ReadCurrentProducers_mA();
	int ReadCurrentConsumers_mA();

	std::array<MPPTData, MPPTCount> ReadAllMPPTs();
	void SetMaxWiper(int mpptID, uint8_t max);
	void SetOutputEnabled(int mpptID, bool en);
private:
	class ReadTimeoutException : public std::runtime_error {
	public:
		ReadTimeoutException() : std::runtime_error("Read timed-out") {}
	};

	std::vector<uint8_t> read() const;
	void write(const uint8_t* cmd_buf, size_t sz) const;

	enum class MPPTCommandID : uint8_t {
		READ_ALL = 0x01,
		SET_MAX_WIPER = 0x02,	// IN Max wiper as a uint8_t
		SET_OUTPUT_ENABLED = 5,
                SET_OUTPUT_DISABLED = 6
	};
	enum class CurrentCommandID : uint8_t {
		MAGIC = 0,
		READ_CURRENT = 1	// OUT Current in mA as a int32_t
	};


	PACK(
		struct CommandHeader {
			uint8_t magic1 = 0x4f;
			uint8_t magic2 = 0xc7;
			uint8_t identity;
			uint8_t command;
		};
	)

	int _fd = -1;
};
