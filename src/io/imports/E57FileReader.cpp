#include "io/imports/E57FileReader.h"
#include "models/pointCloud/PointXYZIRGB.h"
#include "utils/time.h"
#include "utils/Utils.h"
#include "utils/Logger.h"

bool E57FileReader::getReader(const std::filesystem::path& filepath, std::wstring& log, E57FileReader** reader)
{
    try {
        // Read the file from disk
        // NOTE - the lib openE57 works the .string() conversion from filesystem.
        e57::ImageFile imf(filepath.string(), "r");

        // If we are here, this is a valid e57 file
        *reader = new E57FileReader(filepath, imf);
    }
    catch (e57::E57Exception& ex) {
        ex.report(__FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    catch (std::exception& ex) {
        std::cerr << "Got an std::exception, what=" << ex.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Got an unknown exception" << std::endl;
        return false;
    }
    return true;
}

E57FileReader::E57FileReader(const std::filesystem::path& filepath, e57::ImageFile imf)
    : IScanFileReader(filepath)
    , m_imf(imf)
    , m_cvReaderWrapper(nullptr)
{
    e57::StructureNode root = m_imf.root();

    {
        std::string log;
        e57Utils::printFileStructure(m_imf, log);
        Logger::log(LoggerMode::IOLog) << log << Logger::endl;
    }

    e57::StringNode guidNode(root.get("/guid"));
    m_header.guid = xg::Guid(guidNode.value());

    e57::VectorNode data3D(root.get("/data3D"));
    m_header.scanCount = (uint32_t)data3D.childCount();
    m_hasPoseTranslation.assign(m_header.scanCount, false);
    m_hasLocalOffsets.assign(m_header.scanCount, false);
    m_localOffsets.assign(m_header.scanCount, glm::dvec3(0.0));

    if (root.isDefined("/creationDateTime"))
    {
        e57::StructureNode dateNode(root.get("/creationDateTime"));
        e57::FloatNode dateValue(dateNode.get("dateTimeValue"));
        m_header.creationDate = scs::dtime_gps_to_utc(dateValue.value());
    }

    m_header.version = tls::FileVersion::V_UNKNOWN;

    totalPointCount = 0;

    for (uint32_t iData = 0; iData < m_header.scanCount; iData++)
    {
        e57::StructureNode dataSet(data3D.get(iData));

        // Compressed vector containing the points
        e57::CompressedVectorNode cvNode(dataSet.get("points"));
        totalPointCount += cvNode.childCount();

        tls::ScanHeader scHeader;
        std::string msg;
        if (getData3dInfo(iData, scHeader, msg))
        {
            m_scanHeaders.push_back(scHeader);
            totalPointCount += scHeader.pointCount;
        }
        else
        {
            throw std::exception(msg.c_str());
        }
    }
}

bool E57FileReader::getData3dInfo(uint32_t _id, tls::ScanHeader& _info, std::string& errorMsg)
{
    try {
        e57::StructureNode root = m_imf.root();

        e57::VectorNode data3D(root.get("/data3D"));

        if (data3D.childCount() <= _id)
            return false;

        e57::StructureNode dataSet(data3D.get(_id));
        m_hasPoseTranslation[_id] = dataSet.isDefined("pose/translation");

        // Get the name of the Scan(optional in e57)
        if (dataSet.isDefined("name"))
        {
            e57::StringNode name(dataSet.get("name"));
            _info.name = Utils::from_utf8(name.value());
        }

        // Acquisition Date
        if (dataSet.isDefined("acquisitionStart"))
        {
            e57::Node dateNode(dataSet.get("acquisitionStart/dateTimeValue"));
            e57::FloatNode fNode(dateNode);
            _info.acquisitionDate = scs::dtime_gps_to_utc(fNode.value());
        }
        else
        {
            _info.acquisitionDate = 0;
        }

        // Point count
        e57::CompressedVectorNode cvNode(dataSet.get("points"));
        _info.pointCount = cvNode.childCount();

        // Sensor Model
        if (dataSet.isDefined("sensorModel"))
        {
            e57::StringNode modelNode(dataSet.get("sensorModel"));
            _info.sensorModel = Utils::from_utf8(modelNode.value());
        }

        // Sensor Serial Number
        if (dataSet.isDefined("sensorSerialNumber"))
        {
            e57::StringNode serialNumberNode(dataSet.get("sensorSerialNumber"));
            _info.sensorSerialNumber = Utils::from_utf8(serialNumberNode.value());
        }

        // Transformation
        TlResult result = e57Utils::getRigidBodyTransform(dataSet, _info.transfo.translation, _info.transfo.quaternion);
        if (e57Utils::check_tl_result(result, errorMsg) == false)
            return false;

        // Bounding Box
        if (dataSet.isDefined("cartesianBounds"))
        {
            e57::StructureNode cartesianBounds(dataSet.get("cartesianBounds"));
            _info.limits.xMin = (float)e57::FloatNode(cartesianBounds.get("xMinimum")).value();
            _info.limits.xMax = (float)e57::FloatNode(cartesianBounds.get("xMaximum")).value();
            _info.limits.yMin = (float)e57::FloatNode(cartesianBounds.get("yMinimum")).value();
            _info.limits.yMax = (float)e57::FloatNode(cartesianBounds.get("yMaximum")).value();
            _info.limits.zMin = (float)e57::FloatNode(cartesianBounds.get("zMinimum")).value();
            _info.limits.zMax = (float)e57::FloatNode(cartesianBounds.get("zMaximum")).value();
        }
        else
        {
            _info.limits = { (float)_info.transfo.translation[0], (float)_info.transfo.translation[0], 
                (float)_info.transfo.translation[1], (float)_info.transfo.translation[1], 
                (float)_info.transfo.translation[2], (float)_info.transfo.translation[2] };
        }

        if (!dataSet.isDefined("pose/translation") && dataSet.isDefined("cartesianBounds"))
        {
            _info.transfo.translation[0] = round((_info.limits.xMin + _info.limits.xMax) / 200) * 100;
            _info.transfo.translation[1] = round((_info.limits.yMin + _info.limits.yMax) / 200) * 100;
            _info.transfo.translation[2] = round((_info.limits.zMin + _info.limits.zMax) / 200) * 100;
            m_hasLocalOffsets[_id] = true;
            m_localOffsets[_id] = glm::dvec3(_info.transfo.translation[0], _info.transfo.translation[1], _info.transfo.translation[2]);
        }

        E57AttribFormat e57Format;

        // Format
        if (e57Utils::detectFormat(dataSet, e57Format))
        {
            errorMsg += "ERROR: E57 point data -> encoding format not supported\n";
            e57Utils::printFileStructure(m_imf, errorMsg);
            return false;
        }

        // Select the right tls point format based on the attributes available
        if ((e57Format.color != TL_RGB_NONE) && (e57Format.intensity == TL_I_NONE)) {
            _info.format = tls::TL_POINT_XYZ_RGB;
        }
        else if ((e57Format.color == TL_RGB_NONE) && (e57Format.intensity != TL_I_NONE)) {
            _info.format = tls::TL_POINT_XYZ_I;
        }
        else {
            _info.format = tls::TL_POINT_XYZ_I_RGB;
        }
    }
    catch (e57::E57Exception& ex) {
        std::stringbuf buffer;
        std::ostream ostr(&buffer);
        ex.report(__FILE__, __LINE__, __FUNCTION__, ostr);
        errorMsg += buffer.str();
        return false;
    }
    catch (std::exception& ex) {
        errorMsg += "Got an std::excetion, what=";
        errorMsg += ex.what();
        errorMsg += "\n";
        return false;
    }
    catch (...) {
        errorMsg += "Got an unknown exception\n";
        return false;
    }
    return true;
}

E57FileReader::~E57FileReader()
{
    if (m_cvReaderWrapper != nullptr)
    {
        delete m_cvReaderWrapper;
    }
}

FileType E57FileReader::getType() const
{
    return FileType::E57;
}

uint32_t E57FileReader::getScanCount() const
{
    return m_header.scanCount;
}

uint64_t E57FileReader::getTotalPoints() const
{
    return totalPointCount;
}

tls::FileHeader E57FileReader::getTlsHeader() const
{
    return m_header;
}

tls::ScanHeader E57FileReader::getTlsScanHeader(uint32_t scanNumber) const
{
    if (scanNumber < m_scanHeaders.size())
        return m_scanHeaders[scanNumber];
    else
        return (tls::ScanHeader{});
}

bool E57FileReader::hasPoseTranslation(uint32_t scanNumber) const
{
    if (scanNumber < m_hasPoseTranslation.size())
        return m_hasPoseTranslation[scanNumber];
    return false;
}

bool E57FileReader::usesLocalOffsets(uint32_t scanNumber) const
{
    if (scanNumber < m_hasLocalOffsets.size())
        return m_hasLocalOffsets[scanNumber];
    return false;
}

bool E57FileReader::startReadingScan(uint32_t scanNumber)
{
    if (scanNumber >= m_header.scanCount)
        return false;
    m_currentScanIndex = scanNumber;

    TlResult result;
    try {
        // Read the file from disk
        e57::StructureNode root = m_imf.root();

        // Automaticaly checks if the node "data3D" is a VectorNode
        // if not raises an exception
        e57::VectorNode data3D(root.get("/data3D"));

        // Get the number of data sets
        uint64_t nbData3D = data3D.childCount();

        e57::StructureNode dataSet(data3D.get(scanNumber));
        std::vector<e57::SourceDestBuffer> destBuffers;
        E57AttribFormat format;
        Limits limits;
        std::string msg;

        result = e57Utils::detectFormat(dataSet, format);
        if (e57Utils::check_tl_result(result, msg) == false)
            std::cerr << msg << std::endl;

        m_stagingBuffers.resize(1048576u, format);

        result = e57Utils::initDestBuffers(m_imf, dataSet, destBuffers, m_stagingBuffers, format, limits);
        if (e57Utils::check_tl_result(result, msg) == false)
            std::cerr << msg << std::endl;

        // Compressed vector containing the points
        e57::CompressedVectorNode cvNode(dataSet.get("points"));

        // Create the reader from compressed vector and the destination buffers already configured
        if (m_cvReaderWrapper)
        {
            delete m_cvReaderWrapper;
        }
        m_cvReaderWrapper = new CVReaderWrapper{ cvNode.reader(destBuffers), format, limits };
    }
    catch (e57::E57Exception& ex) {
        ex.report(__FILE__, __LINE__, __FUNCTION__);
        return false;
    }
    catch (std::exception& ex) {
        std::cerr << "Got an std::excetion, what=" << ex.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Got an unknown exception" << std::endl;
        return false;
    }
    return true;
}

bool E57FileReader::readPoints(PointXYZIRGB* dstBuf, uint64_t bufSize, uint64_t &readCount)
{
    if (m_cvReaderWrapper == nullptr || bufSize == 0)
        return 0;

    // Read from the file
    uint64_t N = m_cvReaderWrapper->cvr.read();
    if (N > bufSize) {
        std::cerr << "Error: too much points red from the e57 file, destination buffer too small." << std::endl;
        N = bufSize;
    }

    E57AttribFormat& format = m_cvReaderWrapper->format;
    Limits& limits = m_cvReaderWrapper->limits;
    const bool applyLocalOffset = usesLocalOffsets(m_currentScanIndex);
    const glm::dvec3 localOffset = applyLocalOffset ? m_localOffsets[m_currentScanIndex] : glm::dvec3(0.0);

    // Check if the read is finished
    if (N == 0)
    {
        delete m_cvReaderWrapper;
        m_cvReaderWrapper = nullptr;
        return false;
    }

    // Insert the points in the PointXYZIRGB buffer
    if (format.state == TL_STATE_NONE)
    {
        if (format.coordinates == TL_COORD_CARTESIAN_FLOAT ||
            format.coordinates == TL_COORD_CARTESIAN_SCALED_INTEGER)
        {
            for (unsigned int n = 0; n < N; n++)
            {
                if (applyLocalOffset)
                {
                    dstBuf[n].x = static_cast<float>(m_stagingBuffers.cartesianX[n] - localOffset.x);
                    dstBuf[n].y = static_cast<float>(m_stagingBuffers.cartesianY[n] - localOffset.y);
                    dstBuf[n].z = static_cast<float>(m_stagingBuffers.cartesianZ[n] - localOffset.z);
                }
                else
                {
                    dstBuf[n].x = static_cast<float>(m_stagingBuffers.cartesianX[n]);
                    dstBuf[n].y = static_cast<float>(m_stagingBuffers.cartesianY[n]);
                    dstBuf[n].z = static_cast<float>(m_stagingBuffers.cartesianZ[n]);
                }
            }
        }
        else if (format.coordinates == TL_COORD_SPHERICAL_FLOAT)
        {
            for (unsigned int n = 0; n < N; n++)
            {
                double r = m_stagingBuffers.sphericalRange[n];
                double a = m_stagingBuffers.sphericalAzimuth[n];
                double e = m_stagingBuffers.sphericalElevation[n];

                const double x = r * cos(a) * cos(e);
                const double y = r * sin(a) * cos(e);
                const double z = r * sin(e);
                if (applyLocalOffset)
                {
                    dstBuf[n].x = static_cast<float>(x - localOffset.x);
                    dstBuf[n].y = static_cast<float>(y - localOffset.y);
                    dstBuf[n].z = static_cast<float>(z - localOffset.z);
                }
                else
                {
                    dstBuf[n].x = static_cast<float>(x);
                    dstBuf[n].y = static_cast<float>(y);
                    dstBuf[n].z = static_cast<float>(z);
                }
            }
        }

        if (format.color == TL_RGB_UINT8_FORMAT)
        {
            for (unsigned int n = 0; n < N; n++)
            {
                dstBuf[n].r = m_stagingBuffers.colorRed[n];
                dstBuf[n].g = m_stagingBuffers.colorGreen[n];
                dstBuf[n].b = m_stagingBuffers.colorBlue[n];
            }
        }
        else if (format.color == TL_RGB_FLOAT_FORMAT)
        {
            for (unsigned int n = 0; n < N; n++)
            {
                dstBuf[n].r = m_stagingBuffers.colorRed[n];
                dstBuf[n].g = m_stagingBuffers.colorGreen[n];
                dstBuf[n].b = m_stagingBuffers.colorBlue[n];
            }
        }

        if (format.intensity == TL_I_FLOAT_FORMAT || format.intensity == TL_I_SCALED_INTEGER_FORMAT)
        {
            for (unsigned int n = 0; n < N; n++)
            {
                dstBuf[n].i = (uint8_t)(m_stagingBuffers.fIntensity[n] * 255.f / limits.iMax);
            }
        }
        else if (format.intensity == TL_I_UINT8_FORMAT)
        {
            for (unsigned int n = 0; n < N; n++)
            {
                dstBuf[n].i = m_stagingBuffers.uIntensity[n];
            }
        }
        readCount = N;
    }
    else if (format.state == TL_STATE_PRESENT)
    {
        uint64_t validPoints = 0;
        for (unsigned int n = 0; n < N; n++)
        {
            if (m_stagingBuffers.state[n] != 0)
                continue;

            if (applyLocalOffset)
            {
                dstBuf[validPoints].x = static_cast<float>(m_stagingBuffers.cartesianX[n] - localOffset.x);
                dstBuf[validPoints].y = static_cast<float>(m_stagingBuffers.cartesianY[n] - localOffset.y);
                dstBuf[validPoints].z = static_cast<float>(m_stagingBuffers.cartesianZ[n] - localOffset.z);
            }
            else
            {
                dstBuf[validPoints].x = static_cast<float>(m_stagingBuffers.cartesianX[n]);
                dstBuf[validPoints].y = static_cast<float>(m_stagingBuffers.cartesianY[n]);
                dstBuf[validPoints].z = static_cast<float>(m_stagingBuffers.cartesianZ[n]);
            }

            if (format.color == TL_RGB_UINT8_FORMAT)
            {
                dstBuf[validPoints].r = m_stagingBuffers.colorRed[n];
                dstBuf[validPoints].g = m_stagingBuffers.colorGreen[n];
                dstBuf[validPoints].b = m_stagingBuffers.colorBlue[n];
            }
            else if (format.color == TL_RGB_FLOAT_FORMAT)
            {
                dstBuf[validPoints].r = m_stagingBuffers.colorRed[n];
                dstBuf[validPoints].g = m_stagingBuffers.colorGreen[n];
                dstBuf[validPoints].b = m_stagingBuffers.colorBlue[n];
            }

            if (format.intensity == TL_I_FLOAT_FORMAT || format.intensity == TL_I_SCALED_INTEGER_FORMAT)
            {
                dstBuf[validPoints].i = (uint8_t)(m_stagingBuffers.fIntensity[n] * 255.f / limits.iMax);
            }
            else if (format.intensity == TL_I_UINT8_FORMAT)
            {
                dstBuf[validPoints].i = m_stagingBuffers.uIntensity[n];
            }

            ++validPoints;
        }
        readCount = validPoints;
    }

    return true;
}
