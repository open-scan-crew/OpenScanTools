#ifndef _UTILS_H_
#define _UTILS_H_

#include <istream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <string>
#include <filesystem>

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

	//From https://stackoverflow.com/questions/5056645/sorting-stdmap-using-value
	template<typename A, typename B>
	std::pair<B, A> flip_pair(const std::pair<A, B>& p)
	{
		return std::pair<B, A>(p.second, p.first);
	}

	template<typename A, typename B>
	std::multimap<B, A> flip_map(const std::unordered_map<A, B>& src)
	{
		std::multimap<B, A> dst;
		std::transform(src.begin(), src.end(), std::inserter(dst, dst.begin()),
			flip_pair<A, B>);
		return dst;
	}
}

#endif
