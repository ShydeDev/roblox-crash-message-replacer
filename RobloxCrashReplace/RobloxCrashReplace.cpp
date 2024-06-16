#include <iostream>
#include <chrono>
#include <Windows.h>
#include <thread>
#include <tlhelp32.h>

// tags
namespace tag {
    const char* success = "\033[1;32mSUCESS\033[0m";
    const char* restart = "\033[1;33mRESTART\033[0m";
    const char* crash = "\033[1;31mCRASH\033[0m";
    const char* error = "\033[1;31mERROR\033[0m";
}

std::string getUserProfile() {
    char userProfile[MAX_PATH];
    DWORD len = GetEnvironmentVariableA("USERPROFILE", userProfile, MAX_PATH);

    if (len == 0) 
        throw std::runtime_error("ERROR: Geting USERPROFILE environment variable. Error code: " + GetLastError());

    return userProfile;
}

DWORD getProcessId(const std::wstring& processName) {
    DWORD pid = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // get a snapshot of every open aplication

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // iterate though each process
    for (; Process32Next(hSnapshot, &pe32); ) {
        if (_wcsicmp(pe32.szExeFile, processName.c_str()) == 0) {
            pid = pe32.th32ProcessID;
            break;
        }
    }

    CloseHandle(hSnapshot);

    return pid;
}

int main()
{
    SetConsoleTitleA("RBX CRASH LOGGER");
    std::wstring processName = L"RobloxPlayerBeta.exe";

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // Get roblox crash window
        HWND roblox = FindWindowA(0, "Roblox Crash");
        DWORD robloxId = getProcessId(processName);

        if (roblox) {
            try {
                // Get roblox path
                std::string userProfile = getUserProfile();
                std::string path = userProfile + "\\Appdata\\Local\\Roblox\\Versions\\version-2cca5ed32b534b2a\\RobloxPlayerBeta.exe";

                // Terminate roblox process
                HANDLE robloxHandle = OpenProcess(PROCESS_TERMINATE | PROCESS_VM_READ, FALSE, robloxId);

                if (TerminateProcess(robloxHandle, 1)) {
                    std::wcout << tag::crash << ": Process Crashed. Terminated process: " << processName << " Process Id: " << robloxId << " prompting user...\n";
                }

                // Output message box and ask for restart
                uint8_t choice = MessageBoxA(0, "Whoops! Roblox Crashed! Would you like to restart?", "ROBLOX CRASHED",
                    MB_ICONERROR | MB_TOPMOST | MB_YESNO);

                if (choice == IDYES) {
                    // Restart roblox with error handling
                    HINSTANCE hInstance = ShellExecuteA(0, "open", path.c_str(), 0, 0, SW_SHOWNORMAL);

                    if (reinterpret_cast<int>(hInstance) <= 32) { // check if there is any errors
                        throw std::runtime_error("ERROR: Failed to launch RobloxPlayerBeta.exe. Erorr: " + reinterpret_cast<int>(hInstance));
                    }

                    // output information
                    std::cout << tag::success << ": Retrived USERPROFILE environment variable. Path: \"" << userProfile << "\"\n";
                    std::cout << tag::restart << ": Restarting the process \"RobloxPlayerBeta.exe\"...\n";

                    while (!FindWindowA(0, "Roblox")) { // Wait until roblox window with refresh rate of 1.2 seconds
                        Sleep(1200);
                    }

                    std::cout << tag::restart << ": Restarted the process \"RobloxPlayerBeta.exe\"!\n";
                }

            } catch (const std::exception& e) {
                system("cls");
                std::cerr << tag::error << ": " << e.what() << '\n';
            }
        }
    }
}
