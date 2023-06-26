#include "Header.h"
#include <stdio.h>

#pragma warning( push )
#pragma warning( disable : 4820 4668 5039)
#include <windows.h>
#include <WtsApi32.h>
#pragma warning( pop )
#include <Psapi.h>
#pragma warning( disable : 5045)


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

#ifndef _TESTS
int main(void)
{
    const unordered_set<wstring> target_processes{ L"VsDebugConsol1e.exe", L"slack.exe1" , L"OneDrive.exe1" };
    vector<wstring> running_processes;
    if (!GetProcessNamesByWtsApi(running_processes)) {
        return 1;
    }

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