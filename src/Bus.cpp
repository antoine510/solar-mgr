#include "Bus.hpp"

#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <asm/termbits.h>
#include <sys/ioctl.h>

#define GET_ERROR_STR std::string(strerror(errno))
#define GET_ERROR errno

Bus::Bus(const std::string& path, int baudrate) : _currentBaudrate(baudrate) {
	if (_fd = ::open(path.c_str(), O_RDWR | O_NOCTTY | O_SYNC); _fd < 0)
		throw std::runtime_error("Open failed: " + GET_ERROR_STR);

	termios2 tc{};

	tc.c_iflag = 0;
	tc.c_oflag = 0;
	tc.c_lflag = 0;
	tc.c_cflag = CLOCAL | CS8 | BOTHER;
	tc.c_cc[VMIN] = read_buf_sz;	// At most fill the buffer
	tc.c_cc[VTIME] = 1; // Inter-byte timeout of 0.1s
	tc.c_ispeed = baudrate;
	tc.c_ospeed = baudrate;

	if(ioctl(_fd, TCSETS2, &tc) != 0) {
		::close(_fd);
		throw std::runtime_error("TCSETS2 failed: " + GET_ERROR_STR);
	}
}

Bus::~Bus() noexcept {
	if (_fd > -1) ::close(_fd);
	_fd = -1;
}

void Bus::SetBaudrate(int baudrate) {
	if(baudrate == _currentBaudrate) return;
	termios2 tc{};
	if(ioctl(_fd, TCGETS2, &tc) != 0) throw std::runtime_error("TCGETS2 failed: " + GET_ERROR_STR);
	tc.c_ispeed = baudrate;
	tc.c_ospeed = baudrate;
	if(ioctl(_fd, TCSETS2, &tc) != 0) throw std::runtime_error("TCSETS2 failed: " + GET_ERROR_STR);
	_currentBaudrate = baudrate;
}

void Bus::waitForData() const {
	fd_set rx_fd_set{};
	FD_SET(_fd, &rx_fd_set);

	int res = pselect(_fd + 1, &rx_fd_set, nullptr, nullptr, &_timeout, nullptr);
	if(res < 0) {
		throw std::runtime_error("Select failed: " + GET_ERROR_STR);
	} else if(res == 0) {
		throw ReadTimeoutException();
	}
}

std::vector<uint8_t> Bus::Read(size_t expectedSize) const {
	waitForData();

	int read_len = ::read(_fd, _read_buf, expectedSize);
	if (read_len != expectedSize) throw ReadSizeException();

	auto ret = std::vector<uint8_t>(expectedSize);
	std::memcpy(ret.data(), _read_buf, expectedSize);
	return ret;
}

void Bus::Write(const uint8_t* buf, size_t sz) const {
	if (::write(_fd, buf, sz) != sz) throw WriteException();
}
