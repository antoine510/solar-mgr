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
protected:
	template<typename Tres, typename... T>
	Tres sendMessageWithResponse(uint8_t commandID, T... parameters) const {
		std::vector<uint8_t> msg = _header;
		msg.reserve(4 + sizeof...(parameters));
		msg.push_back(commandID);
		serialize(msg, parameters...);
		auto resVector = Bus::Instance().SendCommand(msg, sizeof(Tres));
		Tres res;
		std::memcpy(reinterpret_cast<uint8_t*>(&res), resVector.data(), sizeof(Tres));
		return res;
	}

	template<typename... T>
	void sendMessage(uint8_t commandID, T... parameters) const {
		std::vector<uint8_t> msg = _header;
		msg.reserve(4 + sizeof...(parameters));
		msg.push_back(commandID);
		serialize(msg, parameters...);
		Bus::Instance().SendCommand(msg);
	}

	enum CommonCommands : uint8_t {
		MAGIC = 0
	};

	std::vector<uint8_t> _header;

private:
	template <typename T, typename... Txs>
	static void serialize(std::vector<uint8_t>& msg, T x, const Txs&... xs) {
		const auto initialSize = msg.size();
		msg.resize(initialSize + sizeof(T));
		std::memcpy(msg.data() + initialSize, reinterpret_cast<uint8_t*>(&x), sizeof(T));
		serialize(msg, xs...);
	}
	static void serialize(std::vector<uint8_t>& msg) {}
};
