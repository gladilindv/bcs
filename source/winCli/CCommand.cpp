#include "CCommand.h"
#include "CSocket.h"

#include "CMsg.h"

#include <iostream>
#include <iomanip>
#include <ctime>

CCommand::CCommand(std::unique_ptr<CSocket> && aSock)
	: mSock(std::move(aSock)) {
	std::srand(unsigned(std::time(0)));
}

void CCommand::run() {
	std::string cmd, var, val;
	while (true) {
		std::cout << "> ";
		std::cin >> cmd;
		if (cmd.compare("exit") == 0 || cmd.compare("quit") == 0)
			break;

		if (cmd.compare("GET") == 0) {
			std::cin >> var;
			doGet(var);
			continue;
		}

		if (cmd.compare("GETT") == 0) {
			std::cin >> var >> val;
			int tm = atoi(val.c_str());
			if (tm < 0) tm = 0;			// min 0ms
			if (tm > 10000) tm = 10000; // max 10sec

			while (true) {
				doGet(var);
				Sleep(tm);
			}
			continue;
		}

		if (cmd.compare("SET") == 0) {
			std::cin >> var >> val;
			doSet(var, val);
			continue;
		}

		if (cmd.compare("SETRT") == 0) {
			std::cin >> var >> val;
			int tm = atoi(val.c_str());
			if (tm < 0) tm = 0;			// min 0ms
			if (tm > 10000) tm = 10000; // max 10sec

			while (true) {
				doSet(var, std::to_string(std::rand()));
				Sleep(tm);
			}
			continue;
		}
		std::cout << "Command not supported." << std::endl;
	}
}

void CCommand::doGet(const std::string& aVar) {
	tResponse out = mSock->sendMsg(aVar.c_str());
	if (!out.first)
		std::cout << "ERROR: ";
	std::cout << out.second << std::endl;
}

void CCommand::doSet(const std::string& aVar, const std::string& aVal) {
	// log value to stdout (protocol)
	std::time_t t = std::time(nullptr);
	struct tm newtime;
	localtime_s(&newtime, &t);
	std::cout << std::put_time(&newtime, "%T") << " set " << aVar << " to " << aVal << std::endl;

	tResponse out = mSock->sendMsg(aVar.c_str(), aVal.c_str());
	if (!out.first)
		std::cout << "ERROR: ";
	std::cout << out.second << std::endl;
}