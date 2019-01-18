#pragma once

#include <WinSock2.h>

class CSocketServer
{
public:
	CSocketServer(int aPort, int aConnections);
	~CSocketServer() {
		WSACleanup();
	};

	CSocketServer(const CSocketServer&) = delete;
	CSocketServer& operator=(CSocketServer&) = delete;

	SOCKET accept();
	void close() {
		closesocket(mSock);
	}

private:
	SOCKET mSock;
};

