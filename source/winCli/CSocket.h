#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>

#include "CMsg.h"

class CSocket
{
public:
	CSocket(const char*, const char*);
	~CSocket();

	bool init();

	tResponse sendMsg(const char*, const char* = nullptr);

private:
	WSADATA mData;
	SOCKET mSock;

	const char* mHost;
	const char* mPort;
	bool connect(std::string&);
};

