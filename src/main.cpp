#define WINVER 0x0600
#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include "core/ClipboardMonitor.h"
#include "ui/MainWindow.h"
#include "utils/DebugMacros.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        DEBUG_LOG(L"Failed to initialize COM library");
        return 1;
    }

    ClipboardMonitor monitor;
    MainWindow mainWindow(hInstance, monitor);

    if (!mainWindow.Create()) {
        DEBUG_LOG(L"Failed to create main window");
        CoUninitialize();
        return 1;
    }

    mainWindow.Show(nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CoUninitialize();
    return 0;
}