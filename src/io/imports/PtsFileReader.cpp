#include "io/imports/PtsFileReader.h"
#include "models/pointCloud/PointXYZIRGB.h"
#include "utils/time.h"
#include "utils/Utils.h"
#include "utils/Logger.h"

bool PtsFileReader::getReader(const std::filesystem::path& filepath, std::wstring& log, IScanFileReader** reader, const AsciiImport::Info& asciiInfo)
{
	if (!std::filesystem::exists(filepath))
		return false;

	try
	{
		*reader = new PtsFileReader(filepath, asciiInfo);
	}
	catch (std::exception& e)
	{
		Logger::log(LoggerMode::IOLog) << e.what() << Logger::endl;
		return false;
	}

	return true;
}

PtsFileReader::PtsFileReader(const std::filesystem::path& filepath, const AsciiImport::Info& asciiInfo)
	: IScanFileReader(filepath)
	, m_asciiInfo(asciiInfo)
	, totalPointCount(0)
{
	std::ifstream fileStream = std::ifstream(filepath, std::ios::in);
	if (!fileStream.good())
	{
		char msg[1024];
		sprintf(msg, "Failed to load ascii scan file : %ls \n", filepath.c_str());
		throw (std::exception::exception(msg));
	}

	m_header.guid = xg::newGuid();
	m_header.scanCount = 1;
	m_header.creationDate = 0;
	m_header.version = tls::FileVersion::V_UNKNOWN;

	m_scanHeader.guid = m_header.guid;
	m_scanHeader.version = tls::ScanVersion::SCAN_V_0_4;
	m_scanHeader.acquisitionDate = 0; // no date
	m_scanHeader.name = filepath.filename().wstring();
	m_scanHeader.sensorModel = L"Not provided";
	m_scanHeader.sensorSerialNumber = L"Not provided";

	PointXYZIRGB translationPoint;
	while (!fileStream.eof())
	{
		std::string line;
		PointXYZIRGB point;
		std::getline(fileStream, line);
		if (getNextPoint(line, point))
		{
			translationPoint = point;
			break;
		}
	}

	m_scanHeader.transfo = { { 0.0, 0.0, 0.0, 1.0}, { translationPoint.x, translationPoint.y, translationPoint.z} };

	const std::vector<AsciiImport::ValueRole>& format = m_asciiInfo.columnsRole;

	bool containX = std::find(format.begin(), format.end(), AsciiImport::ValueRole::X) != format.end();
	bool containY = std::find(format.begin(), format.end(), AsciiImport::ValueRole::Y) != format.end();
	bool containZ = std::find(format.begin(), format.end(), AsciiImport::ValueRole::Z) != format.end();

	bool containR = std::find(format.begin(), format.end(), AsciiImport::ValueRole::R) != format.end() || std::find(format.begin(), format.end(), AsciiImport::ValueRole::Rf) != format.end();
	bool containG = std::find(format.begin(), format.end(), AsciiImport::ValueRole::G) != format.end() || std::find(format.begin(), format.end(), AsciiImport::ValueRole::Gf) != format.end();
	bool containB = std::find(format.begin(), format.end(), AsciiImport::ValueRole::B) != format.end() || std::find(format.begin(), format.end(), AsciiImport::ValueRole::Bf) != format.end();
	
	bool containI = std::find(format.begin(), format.end(), AsciiImport::ValueRole::I) != format.end();

	m_scanHeader.format = tls::PointFormat::TL_POINT_NOT_COMPATIBLE;

	if (containZ && containY && containZ)
	{
		//Default I if dont contain I
		m_scanHeader.format = tls::PointFormat::TL_POINT_XYZ_I;

		if (containR && containG && containB)
		{
			if(containI)
				m_scanHeader.format = tls::PointFormat::TL_POINT_XYZ_I_RGB;
			else
				m_scanHeader.format = tls::PointFormat::TL_POINT_XYZ_RGB;


		} 
	}


	fileStream.close();

}

PtsFileReader::~PtsFileReader()
{
	m_streamReadScan.close();
}

FileType PtsFileReader::getType() const
{
	return FileType::PTS;
}

uint32_t PtsFileReader::getScanCount() const
{
	return 1;
}

uint64_t PtsFileReader::getTotalPoints() const
{
	return totalPointCount;
}

const tls::FileHeader& PtsFileReader::getTlsHeader() const
{
	return m_header;
}

