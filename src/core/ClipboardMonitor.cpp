#include "ClipboardMonitor.h"
#include "../utils/Utils.h"
#include <windows.h>
#include <fstream>
#include <algorithm>
#include <regex>
#include <codecvt>
#include <iostream>

ClipboardMonitor::ClipboardMonitor() : m_isModifying(true) {
    loadAppWhitelist();
}

void ClipboardMonitor::loadAppWhitelist() {
    std::ifstream file("../../resources/whitelist.txt");
    std::string line;
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            m_appWhitelist.push_back(converter.from_bytes(line));
        }
    }
    if (m_appWhitelist.empty()) {
        m_appWhitelist = {L"chrome.exe", L"firefox.exe", L"iexplore.exe", L"microsoftedge.exe", L"opera.exe", L"brave.exe"};
    }
}

void ClipboardMonitor::modifyClipboard() {
    if (!m_isModifying) return;
    
    ClipboardContent content = readClipboard();
    if (content.isValid && content.text != m_lastContent) {
        std::wstring activeProcessName = getActiveWindowProcessName();
        std::wcout << L"Active window process: " << activeProcessName << std::endl;
        
        if (isFromWhitelist(activeProcessName)) {
            m_lastContent = content.text;
            std::wstring modifiedText = content.text;
            std::wregex re(L"[\r\n\v\f\x85]+");
            modifiedText = std::regex_replace(modifiedText, re, L" ");
            writeClipboard(modifiedText);
            std::wcout << L"App clipboard content modified" << std::endl;
            m_lastModificationTime = std::chrono::steady_clock::now();
        } else {
            std::wcout << L"Clipboard content not modified (not from whitelisted)" << std::endl;
        }
    }
}

void ClipboardMonitor::toggleModifying() {
    m_isModifying = !m_isModifying;
}

ClipboardContent ClipboardMonitor::readClipboard() {
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

void ClipboardMonitor::writeClipboard(const std::wstring& text) {
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

bool ClipboardMonitor::isFromWhitelist(const std::wstring& processName) {
    return std::find(m_appWhitelist.begin(), m_appWhitelist.end(), processName) != m_appWhitelist.end();
}