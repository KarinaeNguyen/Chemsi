#include <windows.h>
#include <stdio.h>
#include <string.h>

#define APP_NAME "VFEP Launcher"
#define VERSION "10.0.0"
#define IDI_ICON1 101

// Custom message box with icon from resources
int ShowMessageBox(HWND hwnd, const char* message, const char* title, UINT type) {
    MSGBOXPARAMSA params = {0};
    params.cbSize = sizeof(params);
    params.hwndOwner = hwnd;
    params.hInstance = GetModuleHandle(NULL);
    params.lpszText = message;
    params.lpszCaption = title;
    params.dwStyle = type | MB_USERICON;
    params.lpszIcon = MAKEINTRESOURCEA(IDI_ICON1);
    
    return MessageBoxIndirectA(&params);
}

// Function to execute PowerShell script silently
int ExecutePowerShellScript(const char* scriptPath, BOOL showWindow) {
    char cmdLine[2048];
    char workingDir[MAX_PATH];
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    
    // Get the directory where the executable is located
    GetModuleFileNameA(NULL, workingDir, MAX_PATH);
    char* lastSlash = strrchr(workingDir, '\\');
    if (lastSlash) *lastSlash = '\0';  // Remove filename, keep directory
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = showWindow ? SW_SHOW : SW_HIDE;
    
    ZeroMemory(&pi, sizeof(pi));
    
    // Build command line
    snprintf(cmdLine, sizeof(cmdLine),
        "powershell.exe -ExecutionPolicy Bypass -NoProfile -WindowStyle Hidden -File \"%s\"",
        scriptPath);
    
    // Create process with explicit working directory
    if (!CreateProcessA(
        NULL,           // Application name
        cmdLine,        // Command line
        NULL,           // Process attributes
        NULL,           // Thread attributes
        FALSE,          // Inherit handles
        CREATE_NO_WINDOW, // Creation flags - hide console
        NULL,           // Environment
        workingDir,     // Current directory - set to exe location
        &si,            // Startup info
        &pi))           // Process info
    {
        return -1;
    }
    
    // Wait for process to complete
    WaitForSingleObject(pi.hProcess, INFINITE);
    
    // Get exit code
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    
    // Clean up
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    return (int)exitCode;
}

// Function to show results dialog
void ShowResultDialog(const char* operation, int exitCode) {
    char message[512];
    
    if (exitCode == 0) {
        snprintf(message, sizeof(message), 
            "%s completed successfully!\n\nExit Code: 0", 
            operation);
        ShowMessageBox(NULL, message, APP_NAME " - Success", MB_OK);
    } else {
        snprintf(message, sizeof(message), 
            "%s failed.\n\nExit Code: %d\n\nCheck the logs for details.", 
            operation, exitCode);
        ShowMessageBox(NULL, message, APP_NAME " - Error", MB_OK);
    }
}

