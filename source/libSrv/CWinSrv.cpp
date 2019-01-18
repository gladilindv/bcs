#include "CWinSrv.h"

#include "CSocketServer.h"
#include "CLogger.h"
#include "CSQLLite.h"

#include "CMsg.h"

#pragma comment(lib,"ws2_32")

#define DEFAULT_BUFLEN 512

CSocketServer* CWinSrv::mSrv = nullptr;
CSQLLite* CWinSrv::mSQL = nullptr;

std::string GetExeFileName()
{
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	return std::string(buffer);
}

std::string GetExePath()
{
	// std::filesystem::path cwd = std::filesystem::current_path();/
	std::string f = GetExeFileName();
	return f.substr(0, f.find_last_of("\\/"));
}

CWinSrv::CWinSrv() : mIsStarted(false), mSessionTimeLimit(3000) {
	// default params 
	int port = 27015;
	int conn_count = 5;

	// prepare logger
	std::string curPath = GetExePath();
	CLogger::getInstance().init(curPath + "\\app.txt");

	// read INI
	std::string iniFile = curPath + "\\app.ini";
	if (GetFileAttributesA(iniFile.c_str()) != INVALID_FILE_ATTRIBUTES) {
		port = GetPrivateProfileIntA("global", "port", port, iniFile.c_str());
		conn_count = GetPrivateProfileIntA("global", "conn_count", conn_count, iniFile.c_str());
		mSessionTimeLimit = GetPrivateProfileIntA("global", "time_limit", mSessionTimeLimit, iniFile.c_str());
	}

	// init server
	mSrv = new CSocketServer(port, conn_count);
	log((std::string("init server on port ") + std::to_string(port)).c_str());

	// init database
	mSQL = new CSQLLite;
	mSQL->open(curPath + "\\app.db");
}

CWinSrv::~CWinSrv() {
	log("destructor");

	mSQL->close();
	delete mSQL;
	mSQL = nullptr;

	delete mSrv;
	mSrv = nullptr;
}

void CWinSrv::log(const char* aMsg) {
	CLogger::getInstance().log(aMsg);
}

void CWinSrv::start() {
	log("starting server");
	DWORD tId;
	mSrvThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&runThread, (void*)this, 0, &tId);
}

void CWinSrv::stop() {
	log("stopping server");
	mSrv->close();
	CloseHandle(mSrvThread);
}

unsigned long __stdcall CWinSrv::runThread(void* lpParam) {
	DWORD dwStartTick = GetTickCount();

	CWinSrv* srv = reinterpret_cast<CWinSrv*>(lpParam);

	HANDLE hThread = NULL;
	log("runThread started");

	while (mSrv != nullptr) {
		SOCKET client = mSrv->accept();
		log("client connected");

		// setup connection time limit
		DWORD timeout = srv->mSessionTimeLimit;
		setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
		setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

		hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&msgThread, (void*)client, 0, NULL);
		CloseHandle(hThread); // to prevent memory leak
	}

	return 0;
}

unsigned long __stdcall CWinSrv::msgThread(void* lpParam) {
#ifdef _DEBUG
	DWORD dwStartTick = GetTickCount();
	log("msgThread started");
#endif // DEBUG

	SOCKET clientSocket = (SOCKET)lpParam;

	int iResult;
	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN] = {};
	int recvbuflen = DEFAULT_BUFLEN;
	do {
		// check database state
		if (!mSQL->state()) {
			std::string s = CMsg::createResponse(false, "DB closed");
			send(clientSocket, s.c_str(), strlen(s.c_str()), 0);
			break;
		}

		ZeroMemory(&recvbuf, recvbuflen);
		iResult = recv(clientSocket, recvbuf, recvbuflen, 0);

		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);
			// switch commands
			tRequest r = CMsg::parseRequest(recvbuf, iResult);
			bool ok = false;
			short cmd = std::get<0>(r);
			std::string var = std::get<1>(r);
			std::string out = "";

			std::string l = std::string("cmd=") + ((cmd == 0) ? "GET" : "SET") + "\tvar=" + var + ((cmd == 1) ? ("\tval=" + std::get<2>(r)) : "");
			log(l.c_str());

			tData d;
			if (cmd == -1) {
				out = "request parse error";
			}
			if (cmd == 0) {
				if (mSQL->get(var, d)) {
					// sql ok 
					if (d.second)
						out = d.first;
					else
						out = "variable not found";
					ok = true;
				}
				else {
					out = "sql error";
				}
			}

			if (cmd == 1) {
				d.first = std::get<2>(r);
				if (mSQL->set(var, d)) {
					out = "sql ok";
					ok = true;
				}
				else {
					out = "sql error";
				}
			}

			// response to the sender
			std::string s = CMsg::createResponse(ok, out);
			iSendResult = send(clientSocket, s.c_str(), s.size(), 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				break;
			}
			printf("Bytes sent: %d\n", iSendResult);
			// if complete command readed -> shutdown 
			log("connection close by server");
			break;
		}
		else if (iResult == 0) {
			printf("Connection closing...\n");
			log("connection close by client");
			break;
		}
		else if (iResult == -1 && WSAGetLastError() == WSAETIMEDOUT) {
			DWORD err = WSAGetLastError();
			log("connection close by timeout");
			break;
		}
		else {
			DWORD err = WSAGetLastError();
			printf("recv failed with error: %d\n", err);
			log("msgThread failed with error");
			break;
		}

	} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(clientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
	}

#ifdef _DEBUG
	DWORD dwTimeElapsed = GetTickCount() - dwStartTick;
	printf("elapsed %ld \n", dwTimeElapsed);
	log("msgThread completed");
#endif // DEBUG

	closesocket(clientSocket);
	ExitThread(0);
	return 0;
}