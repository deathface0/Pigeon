#include "PigeonClient.h"

PigeonClient::PigeonClient(const std::string& host, unsigned short port, const std::string& username):
    m_host(host),m_port(port),m_username(username)
{
#ifdef _WIN32
	this->m_client = new WindowsTcpClient(m_host, m_port);
#elif __linux__
	this->m_client = new TcpClient(m_host, m_port);
#endif
}

int PigeonClient::Connect()
{
	if (m_client != nullptr)
		return m_client->Connect();
	return -1;
}

PigeonPacket PigeonClient::Handshake()
{
    auto pkt = BuildPacket(CLIENT_HELLO, m_username,String::StringToBytes(R"({"status":"DND"})"));
    auto pktBytes = SerializePacket(pkt);

    int sent = this->m_client->SendAll(pktBytes);
    auto recv = this->ReadPacket();

    return DeserializePacket(recv);
}

std::vector<unsigned char> PigeonClient::ReadPacket()
{
    std::vector<unsigned char> packetBuffer(MAX_HEADER);

    int total = this->m_client->Recv(packetBuffer, 4, 0);

    //This could also mean error when receving. In case of error/wrong packet, empty vector is returned and the connection will be closed
    if (total < 4) {
        return {};
    }

    int headerLength = 0;
    for (int i = 0; i < 4; ++i) {
        headerLength |= packetBuffer[i] << (8 * i);
    }

    if (headerLength > MAX_HEADER) {
        std::cout << (" [FATAL] PACKET HEADER TOO BIG \n") << std::endl;
        return {};
    }

    total = this->m_client->Recv(packetBuffer, headerLength + 4, total);


    packetBuffer.resize(total);

    int payloadLength = 0;
    for (int i = packetBuffer.size() - 1; i >= packetBuffer.size() - 4; --i) {
        payloadLength = (payloadLength << 8) | packetBuffer[i];
    }

    if (payloadLength == 0) {
        return packetBuffer;
    }

    packetBuffer.resize(packetBuffer.size() + payloadLength);

    total = this->m_client->Recv(packetBuffer, headerLength + 4 + payloadLength, total);


    return packetBuffer;
}

std::vector<unsigned char> PigeonClient::SerializePacket(const PigeonPacket& packet)
{
    std::vector<unsigned char> serializedPacket;

    int headerLength = packet.HEADER.HEADER_LENGTH;
    unsigned char* headerLengthBytes = reinterpret_cast<unsigned char*>(&headerLength);
    serializedPacket.insert(serializedPacket.end(), headerLengthBytes, headerLengthBytes + sizeof(int));

    std::time_t timestamp = packet.HEADER.TIME_STAMP;
    unsigned char* timestampBytes = reinterpret_cast<unsigned char*>(&timestamp);
    serializedPacket.insert(serializedPacket.end(), timestampBytes, timestampBytes + sizeof(std::time_t));

    serializedPacket.insert(serializedPacket.end(), packet.HEADER.username.begin(), packet.HEADER.username.end());
    serializedPacket.push_back('\0');

    serializedPacket.push_back(static_cast<unsigned char>(packet.HEADER.OPCODE));

    int contentLength = packet.HEADER.CONTENT_LENGTH;
    unsigned char* contentLengthBytes = reinterpret_cast<unsigned char*>(&contentLength);
    serializedPacket.insert(serializedPacket.end(), contentLengthBytes, contentLengthBytes + sizeof(int));

    serializedPacket.insert(serializedPacket.end(), packet.PAYLOAD.begin(), packet.PAYLOAD.end());

    return serializedPacket;
}

PigeonPacket PigeonClient::DeserializePacket(std::vector<unsigned char>& packet)
{
    if (packet.empty())
        return PigeonPacket();

    PigeonPacket packetRet;

    int offset = 0;

    //String::printBytesInHex(packet);
    std::memcpy(&packetRet.HEADER.HEADER_LENGTH, &packet[offset], sizeof(int));
    offset += sizeof(int);

    std::memcpy(&packetRet.HEADER.TIME_STAMP, &packet[offset], sizeof(std::time_t));
    offset += sizeof(std::time_t);

    while (packet[offset] != '\0') {
        packetRet.HEADER.username.push_back(packet[offset]);
        offset++;
    }

    offset++;
    //opc
    packetRet.HEADER.OPCODE = static_cast<PIGEON_OPCODE>(packet[offset]);
    offset++;



    std::memcpy(&packetRet.HEADER.CONTENT_LENGTH, &packet[offset], sizeof(int));
    offset += sizeof(int);

    packetRet.PAYLOAD.assign(packet.begin() + offset, packet.end());
    return packetRet;
}

