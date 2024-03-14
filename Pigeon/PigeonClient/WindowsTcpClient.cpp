#ifdef _WIN32

#include "WindowsTcpClient.h"

int WindowsTcpClient::ResolveDomainName()
{
	int resolve = getaddrinfo(this->host.c_str(), 0, nullptr, &dnsResult);
	if (resolve != 0) {
		std::cout << "Error while resolving hostname" << std::endl;
		return -1;
	}
	return resolve;
}

int WindowsTcpClient::SocketCreate()
{
	sAddr.sin_family = AF_INET;
	sAddr.sin_port = htons(port);

	addrinfo hints;
	hints.ai_family = AF_INET;

	if (ResolveDomainName() == -1) {
		std::cout << "getaddrinfo failed" << std::endl;
		return -1;
	}

	sockaddr_in* ipv4 = nullptr;
	for (addrinfo* result = dnsResult; result != nullptr; result = result->ai_next) {
		cSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (this->cSocket == -1) {
			continue;
		}

		ipv4 = (sockaddr_in*)(result->ai_addr);
		inet_ntop(AF_INET, &(ipv4->sin_addr), this->ipv4, sizeof(this->ipv4));
	}

	if (ipv4 != nullptr) {
		sAddr.sin_addr = ipv4->sin_addr;
	}


	if (this->cSocket == -1)
		return 1;

	return 0;
}

WindowsTcpClient::WindowsTcpClient(const std::string& host, unsigned short port) :
	host(host), port(port)
{

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cout << "Winsock dll was not found! " << WSAGetLastError() << std::endl;
		WSACleanup();
	}

	SSL_library_init();
	this->sslCtx = SSL_CTX_new(TLS_client_method());
	ssl = SSL_new(this->sslCtx);

	/* Dangerous */
	SSL_CTX_set_verify(sslCtx, SSL_VERIFY_NONE, nullptr);
}

int WindowsTcpClient::Connect()
{
	if (SocketCreate() != 0) {
		std::cout << "Failed to create socket " << WSAGetLastError() << std::endl;
		closesocket(cSocket);
		WSACleanup();
		return -1;
	}

	if (connect(cSocket, (SOCKADDR*)&sAddr, sizeof(sAddr)) == SOCKET_ERROR) {
		std::cout << "Failed to establish TCP handshake." << WSAGetLastError() << std::endl;
		closesocket(cSocket);
		WSACleanup();
		return -1;
	}

	SSL_set_fd(ssl, cSocket);

	if (SSL_connect(ssl) != 1) {
		std::cout << "Failed to establish SSL/TLS handshake." << std::endl;
		closesocket(cSocket);
		SSL_shutdown(ssl);
		SSL_free(ssl);
		WSACleanup();
		return -1;
	}

	return 0;
}

int WindowsTcpClient::Disconnect()
{
	// Close SSL socket 
	SSL_shutdown(ssl);
	SSL_free(ssl);

	// Close TCP socket 
	if (closesocket(cSocket) == SOCKET_ERROR) {
		std::cout << "Failed to close socket: " << WSAGetLastError() << std::endl;
		return 1;
	}

	// Cleanup Winsock
	WSACleanup();

	return 0;
}

int WindowsTcpClient::SendAll(std::vector<unsigned char>& buf)
{
	int totalSent = 0;
	int leftToSend = buf.size();
	int nSent;

	while (totalSent < buf.size()) {
		nSent = SSL_write(this->ssl, buf.data() + totalSent, leftToSend);

		if (nSent <= 0) {
			int error = SSL_get_error(ssl, nSent);
			std::cout << "Error while sending: " << error << std::endl;
			break;
		}

		totalSent += nSent;
		leftToSend -= nSent;

		if (leftToSend == 0) {
			break;
		}
	}

	return totalSent;
}

int WindowsTcpClient::SendAll(const std::string& toSend)
{
	int totalSent = 0;
	int leftToSend = toSend.size();

	int nSent;
	while (totalSent < toSend.size())
	{
		nSent = SSL_write(this->ssl, toSend.data() + totalSent, leftToSend);

		if (nSent <= 0);
		break;
		totalSent += nSent;
		leftToSend -= leftToSend;
	}
	return nSent;
}

int WindowsTcpClient::Recv(std::vector<unsigned char>& buf, size_t toRecv, int total)
{
	do {
		int nRecv = SSL_read(ssl, buf.data() + total, toRecv - total);
		if (nRecv <= 0)
			break;
		total += nRecv;

	} while (total < toRecv);

	return total;
}
#endif