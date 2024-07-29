#include "MainWindow.h"
#include "../utils/DebugMacros.h"
#include <commctrl.h>
#include <windowsx.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <string>
#include <commdlg.h>

#pragma comment(lib, "uxtheme.lib")

#define ID_TOGGLE_BUTTON 1001
#define ID_WHITELIST_BOX 1002
#define ID_ADD_BUTTON 1003
#define ID_REMOVE_BUTTON 1004
#define ID_STATUS_BAR 1005

MainWindow::MainWindow(HINSTANCE hInstance, ClipboardMonitor& monitor)
    : m_hwnd(NULL), m_hInstance(hInstance), m_monitor(monitor) {
    LoadWhitelist();
    DEBUG_LOG(L"MainWindow constructor called");
}

MainWindow::~MainWindow() {
    SaveWhitelist();
    DEBUG_LOG(L"MainWindow destructor called");
}

bool MainWindow::Create() {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = L"ClipboardMonitorWindowClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    m_hwnd = CreateWindowEx(
        0, L"ClipboardMonitorWindowClass", L"Clipboard Monitor",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 500,
        NULL, NULL, m_hInstance, this
    );

    if (m_hwnd == NULL) {
        DEBUG_LOG(L"Failed to create window. Error code: " << GetLastError());
        return false;
    }

    CreateControls();
    return true;
}

