#pragma once

#include <cstdint>
#include <cstring>

#include "Bus.hpp"

class BusModule {
public:
	BusModule(uint8_t moduleID) : _rawMessage{Bus::protocol1, Bus::protocol2, moduleID} {}

	bool CheckOnline() {
		try {
			if(sendMessageWithResponse<uint8_t>((uint8_t)MAGIC) != 0x42) return false;
		} catch(const std::exception& e) {
			return false;
		}
		return true;
	}
protected:
	template<typename Tres, typename... T>
	Tres sendMessageWithResponse(uint8_t commandID, T... parameters) {
		_rawMessage.push_back(commandID);
		_rawMessage.reserve(4 + sizeof...(parameters));
		serialize(parameters...);
		auto resVector = Bus::Instance().SendCommand(_rawMessage, sizeof(Tres));
		_rawMessage.resize(3);
		Tres res;
		std::memcpy(reinterpret_cast<uint8_t*>(&res), resVector.data(), sizeof(res));
		return res;
	}

	template<typename... T>
	void sendMessage(uint8_t commandID, T... parameters) {
		_rawMessage.push_back(commandID);
		_rawMessage.reserve(4 + sizeof...(parameters));
		serialize(parameters...);
		Bus::Instance().SendCommand(_rawMessage);
		_rawMessage.resize(3);
	}

	enum CommonCommands : uint8_t {
		MAGIC = 0
	};

	std::vector<uint8_t> _rawMessage;

private:
	template <typename T, typename... Txs>
	void serialize(T x, const Txs&... xs) {
		const auto initialSize = _rawMessage.size();
		_rawMessage.resize(initialSize + sizeof(T));
		std::memcpy(_rawMessage.data() + initialSize, reinterpret_cast<uint8_t*>(&x), sizeof(T));
		serialize(xs...);
	}
	void serialize() {}
};
