#include "stdafx.h"
#include "CLogger.h"

#include <iomanip>

CRITICAL_SECTION CriticalSection;

void CLogger::init(const std::string& aPath) {

	InitializeCriticalSection(&CriticalSection);

	hFile = CreateFileA(aPath.c_str(),                // name of the write
		FILE_APPEND_DATA,       // open for writing
		FILE_SHARE_READ | FILE_SHARE_WRITE, // do not share
		NULL,                   // default security
		OPEN_ALWAYS,			// OPEN_ALWAYS
		FILE_ATTRIBUTE_NORMAL,  // normal file
		NULL);                  // no attr. template


	if (hFile == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();
		(void)err; // silent warning of unused variable
	}
}

void CLogger::log(const std::string& aMsg, int aType) {
	EnterCriticalSection(&CriticalSection);
	_tsbuff << info() << aMsg << std::endl;
	std::string s = _tsbuff.str();
	if (INVALID_HANDLE_VALUE != hFile) {
		DWORD dw = 0;
		::SetFilePointer(hFile, 0, nullptr, FILE_END);
		::WriteFile(hFile, s.c_str(), (int)s.size(), &dw, 0);
	}

	_tsbuff.str("");
	LeaveCriticalSection(&CriticalSection);
}

std::string CLogger::info() {
	SYSTEMTIME lt;
	GetLocalTime(&lt);

	std::ostringstream info;

	info << std::setw(2) << std::setfill('0') << lt.wHour << ":"
		<< std::setw(2) << std::setfill('0') << lt.wMinute << ":"
		<< std::setw(2) << std::setfill('0') << lt.wSecond << "."
		<< std::setw(3) << std::setfill('0') << lt.wMilliseconds << " "
		<< std::setw(5) << std::setfill(' ') << ::GetCurrentProcessId() << " "
		<< std::setw(5) << std::setfill(' ') << ::GetCurrentThreadId() << " ";

	std::string ts = info.str();
	info.str("");

	return ts;
}