void MainWindow::Show(int nCmdShow) {
    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = NULL;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (MainWindow*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    } else {
        pThis = (MainWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (pThis) {
        switch (uMsg) {
        case WM_CREATE:
            DEBUG_LOG(L"WM_CREATE received");
            if (!AddClipboardFormatListener(hwnd)) {
                DEBUG_LOG(L"Failed to add clipboard format listener");
            }
            return 0;
        case WM_CLIPBOARDUPDATE:
            DEBUG_LOG(L"WM_CLIPBOARDUPDATE received");
            pThis->m_monitor.modifyClipboard();
            return 0;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
            case ID_TOGGLE_BUTTON:
                DEBUG_LOG(L"Toggle button clicked");
                pThis->ToggleMonitoring();
                return 0;
            case ID_ADD_BUTTON:
                DEBUG_LOG(L"Add button clicked");
                pThis->AddToWhitelist();
                return 0;
            case ID_REMOVE_BUTTON:
                DEBUG_LOG(L"Remove button clicked");
                pThis->RemoveFromWhitelist();
                return 0;
            }
            break;
        case WM_SIZE:
            pThis->ResizeControls();
            return 0;
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN:
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (LRESULT)GetStockObject(NULL_BRUSH);
        case WM_CLOSE:
            DEBUG_LOG(L"WM_CLOSE received");
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            DEBUG_LOG(L"WM_DESTROY received");
            RemoveClipboardFormatListener(hwnd);
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void MainWindow::CreateControls() {
    HTHEME hTheme = OpenThemeData(m_hwnd, L"BUTTON");
    
    m_hToggleButton = CreateWindow(
        L"BUTTON", L"Pause Monitoring",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        10, 10, 180, 30, m_hwnd, (HMENU)ID_TOGGLE_BUTTON, m_hInstance, NULL
    );

    m_hWhitelistBox = CreateWindow(
        L"LISTBOX", NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
        10, 50, 360, 350, m_hwnd, (HMENU)ID_WHITELIST_BOX, m_hInstance, NULL
    );

    m_hAddButton = CreateWindow(
        L"BUTTON", L"Add Program",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        10, 410, 120, 30, m_hwnd, (HMENU)ID_ADD_BUTTON, m_hInstance, NULL
    );

    m_hRemoveButton = CreateWindow(
        L"BUTTON", L"Remove Selected",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        140, 410, 120, 30, m_hwnd, (HMENU)ID_REMOVE_BUTTON, m_hInstance, NULL
    );

    m_hStatusBar = CreateWindow(
        STATUSCLASSNAME, NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, m_hwnd, (HMENU)ID_STATUS_BAR, m_hInstance, NULL
    );

    if (!m_hToggleButton || !m_hWhitelistBox || !m_hAddButton || !m_hRemoveButton || !m_hStatusBar) {
        DEBUG_LOG(L"Failed to create one or more controls. Error code: " << GetLastError());
    } else {
        DEBUG_LOG(L"All controls created successfully");
    }

    // Set modern font for all controls
    HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
                             OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                             DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    SendMessage(m_hToggleButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(m_hWhitelistBox, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(m_hAddButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(m_hRemoveButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(m_hStatusBar, WM_SETFONT, (WPARAM)hFont, TRUE);

    // Apply modern style to buttons
    if (hTheme) {
        SetWindowTheme(m_hToggleButton, L"Explorer", NULL);
        SetWindowTheme(m_hAddButton, L"Explorer", NULL);
        SetWindowTheme(m_hRemoveButton, L"Explorer", NULL);
        CloseThemeData(hTheme);
    }

    UpdateWhitelist();
    UpdateStatusBar(L"Ready");
}

void MainWindow::ResizeControls() {
    RECT rcClient;
    GetClientRect(m_hwnd, &rcClient);

    int width = rcClient.right - rcClient.left;
    int height = rcClient.bottom - rcClient.top;

    // Resize and reposition controls
    SetWindowPos(m_hToggleButton, NULL, (width - 180) / 2, 10, 180, 30, SWP_NOZORDER);
    SetWindowPos(m_hWhitelistBox, NULL, 10, 50, width - 20, height - 130, SWP_NOZORDER);
    SetWindowPos(m_hAddButton, NULL, (width - 250) / 2, height - 70, 120, 30, SWP_NOZORDER);
    SetWindowPos(m_hRemoveButton, NULL, (width - 250) / 2 + 130, height - 70, 120, 30, SWP_NOZORDER);

    // Resize status bar
    SendMessage(m_hStatusBar, WM_SIZE, 0, 0);
}

void MainWindow::ToggleMonitoring() {
    m_monitor.toggleModifying();
    SetWindowText(m_hToggleButton, m_monitor.isModifying() ? L"Pause Monitoring" : L"Resume Monitoring");
    UpdateStatusBar(m_monitor.isModifying() ? L"Monitoring active" : L"Monitoring paused");
    DEBUG_LOG(L"Monitoring toggled. New state: " << (m_monitor.isModifying() ? L"Active" : L"Paused"));
}

void MainWindow::UpdateWhitelist() {
    SendMessage(m_hWhitelistBox, LB_RESETCONTENT, 0, 0);
    for (const auto& item : m_monitor.getWhitelist()) {
        SendMessage(m_hWhitelistBox, LB_ADDSTRING, 0, (LPARAM)item.c_str());
    }
    DEBUG_LOG(L"Whitelist updated. Item count: " << m_monitor.getWhitelist().size());
}

void MainWindow::AddToWhitelist() {
    WCHAR currentDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentDir);

    OPENFILENAME ofn;
    wchar_t szFile[260] = { 0 };
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Executable\0*.exe\0All\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        std::wstring fullPath(szFile);
        size_t pos = fullPath.find_last_of(L"\\/");
        std::wstring fileName = (pos == std::wstring::npos) ? fullPath : fullPath.substr(pos + 1);
        
        if (!fileName.empty() && fileName != L"Add") {
            m_monitor.addToWhitelist(fileName);
            UpdateWhitelist();
            UpdateStatusBar(L"Program added to whitelist");
            DEBUG_LOG(L"Added to whitelist: " << fileName);
        }
    }

    SetCurrentDirectory(currentDir);
}

void MainWindow::RemoveFromWhitelist() {
    int selectedIndex = SendMessage(m_hWhitelistBox, LB_GETCURSEL, 0, 0);
    if (selectedIndex != LB_ERR) {
        wchar_t buffer[256];
        if (SendMessage(m_hWhitelistBox, LB_GETTEXT, selectedIndex, (LPARAM)buffer) != LB_ERR) {
            std::wstring removedItem(buffer);
            m_monitor.removeFromWhitelist(removedItem);
            UpdateWhitelist();
            UpdateStatusBar(L"Program removed from whitelist");
            DEBUG_LOG(L"Removed from whitelist: " << removedItem);
        }
    } else {
        UpdateStatusBar(L"No program selected");
    }
}

void MainWindow::LoadWhitelist() {
    m_monitor.loadAppWhitelist();
    UpdateWhitelist();
    UpdateStatusBar(L"Whitelist loaded");
    DEBUG_LOG(L"Whitelist loaded");
}

void MainWindow::SaveWhitelist() {
    m_monitor.saveAppWhitelist();
    UpdateStatusBar(L"Whitelist saved");
    DEBUG_LOG(L"Whitelist saved");
}

void MainWindow::UpdateStatusBar(const std::wstring& message) {
    SendMessage(m_hStatusBar, SB_SETTEXT, 0, (LPARAM)message.c_str());
}