#include "io/exports/CSVWriter.h"

#include "utils/Utils.h"

const EndCSV CSVWriter::endl;

CSVWriter::CSVWriter()
{}

CSVWriter::CSVWriter(const std::filesystem::path& file, const std::string& separator)
	: m_separator(separator)
{
	m_ostream.open(file);
	m_ostream.precision(15);
}

CSVWriter::~CSVWriter()
{
	m_ostream.flush();
	m_ostream.close();
}

bool CSVWriter::open(const std::filesystem::path& file, const std::string& separator)
{
	close();
	m_ostream.open(file);
	if (!m_ostream.is_open())
		return false;
	m_separator = separator;
	return true;
}

bool CSVWriter::isGood() const
{
	return m_ostream.good();
}

bool CSVWriter::isOpen() const
{
	return m_ostream.is_open();
}

void CSVWriter::endLine()
{
	m_ostream << "\n";
}

void CSVWriter::close()
{
	if (m_ostream.is_open())
		m_ostream.close();
}

CSVWriter& CSVWriter::operator<<(std::wstring value)
{
	m_ostream << Utils::to_utf8(value) << m_separator;
	return (*this);
}

CSVWriter& CSVWriter::operator<<(const wchar_t* value)
{
	std::wstring text = L"";
	text.append(value);
	m_ostream << Utils::to_utf8(text) << m_separator;
	return (*this);
}

CSVWriter& CSVWriter::operator<<(std::string value)
{
	m_ostream << value << m_separator;
	return (*this);
}

CSVWriter&  CSVWriter::operator<<(const EndCSV&)
{
	endLine();
	return (*this);
}