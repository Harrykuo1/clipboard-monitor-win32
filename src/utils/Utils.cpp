#include "Utils.h"
#include <windows.h>
#include <psapi.h>

std::wstring getActiveWindowProcessName() {
    HWND hwnd = GetForegroundWindow();
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);
    
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess == NULL) return L"";

    WCHAR szProcessName[MAX_PATH];
    if (GetModuleFileNameEx(hProcess, NULL, szProcessName, MAX_PATH) == 0) {
        CloseHandle(hProcess);
        return L"";
    }

    CloseHandle(hProcess);

    std::wstring processName = szProcessName;
    size_t pos = processName.find_last_of(L"\\/");
    return (pos != std::wstring::npos) ? processName.substr(pos + 1) : processName;
}