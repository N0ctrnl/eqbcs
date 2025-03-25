// Windows Service functions
#include "EQBCS.h"

#ifdef UNIXWIN
SERVICE_STATUS m_ServiceStatus;
SERVICE_STATUS_HANDLE m_ServiceStatusHandle;
BOOL bRunning=true;

void WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
  m_ServiceStatus.dwServiceType = SERVICE_WIN32;
  m_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
  m_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
  m_ServiceStatus.dwWin32ExitCode = 0;
  m_ServiceStatus.dwServiceSpecificExitCode = 0;
  m_ServiceStatus.dwCheckPoint = 0;
  m_ServiceStatus.dwWaitHint = 0;

  m_ServiceStatusHandle = RegisterServiceCtrlHandler(TEXT(PROG_NAME), ServiceCtrlHandler); 
  if (m_ServiceStatusHandle == (SERVICE_STATUS_HANDLE)0)
  {
    return;
  }
  m_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
  m_ServiceStatus.dwCheckPoint = 0;
  m_ServiceStatus.dwWaitHint = 0;
  if (!SetServiceStatus (m_ServiceStatusHandle, &m_ServiceStatus))
  {
  }

  bRunning=true;
  while(bRunning)
  {
    bcs.processMain(1);
  }
  return;
}

void WINAPI ServiceCtrlHandler(DWORD Opcode)
{
  switch(Opcode)
  {
    case SERVICE_CONTROL_PAUSE: 
      m_ServiceStatus.dwCurrentState = SERVICE_PAUSED;
      break;
    case SERVICE_CONTROL_CONTINUE:
      m_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
      break;
    case SERVICE_CONTROL_STOP:
      m_ServiceStatus.dwWin32ExitCode = 0;
      m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
      m_ServiceStatus.dwCheckPoint = 0;
      m_ServiceStatus.dwWaitHint = 0;

      SetServiceStatus (m_ServiceStatusHandle,&m_ServiceStatus);
      bRunning=false;
      break;
    case SERVICE_CONTROL_INTERROGATE:
      break; 
  }
  return;
}

BOOL DoInstallService(char* lpServiceName, char* lpDisplayName, int c, char* v[])
{
  char strPath[MAX_PATH];
  SC_HANDLE hSCManager,hService;
	int i;

  if(!GetModuleFileName(NULL, strPath, MAX_PATH))
  {
	  printf("Cannot install service (%d)\n", GetLastError());
	  return false;
  }

	// ** No error control to make sure path ends up being valid after command line is appended
	strncat(strPath," -SERVICE ",min(10,MAX_PATH-strlen(strPath)-1));
	for (i=1; i<c; i++) {
		// Strip -c (create service) from the command line of installed service.
		if (strncmp(v[i],"-c",2)==0) {
			continue;
		}
		else {
			strncat(strPath,v[i],min(strlen(v[i]),MAX_PATH-strlen(strPath)-1));
			if (strlen(v[i])<MAX_PATH-1)
				strncat(strPath," ",1);
			else {
				printf("Service command line too long.");
				return false;
			}
		}
	}

	// Get a handle to the SCM database. 
  hSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);

  if (hSCManager == NULL) 
	{
		printf("Create: OpenSCManager failed (%d)\n", GetLastError());
		return false;
	}
  
  hService = CreateService(hSCManager,lpServiceName, 
     lpDisplayName, // service name to display
     SERVICE_ALL_ACCESS, // desired access 
     SERVICE_WIN32_OWN_PROCESS, // service type 
     SERVICE_AUTO_START, // start type 
     SERVICE_ERROR_NORMAL, // error control type 
     strPath, // service's binary 
     NULL, // no load ordering group 
     NULL, // no tag identifier 
     NULL, // no dependencies
     NULL, // LocalSystem account
     NULL); // no password


  if (hService == NULL)
	{
		printf("CreateService failed (%d)\n", GetLastError()); 
		CloseServiceHandle(hSCManager);
		return false;
	}

  CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
  return true;
}

