#ifndef _TXF_EXPORT_H_
#define _TXF_EXPORT_H_

#include <fstream>
#include <filesystem>


#define TXTSEPARATOR "§"

class txtExport
{
public:
	txtExport();
	~txtExport();

	bool begin(const std::filesystem::path& fileName);
	bool end();

	//bool tag(const Tag& tagToExport);
private:

	std::ofstream m_file;
	bool f_closed;
	bool m_status;
};

#endif // !_DXF_EXPORT_H_
