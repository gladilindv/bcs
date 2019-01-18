// winSvc.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "CWinSvc.h"

int main(int argc, const char *argv[])
{
	CWinSvc svc;
	if (argc == 2)
	{
		if (strcmp(argv[1], "install") == 0)
			svc.Install();
		
		if (strcmp(argv[1], "remove") == 0)
			svc.Remove();
	}
	else
	{
		svc.Dispatch();
	}

	return 0;
}
