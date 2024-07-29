#define WINVER 0x0600
#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include "core/ClipboardMonitor.h"
#include "ui/TrayIcon.h"

#define WM_TRAYICON (WM_USER + 1)
#define ID_QUIT 2
#define ID_TOGGLE 3

ClipboardMonitor g_monitor;
TrayIcon* g_trayIcon = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            AddClipboardFormatListener(hwnd);
            g_trayIcon = new TrayIcon(hwnd);
            g_trayIcon->create();
            break;
        case WM_CLIPBOARDUPDATE:
            g_monitor.modifyClipboard();
            break;
        case WM_DESTROY:
            RemoveClipboardFormatListener(hwnd);
            if (g_trayIcon) {
                g_trayIcon->remove();
                delete g_trayIcon;
            }
            PostQuitMessage(0);
            break;
        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, ID_TOGGLE, g_monitor.isModifying() ? L"Pause Modification" : L"Resume Modification");
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
                    g_monitor.toggleModifying();
                    if (g_trayIcon) {
                        g_trayIcon->update(g_monitor.isModifying() ? L"Modifying clipboard" : L"Clipboard modification paused");
                    }
                    break;
            }
            break;
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
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

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}