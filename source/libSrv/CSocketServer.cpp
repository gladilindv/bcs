#include "CSocketServer.h"

CSocketServer::CSocketServer(int aPort, int aConnections) : mSock(INVALID_SOCKET)
{
	WSADATA info;
	if (WSAStartup(MAKEWORD(2, 0), &info)) {
		throw "Could not start WSA";
	}

	sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));

	// only IPv4 addresses
	sa.sin_family = AF_INET;
	sa.sin_port = htons(aPort);

	mSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mSock == INVALID_SOCKET) {
		WSACleanup();
		throw "INVALID_SOCKET";
	}

	/* bind the socket to the internet address */
	if (bind(mSock, (sockaddr *)&sa, sizeof(sockaddr_in)) == SOCKET_ERROR) {
		closesocket(mSock);
		WSACleanup();
		throw "INVALID_SOCKET";
	}

	if (listen(mSock, aConnections) == SOCKET_ERROR) {
		closesocket(mSock);
		WSACleanup();
		throw "BIND_FAILED";
	}
}

SOCKET CSocketServer::accept() {
	// Accept a client socket		
	SOCKET newSock = ::accept(mSock, NULL, NULL);
	if (newSock == INVALID_SOCKET) {
		// WSAGetLastError()
		throw "accept failed with error: %d";
	}

	return newSock;
}