BOOL DoDeleteService(char* lpServiceName)
{
  SERVICE_STATUS_PROCESS ssp;
  DWORD BytesNeeded;
	DWORD StartTime=GetTickCount();
	DWORD Timeout = 30000; // 30-second time-out
  SC_HANDLE hSCManager, hService;
  
  // Get a handle to the SCM database
  hSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);

  if (hSCManager == NULL)
    return false;
  
  // Get a handle to the service
  hService=OpenService(hSCManager,lpServiceName,SERVICE_ALL_ACCESS);
	if (hService == NULL) {
		printf("Delete: OpenService failed (%d)\n", GetLastError()); 
		CloseServiceHandle(hSCManager);
    return false;
	}
  
  // Get the current status of the service
	if ( !QueryServiceStatusEx(hService,SC_STATUS_PROCESS_INFO,(LPBYTE) &ssp,sizeof(SERVICE_STATUS_PROCESS),&BytesNeeded) ) {
		printf("Delete: QueryServiceStatusEx failed (%d)\n", GetLastError());
		goto DeleteService_cleanup;
	}

	if (ssp.dwCurrentState == SERVICE_RUNNING) 
	{
    if (!DoStopService(TEXT(PROG_NAME)))
			goto DeleteService_cleanup;
	}

	if(!DeleteService(hService))
	{
		printf("DeleteService failed (%d)\n", GetLastError()); 
    goto DeleteService_cleanup;
	}
  if(!CloseServiceHandle(hService) || !CloseServiceHandle(hSCManager))
    return false;
	return true;

DeleteService_cleanup:
  CloseServiceHandle(hService);
  CloseServiceHandle(hSCManager);
  return false;
}

BOOL DoStopService(char* lpServiceName)
{
  SERVICE_STATUS_PROCESS ssp;
  DWORD BytesNeeded;
	DWORD StartTime=GetTickCount();
	DWORD Timeout = 30000; // 30-second time-out
  SC_HANDLE hSCManager, hService;
  
  // Get a handle to the SCM database
  hSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);

  if (hSCManager == NULL)
    return false;
  
  // Get a handle to the service
  hService=OpenService(hSCManager,lpServiceName,SERVICE_ALL_ACCESS);
	if (hService == NULL) {
		printf("Stop: OpenService failed (%d)\n", GetLastError()); 
		CloseServiceHandle(hSCManager);
    return false;
	}
  
  // Get the current status of the service
	if ( !QueryServiceStatusEx(hService,SC_STATUS_PROCESS_INFO,(LPBYTE) &ssp,sizeof(SERVICE_STATUS_PROCESS),&BytesNeeded) ) {
		printf("Stop: QueryServiceStatusEx failed (%d)\n", GetLastError());
		goto StopService_cleanup;
	}

	if (ssp.dwCurrentState == SERVICE_RUNNING) 
	{
    // Send a stop code to the service.
		if (!ControlService(hService,SERVICE_CONTROL_STOP,(LPSERVICE_STATUS) &ssp) ) 
		{
			printf ("Stop: ControlService failed (%d)\n", GetLastError() );
			goto StopService_cleanup;
		}
		
		// Wait for the service to stop.
		while ( ssp.dwCurrentState != SERVICE_STOPPED ) 
    {
			Sleep( ssp.dwWaitHint );
      if ( !QueryServiceStatusEx(hService,SC_STATUS_PROCESS_INFO,(LPBYTE)&ssp,sizeof(SERVICE_STATUS_PROCESS),&BytesNeeded ) ) {
				printf( "Stop: QueryServiceStatusEx failed (%d)\n", GetLastError() );
        goto StopService_cleanup;
			}

      if ( ssp.dwCurrentState == SERVICE_STOPPED )
				break;
			
			if ( GetTickCount() - StartTime > Timeout ) {
				printf( "Stop: Wait timed out\n" );
				goto StopService_cleanup;
			}
		}
	}

  if(!CloseServiceHandle(hService) || !CloseServiceHandle(hSCManager))
    return false;
	return true;

StopService_cleanup:
  CloseServiceHandle(hService);
  CloseServiceHandle(hSCManager);
  return false;
}

