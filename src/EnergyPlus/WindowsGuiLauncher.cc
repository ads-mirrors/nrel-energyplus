#include <windows.h>
#include <string>
#include <filesystem>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Get the path of the running executable
    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);

    // Extract the directory path
    std::filesystem::path exePath(path);
    std::filesystem::path toolPath = exePath.parent_path() / "energyplus.exe";

    // Construct the command
    std::string command = "\"" + toolPath.string() + "\" auxiliary ";
    if (lpCmdLine && *lpCmdLine) {  // If arguments exist, append them
        command += lpCmdLine;
    }

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (CreateProcess(NULL, command.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    return 0;
}
