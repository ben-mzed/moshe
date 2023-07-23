#pragma once
#include <iostream>


/* @brief Service entry point
*
*/
VOID WINAPI ServiceMain(DWORD argc, LPWSTR* argv);


/* @brief Handler function for service control events
*
* @param control - service status
*/
VOID WINAPI Handler(DWORD control);


/* @brief get mouse speed
*
*/
int GetMouseSpeed();


/* @brief set mouse speed
*/
void SetMouseSpeed(int speed);


/* @brief install the service
*/
VOID SvcInstall();


/* @brief start the service
*/
VOID SvcStart();


/* @brief write to file
*/
void WriteToFile();


/* @brief change the mouse speed to min/max according to the corent speed.
*/
void AnnoyingMouseSpeed();
