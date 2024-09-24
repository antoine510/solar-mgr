#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <mutex>
#include <memory>

class Bus {
public:
	~Bus() noexcept;

	static void SetupBus(const std::string& path, int baudrate = 500'000) {
		inst = std::unique_ptr<Bus>(new Bus(path, baudrate));
	}
	static Bus& Instance() {
		return *inst.get();
	}
	void SetBaudrate(int baudrate);

	class ReadTimeoutException : public std::runtime_error {
	public:
		ReadTimeoutException() : std::runtime_error("Read timed-out") {}
	};

	class ReadSizeException : public std::runtime_error {
	public:
		ReadSizeException() : std::runtime_error("Wrong read size") {}
	};

	class WriteException : public std::runtime_error {
	public:
		WriteException() : std::runtime_error("Write failure") {}
	};

	auto SendCommand(const std::vector<uint8_t>& command, size_t responseSize) {
		std::scoped_lock lk(_serialMutex);
		Write(command);
		return Read(responseSize);
	}
	void SendCommand(const std::vector<uint8_t>& command) {
		std::scoped_lock lk(_serialMutex);
		Write(command);
	}

	/**
	 * Checks the CRC of the response against the value of its last byte
	 */
	static bool CheckCRC(const std::vector<uint8_t>& response) {
		uint8_t crc = 0;
		std::size_t datalen = response.size() - 1;
		for (uint8_t b = 0; b < datalen; ++b) {
			crc ^= response[b];
			for (uint8_t i = 0; i < 8; ++i) {
				if (crc & 0x80)
					crc = (crc << 1) ^ 0x07;	// SMBUS CRC8
				else
					crc = crc << 1;
			}
		}
		return crc == response[datalen];
	}

	static constexpr uint8_t protocol1 = 0x4f, protocol2 = 0xc7;

	static int GetRetryBaudrate(int index) { return _baudrates[index % (sizeof(_baudrates) / sizeof(int))]; }

private:
	Bus(const std::string& path, int baudrate = 9600);

	void waitForData() const;

	std::vector<uint8_t> Read(size_t expectedSize) const;

	void Write(const uint8_t* buf, size_t sz) const;
	void Write(const std::vector<uint8_t>& bytes) const {
		Write(bytes.data(), bytes.size());
	}
	void Write(const std::string& str) const {
		Write(reinterpret_cast<const uint8_t*>(str.data()), str.size());
	}

	static inline std::unique_ptr<Bus> inst;

	int _currentBaudrate;
	static constexpr int _baudrates[] = {9600, 9600, 9500, 9700};

	int _fd = -1;
	struct timespec _timeout = { 0, 100'000'000 };	// 100 ms

	static constexpr size_t read_buf_sz = 255;
	mutable unsigned char _read_buf[read_buf_sz];
	mutable std::mutex _serialMutex;
};
