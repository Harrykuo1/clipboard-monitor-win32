#include "ClipboardMonitor.h"
#include "../utils/Utils.h"
#include "../utils/DebugMacros.h"
#include <windows.h>
#include <fstream>
#include <algorithm>
#include <regex>
#include <codecvt>
#include <iostream>

ClipboardMonitor::ClipboardMonitor() : m_isModifying(true) {
    loadAppWhitelist();
    DEBUG_LOG(L"ClipboardMonitor initialized");
}

void ClipboardMonitor::loadAppWhitelist() {
    std::ifstream file("resources/whitelist.txt");
    std::string line;
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    while (std::getline(file, line)) {
        std::wstring wline = converter.from_bytes(line);
        if (!wline.empty()) {
            m_appWhitelist.insert(wline);
        }
    }
    if (m_appWhitelist.empty()) {
        m_appWhitelist = {L"chrome.exe", L"firefox.exe", L"iexplore.exe", L"microsoftedge.exe", L"opera.exe", L"brave.exe"};
    }
    DEBUG_LOG(L"App whitelist loaded. Items: " << m_appWhitelist.size());
    file.close();
}

void ClipboardMonitor::saveAppWhitelist() {
    std::ofstream file("resources/whitelist.txt");
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    for (const auto& app : m_appWhitelist) {
        file << converter.to_bytes(app) << std::endl;
    }
    DEBUG_LOG(L"App whitelist saved");
    file.close();
}

void ClipboardMonitor::addToWhitelist(const std::wstring& app) {
    auto [it, inserted] = m_appWhitelist.insert(app);
    if (inserted) {
        DEBUG_LOG(L"Added to whitelist: " << app);
    } else {
        DEBUG_LOG(L"Item already in whitelist: " << app);
    }
}

void ClipboardMonitor::removeFromWhitelist(const std::wstring& app) {
    auto it = m_appWhitelist.find(app);
    if (it != m_appWhitelist.end()) {
        m_appWhitelist.erase(it);
        DEBUG_LOG(L"Removed from whitelist: " << app);
    } else {
        DEBUG_LOG(L"Item not found in whitelist: " << app);
    }
}

void ClipboardMonitor::modifyClipboard() {
    if (!m_isModifying) {
        DEBUG_LOG(L"Clipboard modification is paused");
        return;
    }
    
    ClipboardContent content = readClipboard();
    if (content.isValid && content.text != m_lastContent) {
        std::wstring activeProcessName = getActiveWindowProcessName();
        DEBUG_LOG(L"Active window process: " << activeProcessName);
        
        if (isFromWhitelist(activeProcessName)) {
            m_lastContent = content.text;
            std::wstring modifiedText = content.text;
            std::wregex re(L"[\r\n\v\f\x85]+");
            modifiedText = std::regex_replace(modifiedText, re, L" ");
            writeClipboard(modifiedText);
            DEBUG_LOG(L"Clipboard content modified");
            m_lastModificationTime = std::chrono::steady_clock::now();
        } else {
            DEBUG_LOG(L"Clipboard content not modified (not from whitelisted app)");
        }
    } else {
        DEBUG_LOG(L"Clipboard content unchanged or invalid");
    }
}

void ClipboardMonitor::toggleModifying() {
    m_isModifying = !m_isModifying;
    DEBUG_LOG(L"Clipboard modification " << (m_isModifying ? L"enabled" : L"disabled"));
}

ClipboardContent ClipboardMonitor::readClipboard() {
    ClipboardContent content;
    if (!OpenClipboard(NULL)) {
        DEBUG_LOG(L"Error: Unable to open clipboard");
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
                DEBUG_LOG(L"Clipboard content read: " << content.text);
            }
        }
    }

    CloseClipboard();
    return content;
}

void ClipboardMonitor::writeClipboard(const std::wstring& text) {
    if (!OpenClipboard(NULL)) {
        DEBUG_LOG(L"Error: Unable to open clipboard for writing");
        return;
    }
    EmptyClipboard();
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (text.length() + 1) * sizeof(wchar_t));
    if (hMem != NULL) {
        memcpy(GlobalLock(hMem), text.c_str(), (text.length() + 1) * sizeof(wchar_t));
        GlobalUnlock(hMem);
        SetClipboardData(CF_UNICODETEXT, hMem);
        DEBUG_LOG(L"Modified content written to clipboard: " << text);
    }
    CloseClipboard();
}

bool ClipboardMonitor::isFromWhitelist(const std::wstring& processName) {
    return m_appWhitelist.find(processName) != m_appWhitelist.end();
}