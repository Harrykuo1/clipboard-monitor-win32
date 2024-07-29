#ifndef TRAY_ICON_H
#define TRAY_ICON_H

#include <windows.h>

class TrayIcon {
public:
    TrayIcon(HWND hwnd);
    void create();
    void update(const wchar_t* tip);
    void remove();

private:
    NOTIFYICONDATA m_nid;
};

#endif // TRAY_ICON_H