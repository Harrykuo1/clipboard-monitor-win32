#define WINVER 0x0600
#define _WIN32_WINNT 0x0600     // Set Windows Vista as the minimum supported Windows version

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <regex>
#include <chrono>

// Define custom Windows messages and identifiers
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAYICON 1
#define ID_QUIT 2
#define ID_TOGGLE 3

NOTIFYICONDATA nid = {};
bool isModifying = true;
std::string lastContent;
std::chrono::steady_clock::time_point lastModificationTime;

// Structure to store clipboard content
struct ClipboardContent {
    bool isValid = false;
    std::string text;
    
    ClipboardContent() = default;
};

// Function to read clipboard content
ClipboardContent readClipboard() {
    ClipboardContent content;
    if (!OpenClipboard(NULL)) {
        std::cout << "Error: Unable to open clipboard" << std::endl;
        return content;
    }

    if (IsClipboardFormatAvailable(CF_TEXT)) {
        HANDLE hData = GetClipboardData(CF_TEXT);
        if (hData != NULL) {
            char* pszText = static_cast<char*>(GlobalLock(hData));
            if (pszText != NULL) {
                content.text = std::string(pszText);
                content.isValid = true;
                GlobalUnlock(hData);
            }
        }
    }

    CloseClipboard();
    return content;
}

// Function to write text to clipboard
void writeClipboard(const std::string& text) {
    if (!OpenClipboard(NULL)) {
        std::cout << "Error: Unable to open clipboard" << std::endl;
        return;
    }

    EmptyClipboard();

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.length() + 1);
    if (hMem != NULL) {
        memcpy(GlobalLock(hMem), text.c_str(), text.length() + 1);
        GlobalUnlock(hMem);
        SetClipboardData(CF_TEXT, hMem);
    }

    CloseClipboard();
}

// Function to modify clipboard content
void modifyClipboard() {
    if (!isModifying) return;
    
    ClipboardContent content = readClipboard();
    if (content.isValid && content.text != lastContent) {
        lastContent = content.text;
        std::string modifiedText = content.text;
        std::regex re("[\r\n\v\f\x85]+");
        modifiedText = std::regex_replace(modifiedText, re, " ");
        writeClipboard(modifiedText);
        std::cout << "Clipboard content modified" << std::endl;
        lastModificationTime = std::chrono::steady_clock::now();
    }
}

// Function to toggle modification mode
void toggleModifying() {
    isModifying = !isModifying;
    std::string status = isModifying ? "Modifying clipboard" : "Clipboard modification paused";
    strncpy(nid.szTip, status.c_str(), sizeof(nid.szTip) - 1);
    nid.szTip[sizeof(nid.szTip) - 1] = '\0';
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

// Window procedure function to handle various Windows messages
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
                AppendMenu(hMenu, MF_STRING, ID_TOGGLE, isModifying ? "Pause Modification" : "Resume Modification");
                AppendMenu(hMenu, MF_STRING, ID_QUIT, "Exit Program");
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

// Program entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char* CLASS_NAME = "ClipboardMonitorClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        CLASS_NAME, "Clipboard Monitor", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    // Initialize system tray icon
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = ID_TRAYICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    strncpy(nid.szTip, "Clipboard Monitor", sizeof(nid.szTip) - 1);
    nid.szTip[sizeof(nid.szTip) - 1] = '\0';
    Shell_NotifyIcon(NIM_ADD, &nid);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}