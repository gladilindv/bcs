#pragma once

#include <string>
#include <sstream>

#define LOG_NONE	0
#define LOG_FATAL	1
#define LOG_ERROR	2
#define LOG_WARNING 3
#define LOG_INFO	4

class CLogger
{
public:
	static CLogger& getInstance() {
		static CLogger instance; // Guaranteed to be destroyed.
								 // Instantiated on first use.
		return instance;
	}

	void init(const std::string& aPath);
	void log(const std::string& aMsg, int aType = LOG_INFO);

private:
	CLogger() {}

	std::string info();

	std::ostringstream _tsbuff;
	HANDLE hFile;
};

