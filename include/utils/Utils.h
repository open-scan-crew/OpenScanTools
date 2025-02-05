#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>

namespace Utils
{
    std::string completeWithZeros(const uint64_t& number, const uint8_t& size = 3);
    std::wstring wCompleteWithZeros(const uint64_t& number, const uint8_t& size = 3);
    std::string roundFloat(const double& number, const uint8_t& size = 3);
    std::wstring wRoundFloat(const double& number, const uint8_t& size = 3);
    std::string to_utf8(const std::wstring& wide_string);
    std::wstring from_utf8(const std::string& u8string);
    std::wstring fromANSI(const std::string& string);

    std::wstring decode(const std::string& string);

    uint64_t hash_combine(uint64_t hash1, uint64_t hash2);
}

#endif