PigeonPacket PigeonClient::BuildPacket(PIGEON_OPCODE opcode, const std::string& username, const std::vector<unsigned char>& payload)
{
    PigeonPacket pkt;
    pkt.HEADER.OPCODE = opcode;
    pkt.PAYLOAD = payload;
    pkt.HEADER.CONTENT_LENGTH = pkt.PAYLOAD.size();
    pkt.HEADER.TIME_STAMP = std::time(0);
    pkt.HEADER.username = username;
    pkt.HEADER.HEADER_LENGTH = sizeof(std::time_t) +
        pkt.HEADER.username.length() + 1 +
        sizeof(unsigned char) +
        sizeof(int);
    return pkt;

}

void* PigeonClient::ProcessPacket()
{
    Json::Reader reader;
    Json::Value value;

    if (Connect() == -1) {
        //Disconnect & handle GUI
        return nullptr;
    }

    auto sHello = Handshake();

    if (sHello.PAYLOAD.empty() || sHello.HEADER.OPCODE != SERVER_HELLO) {
        //BAD PACKET Disconnect & handle GUI
        return nullptr;
    }

    if (!reader.parse(std::string(sHello.PAYLOAD.begin(), sHello.PAYLOAD.end()), value)) {
        //BAD PACKET Disconnect & handle GUI (it wont happen anyway)
        return nullptr;
    }

    std::string servername = "";
    servername = value["ServerName"].asString();
    PigeonClientGUIInfo::MOTD = value["MOTD"].asString();

    if (servername.empty()) {
        //server didnt send servername (it wont happen)
        return nullptr;
    }

    m_servername = servername;

    std::cout << "CONNECTED TO " << m_servername << std::endl;

    std::string cmsg = "";
    while (true) {
        std::vector<unsigned char> recv = ReadPacket();

        if (recv.empty()) {
            //Disconnect & handle GUI
            m_connected = false;

            std::cout << "DISCONNECTED" << std::endl;

            PigeonClientGUIInfo::msgBuffer.clear();
            Texture::textures.clear();

            return nullptr;
        }

        auto pkt = DeserializePacket(recv);

        std::cout << std::hex << pkt.HEADER.OPCODE << std::dec << std::endl;

        switch (pkt.HEADER.OPCODE)
        {
        case PRESENCE_UPDATE:

            if (!reader.parse(std::string(pkt.PAYLOAD.begin(), pkt.PAYLOAD.end()), value)) {
                //bad json, again, this wont really happen
                break;
            }

            PigeonClientGUIInfo::Users.clear(); //Clear users map
            for (Json::ValueIterator it = value.begin(); it != value.end(); ++it) {
                std::string key = it.key().asString();
                std::string value = (*it).asString();
                
                std::cout << value << std::endl;

                //Update GUI with updated clients
                PigeonClientGUIInfo::Users.insert(std::make_pair(key, value));
            }


            break;
        case TEXT_MESSAGE:
            //send message to GUI...
            cmsg = std::string(pkt.PAYLOAD.begin(), pkt.PAYLOAD.end());
            //PigeonClientGUIInfo::msgBuffer.appendf("%s\n", cmsg.c_str());
            PigeonClientGUIInfo::msgBuffer.push_back({ {}, MSG_TYPE::PIGEON_TEXT, pkt.HEADER.TIME_STAMP , pkt.HEADER.username, cmsg });

            //std::thread([&]() {m_soundPlayer.play(PigeonClientGUIInfo::msgAudioPath); }).detach();

            std::cout << pkt.HEADER.username << ": " << std::string(pkt.PAYLOAD.begin(), pkt.PAYLOAD.end()) << std::endl;
            break;

        case MEDIA_FILE:
        {

            if (!reader.parse(std::string(pkt.PAYLOAD.begin(), pkt.PAYLOAD.end()), value)) {
                //bad json, again, this wont really happen
                break;
            }

            std::string filename = value["filename"].asString();
            std::string ext = value["ext"].asString();

            if (filename.empty()) {
                break;
            }

            if (ext == "png" || ext == "jpg" || ext == "jpeg")
            {
                std::string json = R"({"filename":")" + std::to_string(pkt.HEADER.TIME_STAMP) + '_' + filename + R"("})";
                PigeonPacket downPacket = BuildPacket(MEDIA_DOWNLOAD, pkt.HEADER.username, std::vector<unsigned char>(json.begin(), json.end()));
                SendPacket(downPacket);

                break;
            }

            PigeonClientGUIInfo::msgBuffer.push_back({{}, MSG_TYPE::PIGEON_FILE, pkt.HEADER.TIME_STAMP, pkt.HEADER.username, filename + '.' + ext});

            std::cout << "New media file by: " << pkt.HEADER.username << " Filename: " << filename << std::endl;
            filePaths.push_back(filename);
            break;
        }
        case ACK_MEDIA_DOWNLOAD:
        {
            if (!reader.parse(std::string(pkt.PAYLOAD.begin(), pkt.PAYLOAD.end()), value)) {
                //bad json, again, this wont really happen
                break;
            }

            std::string filename = value["filename"].asString();
            std::string ext = value["ext"].asString();
            std::string content = value["content"].asString();

            if (filename.empty() || ext.empty() || content.empty())
                break;

            auto buf = String::StringToBytes(B64::base64_decode(content));

            if (ext != "png" && ext != "jpg" && ext != "jpeg")
            {
                PigeonClientGUIInfo::msgBuffer.push_back({ {}, MSG_TYPE::PIGEON_FILE, pkt.HEADER.TIME_STAMP, pkt.HEADER.username, filename + '.' + ext });
                std::cout << File::BufferToDisk(buf, PigeonClientGUIInfo::donwloadPath + '/' + filename + '.' + ext) << std::endl;
                break;
            }

            PigeonClientGUIInfo::msgBuffer.push_back({ buf, MSG_TYPE::PIGEON_FILE, pkt.HEADER.TIME_STAMP, pkt.HEADER.username, filename + '.' + ext });
            break;
        }            
        default:
            break;
        }
        value.clear();

        m_connected = true; //Connected after receiving first pkg
    }

    return nullptr;
}

