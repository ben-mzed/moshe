#pragma warning( push )
#pragma warning( disable : 5045 4365 4668 4710 4777 5039)
#include <Windows.h>
#include "Header.h"
#include <tchar.h>
#include <strsafe.h>
#include <fstream>
#include <wtsapi32.h>
#include <processthreadsapi.h>
#pragma warning( pop )

//#pragma comment(lib, "advapi32.lib")

#pragma warning( disable : 5039 4312 4100)
// Global variables
SERVICE_STATUS serviceStatus = { 0 };
SERVICE_STATUS_HANDLE serviceStatusHandle = NULL;
int mouseSpeedCounter = 0;
const LPWSTR serviceName = const_cast<LPWSTR>(L"ABCD");
#define fileLocation TEXT("C:\\Users\\MosheRappaport\\Desktop\\test.txt")

// Save the original mouse speed
const int originalMouseSpeed = GetMouseSpeed();
const int fastMouseSpeed = 20;
const int slowMouseSpeed = 1;


int _tmain(DWORD argc, TCHAR* argv[])
{
    if (lstrcmpi(argv[1], TEXT("install")) == 0)
    {
        SvcInstall();
        SvcStart();
        return 0;
    }
    else if (lstrcmpi(argv[1], TEXT("AnnoyingService")) == 0)
    {
        
        if (GetMouseSpeed() == slowMouseSpeed)
        {
            SetMouseSpeed(fastMouseSpeed);
        }
        else
        {
            SetMouseSpeed(slowMouseSpeed);
        }
            
        Sleep(5000);
        
        return 0;
    }

    SERVICE_TABLE_ENTRY ServiceTable[] =
    {
        {serviceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
        {NULL, NULL}
    };

    if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
    {
        return (int)GetLastError();
    }
    
    return 0;
}


VOID WINAPI ServiceMain(DWORD argc, LPWSTR* argv)
{
    // Register the service control handler
    serviceStatusHandle = RegisterServiceCtrlHandlerW(argv[0], Handler);
    if (serviceStatusHandle == NULL)
    {
        return;
    }

    // Set service status to start pending
    serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    serviceStatus.dwCurrentState = SERVICE_RUNNING;
    serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    serviceStatus.dwWin32ExitCode = NO_ERROR;
    serviceStatus.dwServiceSpecificExitCode = 0;
    serviceStatus.dwCheckPoint = 0;
    serviceStatus.dwWaitHint = 0;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);

    DWORD sessionId = WTSGetActiveConsoleSessionId();
    HANDLE htoken;
    if (!WTSQueryUserToken(sessionId, &htoken)) {
        WriteToFile("WTSQueryUserToken");
    }

    TCHAR szUnquotedPath[MAX_PATH];
    if (!GetModuleFileName(NULL, szUnquotedPath, MAX_PATH))
    {
        WriteToFile("GetModuleFileName");
    }

    /* In case the path contains a space, it must be quoted so that
     it is correctly interpreted. For example,
     "d:\my share\myservice.exe" should be specified as
     ""d:\my share\myservice.exe"".*/
    TCHAR szPath[MAX_PATH];
    StringCbPrintf(szPath, MAX_PATH, TEXT("\"%s\""), szUnquotedPath);

    TCHAR appendString[] = _T(" AnnoyingMouse");
    _tcscat_s(szUnquotedPath, MAX_PATH, appendString);

    // Perform the main service function here
    while (serviceStatus.dwCurrentState == SERVICE_RUNNING)
    {
        STARTUPINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        bool success = CreateProcessAsUser(
            htoken,
            NULL,
            szUnquotedPath,
            NULL,
            NULL,
            false,
            0,
            NULL,
            NULL,
            &si,
            &pi
        );

        if (success) {
            // wait for the process to finish (optional)
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }

        // Sleep for 5 seconds
        Sleep(5000);
    }

    CloseHandle(htoken);


    // Set service status to stopped
    serviceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);
}


VOID WINAPI Handler(DWORD control)
{
    switch (control)
    {
    case SERVICE_CONTROL_STOP:
        // Set service status to stop pending
        serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(serviceStatusHandle, &serviceStatus);

        // Perform cleanup and stop service
        SetMouseSpeed(originalMouseSpeed);
        DeleteFile(fileLocation);

        // Set service status to stopped
        serviceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(serviceStatusHandle, &serviceStatus);

        break;
    }
}


void SetMouseSpeed(int speed)
{
    SystemParametersInfo(SPI_SETMOUSESPEED, 0, (LPVOID)speed, SPIF_SENDCHANGE);
}


int GetMouseSpeed()
{
    int speed = 0;
    SystemParametersInfo(SPI_GETMOUSESPEED, 0, (LPVOID)&speed, 0);
    return speed;
}


void WriteToFile(const char* msg)
{
    std::ofstream ofs(fileLocation, std::ofstream::app);

    ofs << msg << "\n";

    ofs.close();
}



VOID SvcInstall()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    TCHAR szUnquotedPath[MAX_PATH];

    if (!GetModuleFileName(NULL, szUnquotedPath, MAX_PATH))
    {
        return;
    }

    // In case the path contains a space, it must be quoted so that
    // it is correctly interpreted. For example,
    // "d:\my share\myservice.exe" should be specified as
    // ""d:\my share\myservice.exe"".
    TCHAR szPath[MAX_PATH];
    StringCbPrintf(szPath, MAX_PATH, TEXT("\"%s\""), szUnquotedPath);

    // Get a handle to the SCM database. 
    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (NULL == schSCManager)
    {
        return;
    }

    // Create the service
    schService = CreateService(
        schSCManager,              // SCM database 
        serviceName,                   // name of service 
        serviceName,                   // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_DEMAND_START, //SERVICE_AUTO_START,      // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        szPath,                    // path to service's binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL);                     // no password 

    if (schService == NULL)
    {
        CloseServiceHandle(schSCManager);
        return;
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}


VOID SvcStart()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    // Get a handle to the SCM database. 
    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // servicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (NULL == schSCManager)
    {
        return;
    }

    // Get a handle to the service.
    schService = OpenService(
        schSCManager,         // SCM database 
        serviceName,            // name of service 
        SERVICE_ALL_ACCESS);  // full access 

    if (schService == NULL)
    {
        CloseServiceHandle(schSCManager);
        return;
    }

    // Attempt to start the service.
    if (!StartService(
        schService,  // handle to service 
        0,           // number of arguments 
        NULL))      // no arguments 
    {
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return;
    }


    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}