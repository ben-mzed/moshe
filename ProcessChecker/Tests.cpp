#include "Header.h"
#include <set>
#include <algorithm>
#include <iterator>
#ifdef _TESTS


bool IsAnyProcessRunning_UT(void)
{
	unordered_set<wstring> target_processes1 { L"proc-1" };
	unordered_set<wstring> target_processes2 { L"proc-4" };
	unordered_set<wstring> target_processes3 { };
	vector<wstring> running_processes { L"proc-1", L"proc-2", L"proc-3" };
	vector<wstring> running_processes_empty { };

	// "proc-1" is in both target_processes1 and running_processes
	// IsAnyProcessRunning should return true
	if (!IsAnyProcessRunning(target_processes1, running_processes)) {
		return false;
	}

	// "proc-4" is only in running_processes
	// IsAnyProcessRunning should return false
	if (IsAnyProcessRunning(target_processes2, running_processes)) {
		return false;
	}

	// target_processes3 is empty
	// IsAnyProcessRunning should return false
	if (IsAnyProcessRunning(target_processes3, running_processes)) {
		return false;
	}

	// running_processes_empty is empty
	// IsAnyProcessRunning should return false
	if (IsAnyProcessRunning(target_processes1, running_processes_empty)) {
		return false;
	}

	return true;
}


/*
	The following tests checks whether the number of processes returned from the function are not empty.
	And also that VsDebugConsole.exe is inside.
*/
bool GetProcessNamesByPsApi_UT() {
	wstring vs_process = L"VsDebugConsole.exe";
	vector<wstring> names_by_ps = GetProcessNamesByPsApi();
	if (!names_by_ps.size()) {
		return false;
	}
	return count(names_by_ps.begin(), names_by_ps.end(), vs_process) > 0;
}

bool GetProcessNamesByWtsApi_UT() {
	wstring vs_process = L"VsDebugConsole.exe";
	vector<wstring> names_by_wts = GetProcessNamesByWtsApi();
	return count(names_by_wts.begin(), names_by_wts.end(), vs_process) > 0;
}


int main(void)
{
	if (!IsAnyProcessRunning_UT())
	{
		printf("IsAnyProcessRunning_UT error");
		return 1;
	}

	if (!GetProcessNamesByPsApi_UT())
	{
		printf("GetProcessNamesByPsApi_UT error");
		return 1;
	}

	if (!GetProcessNamesByWtsApi_UT())
	{
		printf("GetProcessNamesByWtsApi_UT error");
		return 1;
	}


	return 0;
}
#endif