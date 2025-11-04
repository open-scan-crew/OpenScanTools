#include "io/imports/FlsFileReader.h"
#include "models/pointCloud/PointXYZIRGB.h"

#include "windows.h"  // for CoInitialize() ??
#include <ctime>

#import "C:\Windows\WinSxS\Fusion\amd64_faro.ls_1d23f5635ba800ab_none_7fa4cf1298b29126\1.1\1.1.905.1\iQOpen.dll" no_namespace

#define LicenseCode   L"FARO Open Runtime License\nKey:G38WLPNMTCKXFK6TY3ZUSPVPL\n\nThe software is the registered property of FARO Scanner Production GmbH, Stuttgart, Germany.\nAll rights reserved.\nThis software may only be used with written permission of FARO Scanner Production GmbH, Stuttgart, Germany."

IiQLibIfPtr libRef;


bool FlsFileReader::getReader(const std::filesystem::path& filepath, std::wstring& log, FlsFileReader** reader, const bool forceColor)
{
    try {
        if (libRef == NULL)
        {
            CoInitialize(NULL);

            // FARO LS Licensing
            BSTR licenseCode = SysAllocString(LicenseCode);
            IiQLicensedInterfaceIfPtr liPtr(__uuidof(iQLibIf));
            liPtr->PutLicense(licenseCode);
            libRef = static_cast<IiQLibIfPtr>(liPtr);
            SysFreeString(licenseCode);
        }
        *reader = new FlsFileReader(filepath, log, forceColor);
    }
    catch (std::exception&) {
        log += L"Failed to initialize the FARO LS api. An unhandled exception occured.";
        //std::cerr << "Got an std::excetion, what=" << ex.what() << std::endl;
        return false;
    }
    catch (...) {
        log += L"Failed to initialize the FARO LS api. Some dll may be missing.";
        //std::cerr << "Got an unknown exception" << std::endl;
        return false;
    }
    return true;
}

