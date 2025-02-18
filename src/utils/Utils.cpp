#include "utils/Utils.h"
#include "utils/Logger.h"

#include <fmt/format.h>

std::string Utils::completeWithZeros(const uint64_t& number, const uint8_t& size)
{
	return fmt::format("{:0{}d}", number, size);
}

std::wstring Utils::wCompleteWithZeros(const uint64_t& number, const uint8_t& size)
{
	wchar_t cmd[20];
	_snwprintf(cmd, 20, L"%%0%dI64d", size);
	wchar_t buff[100];
	_snwprintf(buff, 100, cmd, number);
	return std::wstring(buff);
}

std::string Utils::roundFloat(const double& number, const uint8_t& size)
{
	return fmt::format("{:.{}f}", number, size);
}

std::wstring Utils::wRoundFloat(const double& number, const uint8_t& size)
{
	wchar_t cmd[20];
	_snwprintf(cmd, 20, L"%%.%df", size);
	wchar_t buff[100];
	_snwprintf(buff, 100, cmd, number);
	return std::wstring(buff);
}

#ifdef _WIN32
#include <windows.h>
#include <stringapiset.h>

std::string Utils::to_utf8(const std::wstring& wide_string)
{
	int wlen = (int)wide_string.length();
	int len = WideCharToMultiByte(CP_UTF8, 0, wide_string.c_str(), wlen, NULL, 0, NULL, NULL);
	std::string str(len, L'\0');
	WideCharToMultiByte(CP_UTF8, 0, wide_string.c_str(), wlen, str.data(), len, NULL, NULL);
	return str;
}

std::wstring Utils::from_utf8(const std::string& u8string)
{
	int len = (int)u8string.length();
	int wlen = MultiByteToWideChar(CP_UTF8, 0, u8string.c_str(), len, NULL, 0);
	std::wstring wstr(wlen, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, u8string.c_str(), len, wstr.data(), wlen);
	return wstr;
}

std::wstring Utils::fromANSI(const std::string& string)
{
	int len;
	int slength = (int)string.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, string.c_str(), slength, 0, 0);
	std::wstring r(len, L'\0');
	MultiByteToWideChar(CP_ACP, 0, string.c_str(), slength, &r[0], len);
	return r;
}

#else
// TODO - UNIX system
#endif

std::wstring Utils::decode(const std::string& string)
{
	std::wstring ret;
	ret = from_utf8(string);
	if (ret != L"err_wstring")
		return ret;

	ret = fromANSI(string);
	return ret;
}

uint64_t Utils::hash_combine(uint64_t hash1, uint64_t hash2)
{
	return hash1 ^= hash2 + 0x517cc1b727220a95 + (hash1 << 6) + (hash1 >> 2);
}
