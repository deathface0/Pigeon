#ifdef _WIN32


#pragma once

#include<iostream>
#include<vector>
#include<string>
#include<chrono>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>
#include<openssl/ssl.h>
#include<openssl/err.h>
#include<unordered_map>
#include <locale>
#include <cstdlib>
#include<ip2string.h>

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Ntdll.lib")


class WindowsTcpClient
{
private:

    char ipv4[INET_ADDRSTRLEN];

    WSADATA wsaData;

    int cSocket = -1;
    sockaddr_in sAddr;

    addrinfo* dnsResult = nullptr, * iter = nullptr;

    std::string host;
    unsigned short port;

    SSL_CTX* sslCtx = nullptr;
    SSL* ssl = nullptr;

    int SSL_ERROR;

private:
    int ResolveDomainName();
    int SocketCreate();

public:
    WindowsTcpClient(const std::string& host, unsigned short port);

public:
    const int GetTlsVersion();
    const std::string GetCipher();

    int Connect();

    int SendAll(std::vector<unsigned char>& buf);
    int SendAll(const std::string& toSend);

    int Recv(std::vector<unsigned char>& buf, size_t toRecv, int total=0);
};

#endif