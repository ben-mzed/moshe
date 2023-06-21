#include "Header.h"
#include <windows.h>
#include <WtsApi32.h>
#include <Psapi.h>
#include <stdio.h>


// Uses 'EnumProcesses', 'OpenProcess', and 'GetModuleBaseName' functions from 'Psapi' library.
vector<wstring> GetProcessNamesByPsApi() {

    vector<wstring> processes_names;
    DWORD processes[1024];
    DWORD cb_needed;

    // Enumerate all processes into processes argument
    if (!EnumProcesses(processes, sizeof(processes), &cb_needed)) {
        printf("Failed to enumerate processes. Error code: %lu", GetLastError());
        return processes_names;
    }

    // Calculate the number of processes
    DWORD process_count = cb_needed / sizeof(DWORD);

    for (DWORD i = 0; i < process_count; ++i) {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
        if (hProcess != NULL) {
            wchar_t sz_process_name[MAX_PATH];
            if (GetModuleBaseNameW(hProcess, NULL, sz_process_name, sizeof(sz_process_name) / sizeof(wchar_t))) {
                wstring process_name(sz_process_name);
                processes_names.push_back(process_name);
            }
            CloseHandle(hProcess);
        }
    }

    return processes_names;
}


// Uses 'WTSEnumerateProcesses' function from 'WtsApi' library.
vector<wstring> GetProcessNamesByWtsApi() {

    WTS_PROCESS_INFO* pWPIs = NULL;
    DWORD dwProcCount = 0;
    vector<wstring> processes_names;

    if (WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, NULL, 1, &pWPIs, &dwProcCount))
    {
        //save all process names retrieved
        for (DWORD i = 0; i < dwProcCount; i++)
        {
            wstring process_name(pWPIs[i].pProcessName);
            processes_names.push_back(process_name);
        }
    }

    //Free memory
    if (pWPIs)
    {
        WTSFreeMemory(pWPIs);
        pWPIs = NULL;
    }
    return processes_names;

}


bool IsAnyProcessRunning(unordered_set<wstring> target_processes, vector<wstring> running_processes)
{
    for (const wstring& tProcess : running_processes) {
        if (target_processes.count(tProcess)) {
            return true;
        }
    }
    return false;
}

#ifndef _TESTS
int main(void)
{
    unordered_set<wstring> target_processes{ L"chrome.exe1", L"slack.exe1" , L"OneDrive.exe1" };
    vector<wstring> running_processes = GetProcessNamesByPsApi();
    if (IsAnyProcessRunning(target_processes, running_processes)) {
        printf("Yes");
    }
    else
    {
        printf("No");
    }
    return 0;
}
#endif