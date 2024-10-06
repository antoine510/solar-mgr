#pragma once

#include <cstdint>
#include <cstring>

#include "Bus.hpp"

class BusModule {
public:
	BusModule(uint8_t moduleID) : _header{Bus::protocol1, Bus::protocol2, moduleID} {}

	bool CheckOnline() const {
		try {
			if(sendMessageWithResponse<uint8_t>((uint8_t)MAGIC) != 0x42) return false;
		} catch(const std::exception& e) {
			return false;
		}
		return true;
	}

	uint8_t GetModuleID() const { return _header[2]; }

	unsigned retryCount = 5;
protected:
	template<typename Tres, typename... T>
	Tres sendMessageWithResponse(uint8_t commandID, bool crcCheck = false, T... parameters) const {
		auto command = prepareCommand(commandID, parameters...);
		std::vector<uint8_t> response;
		unsigned retries = retryCount;
		do {
			try {
				Bus::Instance().SetBaudrate(Bus::GetRetryBaudrate(retryCount - retries));
				response = Bus::Instance().SendCommand(command, sizeof(Tres) + crcCheck);
				if(crcCheck && !Bus::CheckCRC(response)) {
					response.clear();
					std::this_thread::sleep_for(std::chrono::milliseconds(250));
				}
			}
			catch(const Bus::ReadTimeoutException&) { std::this_thread::sleep_for(std::chrono::milliseconds(250)); }
			catch(const Bus::ReadSizeException&) { std::this_thread::sleep_for(std::chrono::milliseconds(250)); }
			catch(const std::exception& e) { throw; }
		} while(!response.size() && retries--);
		if(!response.size()) throw std::runtime_error("No response");
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