FlsFileReader::FlsFileReader(const std::filesystem::path& filepath, std::wstring& log, const bool forceColor)
    : IScanFileReader(filepath)
    , m_readColor(forceColor)
    , m_currentScan(0)
    , m_currentCol(0)
{
    m_header.guid = xg::Guid(); // No Guid from the file

    int res = libRef->setAttribute("#app/ScanLoadColor", "0"); // usefull for fls
    res = libRef->load(m_filepath.c_str());
    if (res != 0)
    {
        log += L"Error when opening a Faro file: ";
        switch (res)
        {
        case 11:
            log += L"No Workspace (11)";
            break;
        case 12:
            log += L"(12)"; // La doc FARO ne liste pas ce code erreur dans sa table d’erreur
            break;
        case 13:
            log += L"No Scan (13)";
            break;
        case 25:
            log += L"(25)"; // La doc FARO ne liste pas ce code erreur dans sa table d’erreur
            break;
        default:
            log += L"Unknown code error ";
            break;
        }
        throw std::exception();
    }

    // NOTE(robin) - if the file is a single fls, the first and only scan is loaded and will stay loaded until we read it or destroy the file reader.

    m_header.scanCount = libRef->getNumScans();
    m_header.creationDate = 0u;
    m_header.version = tls::FileVersion::V_UNKNOWN;


    for (uint32_t n = 0; n < m_header.scanCount; ++n)
    {
        m_scanRowCols.push_back({ libRef->getScanNumRows(n), libRef->getScanNumCols(n) });

        tls::ScanHeader scanH;

        // Read the transformation
        double position[3];
        double orientation[4];
        libRef->getScanPosition(n, position, position + 1, position + 2);
        libRef->getScanOrientation(n, orientation, orientation + 1, orientation + 2, orientation + 3);

        IiQObjectIfPtr iqScan = libRef->getScanObject(n);
        _bstr_t scanName = iqScan->getName();
        IiQScanObjIfPtr realScan= iqScan->getScanObjSpecificIf();

        // Date
        int year, month, day, hour, min, sec, msec;
        res = realScan->getDate(&year, &month, &day, &hour, &min, &sec, &msec);

        // Hardware infos
        BSTR ID;
        BSTR Type;
        BSTR Serial;
        BSTR Info;
        double Range;
        res = realScan->getHardwareInfo(&ID, &Type, &Serial, &Info, &Range);

        /*
        // Angles
        IFAROScanAnglesIfPtr angles = realScan->getAnglesIf();
        double startHori;
        double deltaHori;
        angles->getHorizontalAngles(0, &startHori, &deltaHori);

        // Calibration
        double distOffset;
        double distFactor;
        double mirrorAdjustment;
        double mirrorAxisAdjustment;
        double horLaserAdjustment;
        double vertLaserAdjustment;
        double addtlAmplOffset;
        double addtlAmplFactor;
        double distCalibAngle;
        double distCalibLength;
        double triggerOffset;
        res = realScan->getCalibInfo(&distOffset, &distFactor,
            &mirrorAdjustment, &mirrorAxisAdjustment,
            &horLaserAdjustment, &vertLaserAdjustment,
            &addtlAmplOffset, &addtlAmplFactor,
            &distCalibAngle, &distCalibLength,
            &triggerOffset);
            */

        //std::wstring_convert<std::codecvt_utf8<char>, char> utfconv;

        scanH.guid = xg::Guid();

        tm timeStruct{ sec, min, hour, day, month, year - 1900, 0, 0, 0};
        scanH.acquisitionDate = mktime(&timeStruct);

        scanH.name = std::wstring(scanName);
         
        //Note (Aurélien) Converting with filesystem::path to not getting access violation from WideCharToMultiByte...
        std::filesystem::path pathConverter(Type);
        scanH.sensorModel = pathConverter.wstring();
        ///buffLength = WideCharToMultiByte(0, 0, Type, 64, cstr, 128, "a", FALSE);
        ///if (buffLength)
        ///    scanH.sensorModel.insert(0, cstr, buffLength);
        //scanH.sensorModel = std::string(cstr);

        pathConverter = std::filesystem::path(Serial);
        scanH.sensorSerialNumber = pathConverter.wstring();
        ///buffLength = WideCharToMultiByte(0, 0, Serial, 64, cstr, 128, "a", FALSE);
        ///if (buffLength)
        ///    scanH.sensorSerialNumber.insert(0, cstr, buffLength);
        //scanH.sensorSerialNumber = std::string(cstr);

        scanH.transfo.translation[0] = position[0];
        scanH.transfo.translation[1] = position[1];
        scanH.transfo.translation[2] = position[2];
        scanH.transfo.quaternion[0] = sin(orientation[3] / 2) * orientation[0];
        scanH.transfo.quaternion[1] = sin(orientation[3] / 2) * orientation[1];
        scanH.transfo.quaternion[2] = sin(orientation[3] / 2) * orientation[2];
        scanH.transfo.quaternion[3] = cos(orientation[3] / 2);
        scanH.limits = { 1.f, -1.f, 1.f, -1.f, 1.f, -1.f }; // fake bbox, we do not have the info
        scanH.pointCount = libRef->getScanNumRows(n) * libRef->getScanNumCols(n);
        scanH.precision = tls::PrecisionType::TL_OCTREE_100UM;
        scanH.format = m_readColor ? tls::PointFormat::TL_POINT_XYZ_I_RGB : tls::PointFormat::TL_POINT_XYZ_I;

        m_scanHeaders.push_back(scanH);
    }
}

FlsFileReader::~FlsFileReader()
{
    libRef = NULL;
    CoUninitialize();
}

FileType FlsFileReader::getType() const
{
    return FileType::FARO_LS;
}

uint32_t FlsFileReader::getScanCount() const
{
    return m_header.scanCount;
}