BOOL DoStartService(char* lpServiceName)
{
	SERVICE_STATUS_PROCESS ssp; 
  DWORD OldCheckPoint, StartTickCount, WaitTime, BytesNeeded;
	SC_HANDLE hSCManager, hService;

	// Get a handle to the SCM database. 
	hSCManager = OpenSCManager( 
		NULL,                    // local computer
		NULL,                    // servicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 
	
	if (NULL == hSCManager) 
	{
		printf("Start: OpenSCManager failed (%d)\n", GetLastError());
		return false;
	}
	
	// Get a handle to the service.
	hService = OpenService(hSCManager,lpServiceName,SERVICE_ALL_ACCESS);
	
	if (hService==NULL)
	{ 
		printf("Start: OpenService failed (%d)\n", GetLastError()); 
		CloseServiceHandle(hSCManager);
		return false;
	}
	
	// Check the status in case the service is not stopped. 
	if (!QueryServiceStatusEx( 
		hService,                     // handle to service 
		SC_STATUS_PROCESS_INFO,         // information level
		(LPBYTE) &ssp,                  // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&BytesNeeded ) )                // size needed if buffer is too small
	{
		printf("Start: QueryServiceStatusEx failed (%d)\n", GetLastError());
		goto StartService_cleanup;
	}
	
	// Check if the service is already running. It would be possible 
	// to stop the service here, but for simplicity we just return. 
	if(ssp.dwCurrentState != SERVICE_STOPPED && ssp.dwCurrentState != SERVICE_STOP_PENDING)
	{
		printf("Start: Cannot start the service because it is already running\n");
		goto StartService_cleanup;
	}
	
	// Save the tick count and initial checkpoint.
	StartTickCount = GetTickCount();
	OldCheckPoint = ssp.dwCheckPoint;
	
	// Wait for the service to stop before attempting to start it.
	while (ssp.dwCurrentState == SERVICE_STOP_PENDING)
	{
		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth of the wait hint but not less than 1 second  
		// and not more than 10 seconds. 
		WaitTime = ssp.dwWaitHint / 10;
		if( WaitTime < 1000 )
			WaitTime = 1000;
		else if ( WaitTime > 10000 )
			WaitTime = 10000;
		Sleep( WaitTime );
		
		// Check the status until the service is no longer stop pending. 
		if (!QueryServiceStatusEx( 
			hService,                       // handle to service 
			SC_STATUS_PROCESS_INFO,         // information level
			(LPBYTE) &ssp,                  // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&BytesNeeded ) )                // size needed if buffer is too small
		{
			printf("Start: QueryServiceStatusEx failed (%d)\n", GetLastError());
			goto StartService_cleanup;
		}
		
		if ( ssp.dwCheckPoint > OldCheckPoint )
		{
			// Continue to wait and check.
			StartTickCount = GetTickCount();
			OldCheckPoint = ssp.dwCheckPoint;
		}
		else
		{
			if(GetTickCount()-StartTickCount > ssp.dwWaitHint)
			{
				printf("Start: Timeout waiting for service to stop\n");
				goto StartService_cleanup;
			}
		}
	}
	
	// Attempt to start the service.
	if (!StartService(
		hService,    // handle to service 
		0,           // number of arguments 
		NULL) )      // no arguments 
	{
		printf("StartService failed (%d)\n", GetLastError());
		goto StartService_cleanup;
	}
	else printf("Service start pending...\n"); 
	
	// Check the status until the service is no longer start pending. 
	if (!QueryServiceStatusEx( 
		hService,                       // handle to service 
		SC_STATUS_PROCESS_INFO,         // info level
		(LPBYTE) &ssp,                  // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&BytesNeeded ) )                // if buffer too small
	{
		printf("Start: QueryServiceStatusEx failed (%d)\n", GetLastError());
		goto StartService_cleanup;
	}
	
	// Save the tick count and initial checkpoint.
	StartTickCount = GetTickCount();
	OldCheckPoint = ssp.dwCheckPoint;
	
	while (ssp.dwCurrentState == SERVICE_START_PENDING) 
	{ 
		WaitTime = ssp.dwWaitHint / 10;
		
		if(WaitTime<1000)
			WaitTime=1000;
		else if (WaitTime>10000)
			WaitTime=10000;
		
		Sleep(WaitTime);
		
		// Check the status again. 
		if (!QueryServiceStatusEx( 
			hService, SC_STATUS_PROCESS_INFO,(LPBYTE) &ssp,
			sizeof(SERVICE_STATUS_PROCESS),&BytesNeeded))
		{
			printf("Start: QueryServiceStatusEx failed (%d)\n", GetLastError());
			break; 
		}
		
		if (ssp.dwCheckPoint>OldCheckPoint)
		{
			// Continue to wait and check.
			StartTickCount = GetTickCount();
			OldCheckPoint = ssp.dwCheckPoint;
		}
		else
		{
			if(GetTickCount()-StartTickCount > ssp.dwWaitHint)
			{
				// No progress made within the wait hint.
				break;
			}
		}
	} 
	
	// Determine whether the service is running.
	if (ssp.dwCurrentState==SERVICE_RUNNING) 
	{
		printf("Service started successfully.\n"); 
	}
	else 
	{
		printf("Service not started. \n");
		printf("  Current State: %d\n", ssp.dwCurrentState); 
		printf("  Exit Code: %d\n", ssp.dwWin32ExitCode); 
		printf("  Check Point: %d\n", ssp.dwCheckPoint); 
		printf("  Wait Hint: %d\n", ssp.dwWaitHint); 
		goto StartService_cleanup;
	} 

    CloseServiceHandle(hService); 
    CloseServiceHandle(hSCManager);
		return true;

StartService_cleanup:
		CloseServiceHandle(hService);
		CloseServiceHandle(hSCManager);
		return false;
}
#endif
// End Windows Service functions
