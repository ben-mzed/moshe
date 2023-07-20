#pragma once
#include <iostream>
// Service entry point
VOID WINAPI ServiceMain(DWORD argc, LPWSTR* argv);

// Handler function for service control events
VOID WINAPI Handler(DWORD control);

int GetMouseSpeed();

void SetMouseSpeed(int speed);
VOID SvcInstall();