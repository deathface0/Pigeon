#pragma once
#ifdef _WIN32
#include "WindowsTcpClient.h"
#include <json/json.h>
#elif __linux__
#include "TcpClient.h"
#include <jsoncpp/json/json.h>
#endif

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <unordered_map>

#include "PigeonPacket.h"
#include "../Utils/Utils.h"

class PigeonClient {
public:
	PigeonClient(const std::string& host, unsigned short port, const std::string& username);
private:
	std::vector<unsigned char> ReadPacket();

	std::vector<unsigned char> SerializePacket(const PigeonPacket& packet);
	PigeonPacket DeserializePacket(std::vector<unsigned char>& packet);
	void* ProcessPacket();

public:
	PigeonPacket BuildPacket(PIGEON_OPCODE opcode, const std::string& username, const std::vector<unsigned char>& payload);
public:
	void Run();
	void SendPacket(const PigeonPacket& pkt);
	
	void SendFile(const std::string& filepath);
	void DownloadFile(const std::string& filename);

private:
	std::string m_host = "";
	unsigned short m_port = -1;
	std::string m_username = "";
	std::string m_servername = "";

	std::vector<std::string> filePaths;

	int Connect();
	PigeonPacket Handshake();


public:
	inline std::string GetUsername() const { return this->m_username; }
private:
#ifdef _WIN32
	WindowsTcpClient* m_client = nullptr;
#elif __linux__
	TcpClient* m_client = nullptr;
#endif
};