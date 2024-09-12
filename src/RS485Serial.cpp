#include "RS485Serial.hpp"

#include <iostream>
#include <vector>
#include <thread>
#include <cstring>

#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h> // for close()
#include <fcntl.h>
#include <termios.h>
#include <string.h>

#define GET_ERROR_STR std::string(strerror(errno))
#define GET_ERROR errno

static int _define_from_baudrate(int baudrate) {
	switch (baudrate) {
	case 9600:
		return B9600;
	case 19200:
		return B19200;
	case 38400:
		return B38400;
	case 57600:
		return B57600;
	case 115200:
		return B115200;
	case 230400:
		return B230400;
	case 460800:
		return B460800;
	case 500000:
		return B500000;
	case 576000:
		return B576000;
	case 921600:
		return B921600;
	case 1000000:
		return B1000000;
	case 1152000:
		return B1152000;
	case 1500000:
		return B1500000;
	case 2000000:
		return B2000000;
	case 2500000:
		return B2500000;
	case 3000000:
		return B3000000;
	case 3500000:
		return B3500000;
	case 4000000:
		return B4000000;
	default:
		throw std::runtime_error("Unknown baudrate " + std::to_string(baudrate));
	}
}


RS485Serial::RS485Serial(const std::string& path, int baudrate) {
	int readBufferSize = 1024;
	int writeBufferSize = 1024;

	// open() hangs on macOS or Linux devices(e.g. pocket beagle) unless you give it O_NONBLOCK
	_fd = ::open(path.c_str(), O_RDWR | O_NOCTTY);
	if (_fd == -1) throw std::runtime_error("Open failed: " + GET_ERROR_STR);

	// We need to clear the O_NONBLOCK again because we can block while reading
	// as we do it in a separate thread.
	if (fcntl(_fd, F_SETFL, 0) == -1) throw std::runtime_error("fcntl failed: " + GET_ERROR_STR);

	struct termios tc;
	bzero(&tc, sizeof(tc));

	if (tcgetattr(_fd, &tc) != 0) {
		::close(_fd);
		throw std::runtime_error("tcgetattr failed: " + GET_ERROR_STR);
	}

	tc.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
	tc.c_oflag &= ~(OCRNL | ONLCR | ONLRET | ONOCR | OFILL | OPOST);
	tc.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG | TOSTOP);
	tc.c_cflag &= ~(CSIZE | PARENB | CRTSCTS);
	tc.c_cflag |= CS8;

	tc.c_cc[VMIN] = 0;
	tc.c_cc[VTIME] = 5;

	tc.c_cflag |= CLOCAL; // Without this a write() blocks indefinitely.

	const int baudrate_or_define = _define_from_baudrate(baudrate);

	if (cfsetspeed(&tc, baudrate_or_define) != 0) {
		::close(_fd);
		throw std::runtime_error("cfsetspeed failed: " + GET_ERROR_STR);
	}

	if (tcsetattr(_fd, TCSANOW, &tc) != 0) {
		::close(_fd);
		throw std::runtime_error("tcsetattr failed: " + GET_ERROR_STR);
	}
}

RS485Serial::~RS485Serial() noexcept {
	if (_fd > -1) ::close(_fd);
	_fd = -1;
}

int RS485Serial::ReadCurrentProducers_mA() {
	CommandHeader cmd;
	cmd.identity = currentProducersAddress;
	cmd.command = (uint8_t)CurrentCommandID::READ_CURRENT;
	write((uint8_t*)&cmd, sizeof(CommandHeader));
	auto resp = read();
	if(resp.size() != sizeof(int)) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		return ReadCurrentProducers_mA();
	}
	return 8 - (*(int32_t*)resp.data());	// Remaining calibration
}

int RS485Serial::ReadCurrentConsumers_mA() {
	CommandHeader cmd;
	cmd.identity = currentConsumersAddress;
	cmd.command = (uint8_t)CurrentCommandID::READ_CURRENT;
	write((uint8_t*)&cmd, sizeof(CommandHeader));
	auto resp = read();
	if(resp.size() != sizeof(int)) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		return ReadCurrentConsumers_mA();
	}
	return -15 - (*(int32_t*)resp.data());	// Remaining calibration
}

std::array<MPPTData, RS485Serial::MPPTCount> RS485Serial::ReadAllMPPTs() {
	CommandHeader cmd;
	cmd.command = (uint8_t)MPPTCommandID::READ_ALL;
	std::array<MPPTData, MPPTCount> res{};
	for(int i = 0; i < MPPTCount; ++i) {
		cmd.identity = i + MPPTStartAddress;
		write((uint8_t*)&cmd, sizeof(CommandHeader));
		auto resp = read();
		if(resp.size() != sizeof(MPPTData)) continue;
		res[i] = *(MPPTData*)resp.data();
	}
	return res;
}

void RS485Serial::SetMaxWiper(int mpptID, uint8_t max) {
	CommandHeader cmd;
	cmd.identity = mpptID;
	cmd.command = (uint8_t)MPPTCommandID::SET_MAX_WIPER;
	uint8_t* s = (uint8_t*)malloc(sizeof(cmd) + 1);
	memcpy(s, (const void*)&cmd, sizeof(cmd));
	s[sizeof(cmd)] = max;
	write(s, sizeof(cmd) + 1);
	free(s);
}

void RS485Serial::SetOutputEnabled(int mpptID, bool en) {
	CommandHeader cmd;
	cmd.identity = mpptID;
	cmd.command = en ? (uint8_t)MPPTCommandID::SET_OUTPUT_ENABLED : (uint8_t)MPPTCommandID::SET_OUTPUT_DISABLED;
	write((uint8_t*)&cmd, sizeof(CommandHeader));
}

std::vector<uint8_t> RS485Serial::read() const {
	int read_len = 0;
	uint8_t _readBuffer[1024];

	read_len = ::read(_fd, _readBuffer, sizeof(_readBuffer));

	if (read_len <= 0) return std::vector<uint8_t>();

	auto ret = std::vector<uint8_t>(read_len);
	std::memcpy(ret.data(), _readBuffer, read_len);
	return ret;
}

void RS485Serial::write(const uint8_t* cmd_buf, size_t sz) const {
	int sent_len = static_cast<int>(::write(_fd, cmd_buf, sz));
	if (sent_len != sz) {
		std::cerr << "Write wrong count: " << sent_len << " excpected " << sz << std::endl;
		return;
	}
}
