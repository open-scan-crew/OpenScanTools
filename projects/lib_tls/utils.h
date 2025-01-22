#ifndef UTILS_H
#define UTILS_H
#include <string>

class Utils
{
public:
    static std::string wstr_to_utf8(const std::wstring& wide_string);
    static std::wstring utf8_to_wstr(const std::string& u8string);
};

#endif