#pragma once

#include "libsrv_global.h"

class CSocketServer;
class CSQLLite;

class LIBSRV_EXPORT CWinSrv
{
public:
	CWinSrv();
	~CWinSrv();

	void start();
	void stop();

private:
	void* mSrvThread;
	bool mIsStarted;
	unsigned int mSessionTimeLimit;

	static CSocketServer* mSrv;
	static CSQLLite* mSQL;

	static void log(const char*);

	static unsigned long __stdcall runThread(void*);
	static unsigned long __stdcall msgThread(void*);
};

