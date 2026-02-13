// VFEP Fast Test Launcher
// Compiles to fast_test.exe with icon

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Get current directory
    char currentDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, currentDir);
    
    // Build PowerShell command
    char command[4096];
    snprintf(command, sizeof(command), 
        "powershell.exe -ExecutionPolicy Bypass -NoProfile -File \"%s\\fast_test.ps1\"",
        currentDir);
    
    // Show console window
    AllocConsole();
    
    // Execute
    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;
    
    if (CreateProcessA(NULL, command, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, currentDir, &si, &pi))
    {
        WaitForSingleObject(pi.hProcess, INFINITE);
        
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        
        return exitCode;
    }
    else
    {
        MessageBoxA(NULL, "Failed to launch fast_test.ps1", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
}
