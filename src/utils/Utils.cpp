#include "utils/Utils.h"
#include "utils/Logger.h"

#include <locale>
#include <codecvt>
#include <windows.h>
#include <stringapiset.h>

#include <fmt/format.h>

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

std::string Utils::to_utf8(const std::wstring& wide_string)
{
	try {
		static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
		return utf8_conv.to_bytes(wide_string);
	}
	catch (std::exception) {
		Logger::log(LoggerMode::IOLog) << "Error when converting wstring: " << Logger::endl;
		return "err_string";
	}
}

std::wstring Utils::from_utf8(const std::string& u8string)
{
	try {
		static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
		return utf8_conv.from_bytes(u8string);
	}
	catch (std::exception) 
	{
		Logger::log(LoggerMode::IOLog) << "Error when converting string: " << u8string << Logger::endl;
		return L"err_wstring";
	}
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