// Function to show menu and get choice
int ShowMenu() {
    const char* message = 
        "VFEP Launcher v" VERSION "\n\n"
        "Select an operation:\n\n"
        "1. Run Simulation (launch VFEP.exe)\n"
        "2. Build Project\n"
        "3. Run Tests\n"
        "4. Build + Test\n\n"
        "Click Yes to Run Simulation\n"
        "Click No for Build/Test options\n"
        "Click Cancel to exit";
    
    int result = ShowMessageBox(NULL, message, APP_NAME " v" VERSION, MB_YESNOCANCEL);
    
    if (result == IDYES) {
        return 4; // Run simulation
    } else if (result == IDNO) {
        // Show build/test options
        const char* message2 = 
            "Build & Test Options:\n\n"
            "Click Yes for Build only\n"
            "Click No for Tests only\n"
            "Click Cancel for Build + Test";
        
        int choice = ShowMessageBox(NULL, message2, APP_NAME, MB_YESNOCANCEL);
        
        if (choice == IDYES) return 1; // Build
        if (choice == IDNO) return 2;  // Test
        if (choice == IDCANCEL) return 3; // Build + Test
    }
    
    return 0; // Cancel
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) {
    int exitCode = 0;
    int operation = 0;
    
    // Parse command line arguments
    if (lpCmdLine != NULL && strlen(lpCmdLine) > 0) {
        // Remove quotes if present
        char* arg = lpCmdLine;
        if (arg[0] == '"') arg++;
        char* end = strchr(arg, '"');
        if (end) *end = '\0';
        
        // Trim whitespace
        while (*arg == ' ') arg++;
        
        if (_stricmp(arg, "run") == 0 || _stricmp(arg, "sim") == 0) {
            operation = 4;
        } else if (_stricmp(arg, "build") == 0) {
            operation = 1;
        } else if (_stricmp(arg, "test") == 0) {
            operation = 2;
        } else if (_stricmp(arg, "both") == 0 || _stricmp(arg, "all") == 0) {
            operation = 3;
        } else {
            char msg[512];
            snprintf(msg, sizeof(msg),
                "Invalid argument: %s\n\n"
                "Usage:\n"
                "  vfep_launcher.exe run    - Launch simulation\n"
                "  vfep_launcher.exe build  - Build project\n"
                "  vfep_launcher.exe test   - Run tests\n"
                "  vfep_launcher.exe both   - Build + Test\n"
                "  vfep_launcher.exe        - Show menu",
                arg);
            ShowMessageBox(NULL, msg, APP_NAME " - Error", MB_OK);
            return 1;
        }
    } else {
        // No arguments - show menu
        operation = ShowMenu();
        if (operation == 0) {
            return 0; // User cancelled
        }
    }
    
    // Execute operation
    if (operation == 4) {
        // Run simulation
        char exePath[MAX_PATH];
        char workingDir[MAX_PATH];
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        
        // Get exe directory
        GetModuleFileNameA(NULL, workingDir, MAX_PATH);
        char* lastSlash = strrchr(workingDir, '\\');
        if (lastSlash) *lastSlash = '\0';
        
        // Build path to VFEP.exe
        snprintf(exePath, sizeof(exePath), "%s\\build-mingw64\\VFEP.exe", workingDir);
        
        if (GetFileAttributesA(exePath) == INVALID_FILE_ATTRIBUTES) {
            char msg[512];
            snprintf(msg, sizeof(msg),
                "Simulation executable not found:\n\n%s\n\n"
                "Please build the project first.\n\n"
                "Run: vfep_launcher.exe build",
                exePath);
            ShowMessageBox(NULL, msg, APP_NAME " - Error", MB_OK);
            return 1;
        }
        
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        
        // Launch VFEP.exe without waiting
        if (!CreateProcessA(
            exePath,        // Application path
            NULL,           // Command line
            NULL,           // Process attributes
            NULL,           // Thread attributes
            FALSE,          // Inherit handles
            0,              // Creation flags
            NULL,           // Environment
            workingDir,     // Current directory
            &si,            // Startup info
            &pi))           // Process info
        {
            char msg[512];
            snprintf(msg, sizeof(msg),
                "Failed to launch simulation.\n\nError code: %lu",
                GetLastError());
            ShowMessageBox(NULL, msg, APP_NAME " - Error", MB_OK);
            return 1;
        }
        
        // Don't wait - let simulation run independently
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        
        return 0;
    }
    
    if (operation == 1 || operation == 3) {
        // Build
        ShowMessageBox(NULL, 
            "Building project...\n\nThis may take a moment. "
            "The process is running in the background.",
            APP_NAME, MB_OK);
        
        exitCode = ExecutePowerShellScript("fast_build.ps1", FALSE);
        ShowResultDialog("Build", exitCode);
        
        if (exitCode != 0 && operation == 3) {
            ShowMessageBox(NULL, 
                "Build failed. Skipping tests.",
                APP_NAME, MB_OK);
            return exitCode;
        }
    }
    
    if (operation == 2 || operation == 3) {
        // Test
        if (operation == 3) {
            // Small delay between build and test
            Sleep(500);
        }
        
        ShowMessageBox(NULL, 
            "Running tests...\n\nThis may take a moment. "
            "The process is running in the background.",
            APP_NAME, MB_OK);
        
        exitCode = ExecutePowerShellScript("fast_test.ps1", FALSE);
        ShowResultDialog("Tests", exitCode);
    }
    
    return exitCode;
}
