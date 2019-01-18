#include "CSocket.h"

#include <iostream>

#define DEFAULT_BUFLEN 512

void GetErrorMessage(std::string&);

CSocket::CSocket(const char* aHost, const char* aPort)
	: mSock(INVALID_SOCKET), mHost(aHost), mPort(aPort)
{
}

CSocket::~CSocket()
{
	WSACleanup();
}

bool CSocket::init() {
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &mData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed with error: " << iResult << std::endl;
		return 1;
	}

	return true;
}

bool CSocket::connect(std::string& err) {
	int iResult;
	struct addrinfo *result = nullptr,
		*ptr = nullptr,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; //we only want IPv4 addresses
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(mHost, mPort, &hints, &result);
	if (iResult != 0) {
		err = std::string("getaddrinfo failed with error: ") + std::to_string(iResult);
		return false;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		// Create a SOCKET for connecting to server
		mSock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (mSock == INVALID_SOCKET) {
			GetErrorMessage(err);
			return false;
		}

		// Connect to server.
		iResult = ::connect(mSock, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			GetErrorMessage(err);
			closesocket(mSock);
			mSock = INVALID_SOCKET;

			continue;
		}

		break;
	}

	freeaddrinfo(result);

	if (mSock == INVALID_SOCKET) {
		//std::cout << "Unable to connect to server!" << std::endl;
		return false;
	}

	return true;
}

tResponse CSocket::sendMsg(const char* aVar, const char* aVal) {
	int iResult;
	std::string req;
	tResponse out;
	if (aVal)
		req = CMsg::createRequest(aVar, aVal); //set
	else
		req = CMsg::createRequest(aVar); // get

	// connect every time . new message -> new connection (as designed)
	if (!connect(out.second))
		return out;

	// Send an initial buffer
	iResult = ::send(mSock, req.c_str(), (int)req.size(), 0);
	if (iResult == SOCKET_ERROR) {
		GetErrorMessage(out.second);
		//std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
		closesocket(mSock);
		return out;
	}

	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	ZeroMemory(&recvbuf, recvbuflen);
	iResult = ::recv(mSock, recvbuf, recvbuflen, 0);
	if (iResult > 0) {
		// bool - err sign, string txt/err message
		out = CMsg::parseResponse(recvbuf, iResult);
		//break;
	}
	else if (iResult == 0) {
		out.second = "Connection closed";
	}
#ifdef _DEBUG
	else {
		std::cout << "recv failed with error: " << WSAGetLastError() << std::endl;
	}
#endif // _DEBUG


	// shutdown the connection since no more data will be sent
	iResult = ::shutdown(mSock, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		//std::cout << "shutdown failed with error: " << WSAGetLastError() << std::endl;
	}

	// cleanup
	closesocket(mSock);

	return out;
}

void GetErrorMessage(std::string& aOut) {
	wchar_t *s = NULL;

	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s, 0, NULL);

	// wchar* => std::string
	std::wstring ws(s);
	aOut = std::string(ws.begin(), ws.end());
	LocalFree(s);
}
