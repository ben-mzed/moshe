#include <iostream>
#include <Windows.h>
#include "Header.h"
#include <tchar.h>
#include <strsafe.h>

// Global variables
SERVICE_STATUS serviceStatus = { 0 };
SERVICE_STATUS_HANDLE serviceStatusHandle = NULL;
int mouseSpeedCounter = 0;

// Save the original mouse speed
const int originalMouseSpeed = GetMouseSpeed();
const int fastMouseSpeed = 20;
const int slowMouseSpeed = 1;
const LPWSTR serviceName = const_cast<LPWSTR>(L"ABCD");
//#define serviceName TEXT("SvcName")
int main(int argc, TCHAR* argv[])
{   
    //// If command-line parameter is "install", install the service. 
    //// Otherwise, the service is probably being started by the SCM.
    //if (lstrcmpi(argv[1], TEXT("install")) == 0)
    //{
    //    return 0;
    //}
    SvcInstall();

    // Service table entry structure
    SERVICE_TABLE_ENTRY serviceTable[] =
    {
        { serviceName, (LPSERVICE_MAIN_FUNCTION) ServiceMain },
        { NULL, NULL }
    };

    // Register the service control handler
    if (!StartServiceCtrlDispatcher(serviceTable))
    {
        int err = GetLastError();
        return err;
    }

    return 0;
}

VOID WINAPI ServiceMain(DWORD argc, LPWSTR* argv)
{
    // Register the service control handler
    serviceStatusHandle = RegisterServiceCtrlHandlerW(argv[0], Handler);
    if (serviceStatusHandle == NULL)
    {
        std::cerr << "Failed to register service control handler." << std::endl;
        return;
    }

    // Set service status to start pending
    serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    serviceStatus.dwCurrentState = SERVICE_START_PENDING;
    serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    serviceStatus.dwWin32ExitCode = NO_ERROR;
    serviceStatus.dwServiceSpecificExitCode = 0;
    serviceStatus.dwCheckPoint = 0;
    serviceStatus.dwWaitHint = 0;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);

    // Perform initialization tasks here


    // Set service status to running
    serviceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);

    // Perform the main service function here
    while (serviceStatus.dwCurrentState == SERVICE_RUNNING)
    {
        // Change the mouse speed every 5 seconds
        if (mouseSpeedCounter % 2 == 0)
        {
            SetMouseSpeed(fastMouseSpeed);
        }
        else
        {
            SetMouseSpeed(slowMouseSpeed);
        }

        ++mouseSpeedCounter;

        // Sleep for 5 seconds
        Sleep(5000);
    }

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

//
// Purpose: 
//   Installs a service in the SCM database
//
// Parameters:
//   None
// 
// Return value:
//   None
//
VOID SvcInstall()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    TCHAR szUnquotedPath[MAX_PATH];

    if (!GetModuleFileName(NULL, szUnquotedPath, MAX_PATH))
    {
        printf("Cannot install service (%d)\n", GetLastError());
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
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Create the service

    schService = CreateService(
        schSCManager,              // SCM database 
        serviceName,                   // name of service 
        serviceName,                   // service name to display 
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

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}
