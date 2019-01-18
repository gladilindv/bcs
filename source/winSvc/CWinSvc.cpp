#include "pch.h"
#include "CWinSvc.h"

#include <strsafe.h>

#include "CWinSrv.h"

#define SVCNAME TEXT("BCS_pbuf_svc")
#define SVCDISP TEXT("BCS Protobuf Service")
#define SVCDESC TEXT("BCS Protocol Buffer Service (Gladilin test job)")

SERVICE_STATUS          gSvcStatus;
SERVICE_STATUS_HANDLE   gSvcStatusHandle;
HANDLE                  ghSvcStopEvent = NULL;

void WINAPI SvcMain(DWORD, LPWSTR *);
void WINAPI SvcCtrlHandler(DWORD);
void ChangeSvcStatus(DWORD, DWORD, DWORD);
void WriteEventLogEntry(LPCWSTR, WORD);
void WriteErrorLogEvent(LPCWSTR);


void CWinSvc::Install() {
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	wchar_t szPath[MAX_PATH];

	printf("try to install\n");

	if (!GetModuleFileName(NULL, szPath, MAX_PATH))
	{
		printf("Cannot install service (%d)\n", GetLastError());
		return;
	}

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	// Create the service

	schService = CreateService(
		schSCManager,              // SCM database 
		SVCNAME,                   // name of service 
		SVCDISP,				   // service name to display 
		SERVICE_ALL_ACCESS,        // desired access 
		SERVICE_WIN32_OWN_PROCESS, // service type 
		SERVICE_DEMAND_START,      // start type 
		SERVICE_ERROR_NORMAL,      // error control type 
		szPath,                    // path to service's binary 
		NULL,                      // no load ordering group 
		NULL,                      // no tag identifier 
		NULL,                      // no dependencies 
		NULL,                      // LocalSystem account 
		NULL);                     // no password 

	if (schService == NULL)
	{
		printf("CreateService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}
	else printf("Service installed successfully\n");

	SERVICE_DESCRIPTION sd;
	sd.lpDescription = (LPWSTR)SVCDESC;
	if (ChangeServiceConfig2(schService,
		SERVICE_CONFIG_DESCRIPTION,
		&sd) == FALSE)
	{
		printf("ChangeServiceConfig2 failed (%d)\n", GetLastError());
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

void CWinSvc::Remove() {
	SC_HANDLE schSCManager;
	SC_HANDLE schService;

	printf("try to delete\n");

	// Get a handle to the SCM database.
	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	// Get a handle to the service.

	schService = OpenService(
		schSCManager,       // SCM database 
		SVCNAME,			// name of service 
		DELETE);            // need delete access 

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}

	// Delete the service.
	if (!DeleteService(schService))
	{
		printf("DeleteService failed (%d)\n", GetLastError());
	}
	else printf("Service deleted successfully\n");

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

void CWinSvc::Dispatch() {
	// TO_DO: Add any additional services for the process to this table.
	SERVICE_TABLE_ENTRY DispatchTable[] =
	{
		{ (LPWSTR)SVCNAME, (LPSERVICE_MAIN_FUNCTION)SvcMain },
		{ NULL, NULL }
	};

	// This call returns when the service has stopped. 
	// The process should simply terminate when the call returns.

	if (!StartServiceCtrlDispatcher(DispatchTable))
	{
		WriteErrorLogEvent(TEXT("StartServiceCtrlDispatcher"));
	}
}


//   Entry point for the service
void WINAPI SvcMain(DWORD dwArgc, LPWSTR *lpszArgv) {

	// Register the handler function for the service
	gSvcStatusHandle = RegisterServiceCtrlHandler(SVCNAME, SvcCtrlHandler);
	if (!gSvcStatusHandle) {
		WriteErrorLogEvent(TEXT("RegisterServiceCtrlHandler"));
		return;
	}

	gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	gSvcStatus.dwServiceSpecificExitCode = 0;

	// Report initial status to the SCM
	ChangeSvcStatus(SERVICE_START_PENDING, NO_ERROR, 1000);

	//SvcInit(dwArgc, lpszArgv);
	ghSvcStopEvent = CreateEvent(
		NULL,    // default security attributes
		TRUE,    // manual reset event
		FALSE,   // not signaled
		NULL);   // no name

	if (ghSvcStopEvent == NULL)	{
		ChangeSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}
	
	// init and run network server thread
	CWinSrv srv;
	srv.start();

	// Report running status when initialization is complete.
	ChangeSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

	// TO_DO: Perform work until service stops.
	while (1) {
		// Check whether to stop the service.
		WaitForSingleObject(ghSvcStopEvent, INFINITE);

		srv.stop();

		ChangeSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}
}


void ChangeSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint) {
	static DWORD dwCheckPoint = 1;

	// Fill in the SERVICE_STATUS structure.
	gSvcStatus.dwCurrentState = dwCurrentState;
	gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
	gSvcStatus.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_START_PENDING)
		gSvcStatus.dwControlsAccepted = 0;
	else 
		gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	if ((dwCurrentState == SERVICE_RUNNING) ||
		(dwCurrentState == SERVICE_STOPPED))
		gSvcStatus.dwCheckPoint = 0;
	else 
		gSvcStatus.dwCheckPoint = dwCheckPoint++;

	// Report the status of the service to the SCM.
	SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}

void WINAPI SvcCtrlHandler(DWORD dwCtrl) {
	// Handle the requested control code. 

	switch (dwCtrl)
	{
	case SERVICE_CONTROL_STOP:
		ChangeSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

		// Signal the service to stop.
		SetEvent(ghSvcStopEvent);
		ChangeSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);

		return;

	case SERVICE_CONTROL_INTERROGATE:
		break;

	default:
		break;
	}
}

void WriteEventLogEntry(LPCWSTR pszMessage, WORD wType = EVENTLOG_ERROR_TYPE)
{
	LPCWSTR lpszStrings[2] = { NULL, NULL };
	HANDLE hEventSource = RegisterEventSource(NULL, SVCNAME);

	if (NULL != hEventSource)
	{
		lpszStrings[0] = SVCNAME;
		lpszStrings[1] = pszMessage;

		ReportEvent(hEventSource,        // event log handle
			wType,				 // event type
			0,                   // event category
			0,					 // event identifier
			NULL,                // no security identifier
			2,                   // size of lpszStrings array
			0,                   // no binary data
			lpszStrings,         // array of strings
			NULL);               // no binary data

		DeregisterEventSource(hEventSource);
	}
}


void WriteErrorLogEvent(LPCWSTR szFunction) {
	wchar_t Buffer[260];

	StringCchPrintf(Buffer, ARRAYSIZE(Buffer), L"%s failed w/err 0x%08lx", szFunction, GetLastError());
	WriteEventLogEntry(Buffer, EVENTLOG_ERROR_TYPE);
}