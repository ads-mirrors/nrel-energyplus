#include <string>
#include <windows.h>

int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // Get the path of the current executable
    wchar_t path[MAX_PATH];
    if (GetModuleFileNameW(NULL, path, MAX_PATH) == 0) {
        MessageBoxW(NULL, L"Failed to get module file name!", L"Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    // Remove the executable name to get the directory
    std::wstring directory(path);
    size_t pos = directory.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        directory = directory.substr(0, pos); // Extract just the directory path
    }

    // Construct the target application path (assuming it's in the same directory)
    std::wstring targetApp = directory + L"\\energyplus.exe";

    // Check if the file exists
    if (GetFileAttributesW(targetApp.c_str()) == INVALID_FILE_ATTRIBUTES) {
        MessageBoxW(NULL, L"Application not found!", L"Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    // Create the command line arguments
    std::wstring args = L"auxiliary ";
    args += lpCmdLine;

    // Launch the target application
    HINSTANCE hInstance2 = ShellExecuteW(NULL,              // Parent window
                                         L"open",           // Operation
                                         targetApp.c_str(), // Executable path
                                         args.c_str(),      // Arguments
                                         directory.c_str(), // Working directory
                                         SW_HIDE            // Hide console
    );

    // Error handling
    if ((intptr_t)hInstance2 <= 32) {
        std::wstring errorMsg = L"Failed to launch application! Error code: " + std::to_wstring((intptr_t)hInstance2);
        MessageBoxW(NULL, errorMsg.c_str(), L"Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    return 0;
}
