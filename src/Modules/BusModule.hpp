#pragma once

#include <cstdint>
#include <cstring>
#include <thread>

#include "Bus.hpp"

class BusModule {
public:
	BusModule(uint8_t moduleID, bool crcCheck = false) : _header{Bus::protocol1, Bus::protocol2, moduleID}, _crcCheck(crcCheck) {}

	bool CheckOnline() const {
		try {
			if(sendMessageWithResponse<uint8_t>((uint8_t)MAGIC) != 0x42) return false;
		} catch(const std::exception& e) {
			return false;
		}
		return true;
	}

	uint8_t GetModuleID() const { return _header[2]; }

	class NoResponseException : public std::runtime_error {
	public:
		NoResponseException() : std::runtime_error("No response") {}
	};

protected:
	template<typename Tres, typename... T>
	Tres sendMessageWithResponse(uint8_t commandID, unsigned numRetries = 4, T... parameters) const {
		auto command = prepareCommand(commandID, parameters...);
		std::vector<uint8_t> response;
		unsigned tryCount = 0;
		do {
			try {
				Bus::Instance().SetBaudrate(Bus::GetTryBaudrate(tryCount));
				response = Bus::Instance().SendCommand(command, sizeof(Tres) + _crcCheck);
				if(_crcCheck && !Bus::CheckCRC(response)) {
					response.clear();
					std::this_thread::sleep_for(std::chrono::milliseconds(250));
				}
			}
			catch(const Bus::ReadTimeoutException&) { std::this_thread::sleep_for(std::chrono::milliseconds(250)); }
			catch(const Bus::ReadSizeException&) { std::this_thread::sleep_for(std::chrono::milliseconds(250)); }
			catch(const std::exception& e) { throw; }
		} while(!response.size() && tryCount++ < numRetries);
		if(!response.size()) throw NoResponseException();
		return deserialize<Tres>(response);
	}

	template<typename... T>
	void sendMessage(uint8_t commandID, T... parameters) const {
		Bus::Instance().SendCommand(prepareCommand(commandID, parameters...));
	}

	enum CommonCommands : uint8_t {
		MAGIC = 0
	};

	std::vector<uint8_t> _header;
	bool _crcCheck;

private:
	template<typename... T>
	auto prepareCommand(uint8_t commandID, T... parameters) const {
		std::vector<uint8_t> msg = _header;
		msg.reserve(4 + sizeof...(parameters));
		msg.push_back(commandID);
		serialize(msg, parameters...);
		return msg;
	}

	template <typename T, typename... Txs>
	static void serialize(std::vector<uint8_t>& msg, T x, const Txs&... xs) {
		const auto initialSize = msg.size();
		msg.resize(initialSize + sizeof(T));
		std::memcpy(msg.data() + initialSize, reinterpret_cast<uint8_t*>(&x), sizeof(T));
		serialize(msg, xs...);
	}
	static void serialize(std::vector<uint8_t>& msg) {}

	template<typename Tres>
	static Tres deserialize(const std::vector<uint8_t>& response) {
		Tres res;
		std::memcpy(reinterpret_cast<uint8_t*>(&res), response.data(), sizeof(Tres));
		return res;
	}
};
