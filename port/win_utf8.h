#pragma once

#if defined(_WIN32)

#include <windows.h>

#include <string>

namespace ssb64 {

/**
 * Converts a UTF-8 string to UTF-16 for the wide (W) Win32 calls. The port keeps paths as UTF-8, and the ANSI (A)
 * calls would read them in the system code page, so a folder like "Área de Trabalho" would not be found.
 *
 * @param utf8 UTF-8 text, usually a path or a command line.
 * @return The UTF-16 text, or an empty string when the input is empty or not valid UTF-8.
 */
inline std::wstring Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) {
        return std::wstring();
    }
    const int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), (int)utf8.size(), nullptr, 0);
    if (len <= 0) {
        return std::wstring();
    }
    std::wstring wide(len, L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), (int)utf8.size(), wide.data(), len);
    return wide;
}

/**
 * Converts UTF-16 text from a wide (W) Win32 call to the UTF-8 the port uses for paths.
 *
 * @param wide UTF-16 text.
 * @return The UTF-8 text, or an empty string when the input is empty or cannot be converted.
 */
inline std::string WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) {
        return std::string();
    }
    const int len = WideCharToMultiByte(CP_UTF8, 0, wide.data(), (int)wide.size(), nullptr, 0, nullptr, nullptr);
    if (len <= 0) {
        return std::string();
    }
    std::string utf8(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), (int)wide.size(), utf8.data(), len, nullptr, nullptr);
    return utf8;
}

} // namespace ssb64

#endif