tls::ScanHeader PtsFileReader::getTlsScanHeader(uint32_t scanNumber) const
{
	return m_scanHeader;
}

bool PtsFileReader::startReadingScan(uint32_t scanNumber)
{
	m_streamReadScan = std::ifstream(m_filepath, std::ios::in);
	if (!m_streamReadScan.good())
	{
		Logger::log(LoggerMode::IOLog) << "Failed to load ascii scan file : " << m_filepath << LOGENDL;
		return false;
	}
	return true;
}

bool PtsFileReader::readPoints(PointXYZIRGB* dstBuf, uint64_t bufSize, uint64_t& readCount)
{
	readCount = 0;
	while (readCount < bufSize)
	{
		if (m_streamReadScan.eof())
			return false;

		std::string line;
		std::getline(m_streamReadScan, line);

		if (line.empty())
			return true;

		PointXYZIRGB point;
		point.i = 255;
		if (!getNextPoint(line, point))
			continue;

		point.x -= (float)(m_scanHeader.transfo.translation[0]);
		point.y -= (float)(m_scanHeader.transfo.translation[1]);
		point.z -= (float)(m_scanHeader.transfo.translation[2]);

		dstBuf[readCount] = point;
		readCount++;
	}

	return true;
}

bool PtsFileReader::getNextPoint(const std::string& line, PointXYZIRGB& point)
{
	std::string temp;
	int valueInd = 0;
	for (char c : line)
	{
		if (m_asciiInfo.useCommaAsDecimal && c == ',')
			c = '.';

		if (c == m_asciiInfo.sep)
		{
			if (temp.empty())
				continue;

			if (valueInd >= m_asciiInfo.columnsRole.size())
				return false;

			AsciiImport::ValueRole role = m_asciiInfo.columnsRole[valueInd];

			if (!fillPoint(temp, point, role))
				return false;

			temp.clear();
			valueInd++;
		}
		else
			temp += c;
	}

	if (temp.empty())
	{
		if (valueInd != m_asciiInfo.columnsRole.size())
			return false;
		return true;
	}
	else
	{
		if (valueInd != m_asciiInfo.columnsRole.size() - 1)
			return false;

		AsciiImport::ValueRole role = m_asciiInfo.columnsRole[valueInd];

		if (!fillPoint(temp, point, role))
			return false;
		return true;
	}
}

bool PtsFileReader::fillPoint(const std::string& value, PointXYZIRGB& point, const AsciiImport::ValueRole& valueRole)
{
	try {
		switch (valueRole)
		{
			case AsciiImport::ValueRole::X:
			{
				point.x = std::stof(value);
			}
			break;
			case AsciiImport::ValueRole::Y:
			{
				point.y = std::stof(value);
			}
			break;
			case AsciiImport::ValueRole::Z:
			{
				point.z = std::stof(value);
			}
			break;
			case AsciiImport::ValueRole::R:
			case AsciiImport::ValueRole::G:
			case AsciiImport::ValueRole::B:
			case AsciiImport::ValueRole::I:
			{
				int val = std::stoi(value);
				if (val > 255)
					val = 255;
				if (val < 0)
					val = 0;
				switch (valueRole)
				{
					case AsciiImport::ValueRole::R:
						point.r = val;
						break;
					case AsciiImport::ValueRole::G:
						point.g = val;
						break;
					case AsciiImport::ValueRole::B:
						point.b = val;
						break;
					case AsciiImport::ValueRole::I:
						point.i = val;
						break;
				}
			}
			break;
			case AsciiImport::ValueRole::Rf:
			case AsciiImport::ValueRole::Gf:
			case AsciiImport::ValueRole::Bf:
			{
				double val = std::stod(value);
				if (val > 1.)
					val = 1.;
				if (val < 0.)
					val = 0.;

				int ival = (int)(255. * val);
				switch (valueRole)
				{
				case AsciiImport::ValueRole::Rf:
					point.r = ival;
					break;
				case AsciiImport::ValueRole::Gf:
					point.g = ival;
					break;
				case AsciiImport::ValueRole::Bf:
					point.b = ival;
					break;
				}
			}
			break;
			case AsciiImport::ValueRole::Ignore:
			{}
			break;
			default:
			{
				assert(false);
				return false;
			}
			break;
		}
	}
	catch (...)
	{
		//assert(false);
		return false;
	}

	return true;
}
