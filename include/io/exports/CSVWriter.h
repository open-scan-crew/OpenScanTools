#ifndef _CSVWRITER_H_
#define _CSVWRITER_H_

#include <fstream>
#include <filesystem>
#include "utils/Utils.h"

class EndCSV
{
	friend std::ofstream& operator<<(std::ofstream& os, const EndCSV& endl)
	{
		return os;
	}
};

class CSVWriter
{
	public:
		CSVWriter();
		CSVWriter(const std::filesystem::path& file, const std::string& separator= ";");
		~CSVWriter();

		bool open(const std::filesystem::path& file, const std::string& separator = ";");
		bool isGood() const;
		bool isOpen() const;
		void endLine();
		void close();

		template<typename T>
		CSVWriter& operator<<(T &&value);

		CSVWriter& operator<<(std::wstring value);
		CSVWriter& operator<<(const wchar_t* value);
		CSVWriter& operator<<(std::string value);
		CSVWriter& operator<<(const EndCSV&);

	public:
		static const EndCSV endl;

	protected:
		std::string m_separator;
		std::ofstream m_ostream;
};

#endif //_CSVWRITER_H_