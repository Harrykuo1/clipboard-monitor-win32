#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#define WINVER 0x0600
#define _WIN32_WINNT 0x0600

#include <windows.h>
#include <shellapi.h>
#include <psapi.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <regex>
#include <chrono>
#include <fstream>
#include <sstream>
#include <codecvt>
#include <locale>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAYICON 1
#define ID_QUIT 2
#define ID_TOGGLE 3

bool isModifying = true;
std::wstring lastContent;
std::chrono::steady_clock::time_point lastModificationTime;
NOTIFYICONDATA nid = {};
std::vector<std::wstring> appWhitelist;

struct ClipboardContent {
    bool isValid = false;
    std::wstring text;
    
    ClipboardContent() = default;
};

void loadAppWhitelist() {
    std::ifstream file("whitelist.txt");
    std::string line;
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            appWhitelist.push_back(converter.from_bytes(line));
        }
    }
    if (appWhitelist.empty()) {
        // Default whitelist if file is empty or doesn't exist
        appWhitelist = {L"chrome.exe", L"firefox.exe", L"iexplore.exe", L"microsoftedge.exe", L"opera.exe", L"brave.exe"};
    }
}

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

bool isFromWhitelist(const std::wstring& processName) {
    return std::find(appWhitelist.begin(), appWhitelist.end(), processName) != appWhitelist.end();
}

ClipboardContent readClipboard() {
    ClipboardContent content;
    if (!OpenClipboard(NULL)) {
        std::wcout << L"Error: Unable to open clipboard" << std::endl;
        return content;
    }

    if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (hData != NULL) {
            wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hData));
            if (pszText != NULL) {
                content.text = std::wstring(pszText);
                content.isValid = true;
                GlobalUnlock(hData);
            }
        }
    }

    CloseClipboard();
    return content;
}

void writeClipboard(const std::wstring& text) {
    if (!OpenClipboard(NULL)) {
        std::wcout << L"Error: Unable to open clipboard" << std::endl;
        return;
    }
    EmptyClipboard();
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (text.length() + 1) * sizeof(wchar_t));
    if (hMem != NULL) {
        memcpy(GlobalLock(hMem), text.c_str(), (text.length() + 1) * sizeof(wchar_t));
        GlobalUnlock(hMem);
        SetClipboardData(CF_UNICODETEXT, hMem);
    }
    CloseClipboard();
}

void modifyClipboard() {
    if (!isModifying) return;
    
    ClipboardContent content = readClipboard();
    if (content.isValid && content.text != lastContent) {
        std::wstring activeProcessName = getActiveWindowProcessName();
        std::wcout << L"Active window process: " << activeProcessName << std::endl;
        
        if (isFromWhitelist(activeProcessName)) {
            lastContent = content.text;
            std::wstring modifiedText = content.text;
            std::wregex re(L"[\r\n\v\f\x85]+");
            modifiedText = std::regex_replace(modifiedText, re, L" ");
            writeClipboard(modifiedText);
            std::wcout << L"App clipboard content modified" << std::endl;
            lastModificationTime = std::chrono::steady_clock::now();
        } else {
            std::wcout << L"Clipboard content not modified (not from whitelisted)" << std::endl;
        }
    }
}

void toggleModifying() {
    isModifying = !isModifying;
    std::wstring status = isModifying ? L"Modifying clipboard" : L"Clipboard modification paused";
    wcsncpy_s(nid.szTip, status.c_str(), _TRUNCATE);
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            AddClipboardFormatListener(hwnd);
            break;
        case WM_CLIPBOARDUPDATE:
            modifyClipboard();
            break;
        case WM_DESTROY:
            RemoveClipboardFormatListener(hwnd);
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;
        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, ID_TOGGLE, isModifying ? L"Pause Modification" : L"Resume Modification");
                AppendMenu(hMenu, MF_STRING, ID_QUIT, L"Exit Program");
                SetForegroundWindow(hwnd);
                TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);
            }
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_QUIT:
                    DestroyWindow(hwnd);
                    break;
                case ID_TOGGLE:
                    toggleModifying();
                    break;
            }
            break;
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    loadAppWhitelist();

    const wchar_t* CLASS_NAME = L"ClipboardMonitorClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowW(
        CLASS_NAME, L"Clipboard Monitor", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = ID_TRAYICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcscpy_s(nid.szTip, L"Clipboard Monitor");
    Shell_NotifyIcon(NIM_ADD, &nid);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}