void PigeonClient::Run()
{
    std::thread([&] {
        ProcessPacket();
    }).detach();
}

void PigeonClient::SendPacket(const PigeonPacket& pkt)
{
    auto bytes = SerializePacket(pkt);
    this->m_client->SendAll(bytes);
}

void PigeonClient::ChangeStatus(const std::string& status)
{
    std::string m = R"({"status":)" + std::string(R"(")") + status + std::string(R"(")") + "}";
    

    PigeonPacket pkg = BuildPacket(PIGEON_OPCODE::PRESENCE_UPDATE, m_username, String::StringToBytes(m));
    
    //String::printBytesInHex(SerializePacket(pkg));
    
    SendPacket(pkg);
}

void PigeonClient::SendMsg(const std::string& message)
{
    PigeonPacket pkg = BuildPacket(PIGEON_OPCODE::TEXT_MESSAGE, m_username, String::StringToBytes(message));

    SendPacket(pkg);
}

void PigeonClient::SendFile(const std::string& filepath)
{
    std::string filename = std::to_string(std::time(0)) + "_" + this->m_username + "_" + File::GetFilenameFromPath(filepath);
    std::string ext = File::GetFileExtension(filepath);
    std::string content = B64::base64_encode(String::BytesToString(File::DiskToBuffer(filepath)));

    std::string toSend = R"({"content":")" + content + R"(", "filename": ")" + filename + R"(", "ext":")" + ext + R"("})";

    if (content.empty() || ext.empty())
        return;

    auto pkt = BuildPacket(MEDIA_FILE, this->m_username, String::StringToBytes(toSend));
    SendPacket(pkt);
}

void PigeonClient::DownloadFile(const std::string& filename)
{
    std::string toSend = R"({"filename":")" + filename + R"("})";
    auto pkt = BuildPacket(MEDIA_DOWNLOAD, this->m_username, String::StringToBytes(toSend));
    SendPacket(pkt);
}
