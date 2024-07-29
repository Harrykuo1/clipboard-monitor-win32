#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <windows.h>
#include <vector>
#include <string>
#include "../core/ClipboardMonitor.h"

class MainWindow {
public:
    MainWindow(HINSTANCE hInstance, ClipboardMonitor& monitor);
    ~MainWindow();

    bool Create();
    void Show(int nCmdShow);
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND m_hwnd;
    HWND m_hToggleButton;
    HWND m_hWhitelistBox;
    HWND m_hAddButton;
    HWND m_hRemoveButton;
    HWND m_hStatusBar;
    HINSTANCE m_hInstance;
    ClipboardMonitor& m_monitor;

    void CreateControls();
    void ResizeControls();
    void ToggleMonitoring();
    void UpdateWhitelist();
    void AddToWhitelist();
    void RemoveFromWhitelist();
    void LoadWhitelist();
    void SaveWhitelist();
    void UpdateStatusBar(const std::wstring& message);
};

#endif // MAIN_WINDOW_H