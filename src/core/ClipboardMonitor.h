#ifndef CLIPBOARD_MONITOR_H
#define CLIPBOARD_MONITOR_H

#include <string>
#include <vector>
#include <chrono>

struct ClipboardContent {
    bool isValid = false;
    std::wstring text;
    ClipboardContent() = default;
};

class ClipboardMonitor {
public:
    ClipboardMonitor();
    void loadAppWhitelist();
    void modifyClipboard();
    void toggleModifying();
    bool isModifying() const { return m_isModifying; }

private:
    bool m_isModifying;
    std::wstring m_lastContent;
    std::chrono::steady_clock::time_point m_lastModificationTime;
    std::vector<std::wstring> m_appWhitelist;

    ClipboardContent readClipboard();
    void writeClipboard(const std::wstring& text);
    bool isFromWhitelist(const std::wstring& processName);
};

#endif // CLIPBOARD_MONITOR_H