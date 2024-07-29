#include "TrayIcon.h"
#include <shellapi.h>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAYICON 1

TrayIcon::TrayIcon(HWND hwnd) {
    m_nid.cbSize = sizeof(NOTIFYICONDATA);
    m_nid.hWnd = hwnd;
    m_nid.uID = ID_TRAYICON;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAYICON;
    m_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
}

void TrayIcon::create() {
    Shell_NotifyIcon(NIM_ADD, &m_nid);
}

void TrayIcon::update(const wchar_t* tip) {
    wcsncpy_s(m_nid.szTip, tip, _TRUNCATE);
    Shell_NotifyIcon(NIM_MODIFY, &m_nid);
}

void TrayIcon::remove() {
    Shell_NotifyIcon(NIM_DELETE, &m_nid);
}