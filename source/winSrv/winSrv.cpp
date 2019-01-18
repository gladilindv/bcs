// winSrv.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"

#include "CWinSrv.h"

#include  <Windows.h>

int main()
{
	// launch server app
	CWinSrv srv;
	srv.start();

	// just for stop main loop events
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); 
	WaitForSingleObject(hEvent, INFINITE);

	return 0;
}