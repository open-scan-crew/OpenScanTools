#include "utils.h"

#ifdef _WIN32
#include <windows.h>

std::string Utils::wstr_to_utf8(const std::wstring& wide_string)
{
    int wlen = (int)wide_string.length() + 1;
    int len = WideCharToMultiByte(CP_UTF8, 0, wide_string.c_str(), wlen, NULL, 0, NULL, NULL);
    std::string str(len, L'\0');
    WideCharToMultiByte(CP_UTF8, 0, wide_string.c_str(), wlen, str.data(), len, NULL, NULL);
    return str;
}

std::wstring Utils::utf8_to_wstr(const std::string& u8string)
{
    int len = (int)u8string.length() + 1;
    int wlen = MultiByteToWideChar(CP_UTF8, 0, u8string.c_str(), len, NULL, 0);
    std::wstring wstr(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, u8string.c_str(), len, wstr.data(), wlen);
    return wstr;
}

#endif