uint64_t FlsFileReader::getTotalPoints() const
{
    uint64_t numPoints = 0;
    for (uint32_t n = 0; n < m_scanRowCols.size(); ++n)
    {
        numPoints += m_scanRowCols[n].first * m_scanRowCols[n].second;
    }

    return (numPoints);
}

tls::FileHeader FlsFileReader::getTlsHeader() const
{
    return m_header;
}

tls::ScanHeader FlsFileReader::getTlsScanHeader(uint32_t scanNumber) const
{
    if (scanNumber < m_scanHeaders.size())
        return m_scanHeaders[scanNumber];
    else
        return (tls::ScanHeader{});
}

bool FlsFileReader::startReadingScan(uint32_t scanNumber)
{
    if (scanNumber >= m_header.scanCount)
        return false;

    m_currentScan = scanNumber;
    m_currentCol = 0;
    int result;

    // Read all the colors and store them for a future assembly with intensity.
    // Positions are not stored they will be read again with the intensity.
    if (m_readColor)
    {
        m_colorBuffer.clear();
        int numRows = libRef->getScanNumRows(m_currentScan);
        int numCols = libRef->getScanNumCols(m_currentScan);
        m_colorBuffer.resize(numRows * numCols);
        double* positions = new double[numRows * 3];

        libRef->unloadScan(m_currentScan);
        // Reflection Mode (2) to get the color on 24bits
        libRef->PutscanReflectionMode(2);
        result = libRef->setAttribute("#app/ScanLoadColor", "1");
        libRef->loadScan(m_currentScan);

        for (int col = 0; col < numCols; col++)
        {
            result = libRef->getXYZScanPoints(m_currentScan, 0, col, numRows, positions, m_colorBuffer.data() + col * numRows);
        }
        delete[] positions;
        libRef->unloadScan(m_currentScan);
    }

    libRef->PutscanReflectionMode(1);
    result = libRef->setAttribute("#app/ScanLoadColor", "0");
    if (result != 0)
        return false;
    libRef->loadScan(m_currentScan);

    return true;
}

bool FlsFileReader::readPoints(PointXYZIRGB* dstBuf, uint64_t bufSize, uint64_t& readCount)
{
    if (libRef->getScanLoadState(m_currentScan) == 0)
    {
        readCount = 0;
        return false;
    }

    int result;
    int numRows = libRef->getScanNumRows(m_currentScan);
    int numCols = libRef->getScanNumCols(m_currentScan);

    // local buffers
    double* positions = new double[numRows * 3];
    std::vector<int> reflections;
    reflections.resize(numRows);

    // Read column by column
    uint64_t bufId = 0;
    for ( ; m_currentCol < numCols; ++m_currentCol)
    {
        // check that there is enough space in the destination buffer
        if (bufId + numRows > bufSize)
            break;

        result = libRef->getXYZScanPoints(m_currentScan, 0, m_currentCol, numRows, positions, reflections.data());
        if (result != 0)
            continue;

        for (int row = 0; row < numRows; row++)
        {
            if (positions[3 * row] == 0.0)
                continue;

            // write the points position and intensity
            dstBuf[bufId].x = (float)positions[3 * row];
            dstBuf[bufId].y = (float)positions[3 * row + 1];
            dstBuf[bufId].z = (float)positions[3 * row + 2];
            dstBuf[bufId].i = (uint8_t)reflections[row];
            // write the color previouly red
            if (m_readColor)
            {
                dstBuf[bufId].r = (uint8_t)m_colorBuffer[m_currentCol * numRows + row];
                dstBuf[bufId].g = (uint8_t)(m_colorBuffer[m_currentCol * numRows + row] >> 8);
                dstBuf[bufId].b = (uint8_t)(m_colorBuffer[m_currentCol * numRows + row] >> 16);
            }
            ++bufId;
        }
    }
    delete[] positions;

    if (bufId == 0)
    {
        m_colorBuffer.clear();
        libRef->unloadScan(m_currentScan);
    }

    readCount = bufId;
    return true;
}
