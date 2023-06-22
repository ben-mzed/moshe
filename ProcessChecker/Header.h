#pragma once
#pragma warning( push )
#pragma warning( disable : 4365)
#include <stdio.h>
#include <unordered_set>
#pragma warning( pop )
using namespace std;

/*
Checks whether one of the processes in the target_processes is in the vector of running_processes
*/
bool IsAnyProcessRunning(unordered_set<wstring> target_processes, vector<wstring> running_processes);

/*
Returns the names of all the processes currently running on the computer, by uses the 'Psapi' library.
*/
vector<wstring> GetProcessNamesByPsApi();

/*
Returns the names of all the processes currently running on the computer, by uses the 'WtsApi' library.
The implementation is performed by using the 'WTSEnumerateProcesses' function.
*/
vector<wstring> GetProcessNamesByWtsApi();