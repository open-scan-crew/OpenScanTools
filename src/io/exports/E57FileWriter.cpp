#include "io/exports/E57FileWriter.h"
#include "models/pointCloud/PointXYZIRGB.h"
#include "utils/time.h"
#include "utils/math/trigo.h"
#include "utils/Utils.h"

E57FileWriter::E57FileWriter(const std::filesystem::path& filepath, e57::ImageFile imf)
    : IScanFileWriter(filepath)
    , m_imf(imf)
    , m_storedWriter(nullptr)
{
}

E57FileWriter::~E57FileWriter()
{
    try {
        if (m_storedWriter)
            delete m_storedWriter;

        if (m_totalPointCount == 0)
            std::filesystem::remove(m_filepath);

        m_imf.close();
    }
    catch (e57::E57Exception& ex) {
        ex.report(__FILE__, __LINE__, __FUNCTION__);
    }
    catch (std::exception& ex) {
        std::cerr << "Got an std::excetion, what=" << ex.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Got an unknown exception" << std::endl;
    }
}

bool E57FileWriter::getWriter(const std::filesystem::path& dirPath, const std::wstring& fileName, std::wstring& log, IScanFileWriter** writer)
{
    try {
        // Read the file from disk
        std::filesystem::path completePath = dirPath / fileName;
        completePath += ".e57";
        e57::ImageFile imf(completePath.string(), "w");

        // If we are here, we have created a valid e57 file ready for writing
        *writer = new E57FileWriter(completePath, imf);

        static_cast<E57FileWriter*>(*writer)->setHeader();
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

bool E57FileWriter::setHeader()
{
    try {
        e57::StructureNode root = m_imf.root();

        root.set("formatName", e57::StringNode(m_imf, "ASTM E57 3D Imaging Data File"));
        root.set("guid", e57::StringNode(m_imf, xg::newGuid().str()));

        // Get the version of this library
        int astmMajor = 1;
        int astmMinor = 0;
        e57::ustring libraryId = "OpenScanTools_E57RefImpl-1.1.312"; // TODO - recompiler la librairie e57 avec la bonne valeur pour E57_LIBRARY_ID
        //e57::E57Utilities().getVersions(astmMajor, astmMinor, libraryId);
        root.set("versionMajor", e57::IntegerNode(m_imf, astmMajor));
        root.set("versionMinor", e57::IntegerNode(m_imf, astmMinor));
        root.set("e57LibraryVersion", e57::StringNode(m_imf, libraryId));

        // Get the creation time in GPS time
        auto time_now = std::chrono::system_clock::now();
        std::time_t utcTime = std::chrono::system_clock::to_time_t(time_now);
        double gpsTime = scs::time_utc_to_gps(utcTime);
        // Set the "creationDateTime" node
        e57::StructureNode creationDateTime(m_imf);
        creationDateTime.set("dateTimeValue", e57::FloatNode(m_imf, gpsTime)); // required
        creationDateTime.set("isAtomicClockReferenced", e57::IntegerNode(m_imf, 0)); // optional
        root.set("creationDateTime", creationDateTime);

        // Create the vector node "data3D" that will contain the point clouds
        e57::VectorNode data3D(m_imf, true);
        root.set("data3D", data3D);

        // "image2D" Optional and not in use. But some software (Faro Scene, Sketchup) seems very fond of it...
        e57::VectorNode images2D(m_imf, false);
        root.set("images2D", images2D);

        // "coordinateMetadata" is optional but we tend to be cautious with the optional fields
        e57::StringNode coordinateStr(m_imf, "");
        root.set("coordinateMetadata", coordinateStr);
    }
    catch (e57::E57Exception& ex) {
        ex.report(__FILE__, __LINE__, __FUNCTION__, std::cerr);
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

FileType E57FileWriter::getType() const
{
    return FileType::E57;
}

bool E57FileWriter::appendPointCloud(const tls::ScanHeader& info, const TransformationModule& transfo)
{
    // We can store only one writer at a time
    if (m_storedWriter)
    {
        delete m_storedWriter;
        m_storedWriter = nullptr;
    }

    m_scanHeader = info;
    scan_transfo = transfo;
    m_scanPointCount = 0;

    e57::StructureNode root = m_imf.root();

    e57::VectorNode data3D(root.get("/data3D"));

    // Add a new Scanto the "data3D"
    e57::StructureNode pointCloud(m_imf);
    data3D.append(pointCloud);

    // Add guid [required]
    // Generate a new Guid for this point cloud
    xg::Guid newGuid = xg::Guid();
    pointCloud.set("guid", e57::StringNode(m_imf, newGuid.str()));

    // Add name [optional]
    pointCloud.set("name", e57::StringNode(m_imf, Utils::to_utf8(m_scanHeader.name)));

    // Add description [optional]

    // Construct the prototype
    e57::StructureNode proto(m_imf);
    // TODO - set the maximum and minimum for the cartesian coordinates based on the encoding precision
    // i.e. for 0.12mm -> max = 2048 (0x45000000)
    proto.set("cartesianX", e57::FloatNode(m_imf, 0, e57::E57_SINGLE, -2048.f, 2048.f));
    proto.set("cartesianY", e57::FloatNode(m_imf, 0, e57::E57_SINGLE, -2048.f, 2048.f));
    proto.set("cartesianZ", e57::FloatNode(m_imf, 0, e57::E57_SINGLE, -2048.f, 2048.f));
    if (m_scanHeader.format == tls::TL_POINT_XYZ_I || m_scanHeader.format == tls::TL_POINT_XYZ_I_RGB)
    {
        proto.set("intensity", e57::IntegerNode(m_imf, 0, 0, 255));
    }
    if (m_scanHeader.format == tls::TL_POINT_XYZ_RGB || m_scanHeader.format == tls::TL_POINT_XYZ_I_RGB)
    {
        proto.set("colorRed", e57::IntegerNode(m_imf, 0, 0, 255));
        proto.set("colorGreen", e57::IntegerNode(m_imf, 0, 0, 255));
        proto.set("colorBlue", e57::IntegerNode(m_imf, 0, 0, 255));
    }

    // Make empty codecs vector for use in creating points CompressedVector.
    e57::VectorNode codecs(m_imf, true);

    // Add "intensityLimits" if available in format
    if (m_scanHeader.format == tls::TL_POINT_XYZ_I || m_scanHeader.format == tls::TL_POINT_XYZ_I_RGB)
    {
        e57::StructureNode intensityLimits(m_imf);
        pointCloud.set("intensityLimits", intensityLimits);
        intensityLimits.set("intensityMinimum", e57::IntegerNode(m_imf, 0));
        intensityLimits.set("intensityMaximum", e57::IntegerNode(m_imf, 255));
    }

    // Add "colorLimits" if available in format
    if (m_scanHeader.format == tls::TL_POINT_XYZ_RGB || m_scanHeader.format == tls::TL_POINT_XYZ_I_RGB)
    {
        e57::StructureNode colorLimits(m_imf);
        pointCloud.set("colorLimits", colorLimits);
        colorLimits.set("colorRedMinimum", e57::IntegerNode(m_imf, 0));
        colorLimits.set("colorRedMaximum", e57::IntegerNode(m_imf, 255));
        colorLimits.set("colorGreenMinimum", e57::IntegerNode(m_imf, 0));
        colorLimits.set("colorGreenMaximum", e57::IntegerNode(m_imf, 255));
        colorLimits.set("colorBlueMinimum", e57::IntegerNode(m_imf, 0));
        colorLimits.set("colorBlueMaximum", e57::IntegerNode(m_imf, 255));
    }

    e57::StructureNode bboxNode(m_imf);
    pointCloud.set("cartesianBounds", bboxNode);
    m_bbox = m_scanHeader.bbox;

    // Create "pose" structure to record the transformation
    e57::StructureNode pose(m_imf);
    pointCloud.set("pose", pose);
    e57::StructureNode rotation(m_imf);
    pose.set("rotation", rotation);
    rotation.set("w", e57::FloatNode(m_imf, scan_transfo.getOrientation().w));
    rotation.set("x", e57::FloatNode(m_imf, scan_transfo.getOrientation().x));
    rotation.set("y", e57::FloatNode(m_imf, scan_transfo.getOrientation().y));
    rotation.set("z", e57::FloatNode(m_imf, scan_transfo.getOrientation().z));
    e57::StructureNode translation(m_imf);
    pose.set("translation", translation);
    translation.set("x", e57::FloatNode(m_imf, scan_transfo.getCenter().x));
    translation.set("y", e57::FloatNode(m_imf, scan_transfo.getCenter().y));
    translation.set("z", e57::FloatNode(m_imf, scan_transfo.getCenter().z));

    // No point grouping scheme

    // Add "points"
    e57::CompressedVectorNode points(m_imf, proto, codecs);
    pointCloud.set("points", points);

    E57AttribFormat e57Format{ TL_COORD_CARTESIAN_FLOAT,
        TL_RGB_UINT8_FORMAT,
        TL_I_UINT8_FORMAT,
        TL_STATE_NONE };
    m_stagingBufs.resize(1048576u, e57Format);

    // Construct the source buffers
    std::vector<e57::SourceDestBuffer> sourceBuffers;

    // NOTE(robin) - for the coordinates we can choose to use a scaleInteger and use a uintXX_t buffer
    // as input. We must then specify to the SourceDestBuffer (doConversion = false, doScaling = false)
    sourceBuffers.push_back(e57::SourceDestBuffer(m_imf, "cartesianX", m_stagingBufs.cartesianX, m_stagingBufs.size));
    sourceBuffers.push_back(e57::SourceDestBuffer(m_imf, "cartesianY", m_stagingBufs.cartesianY, m_stagingBufs.size));
    sourceBuffers.push_back(e57::SourceDestBuffer(m_imf, "cartesianZ", m_stagingBufs.cartesianZ, m_stagingBufs.size));
    if (proto.isDefined("intensity"))
    {
        sourceBuffers.push_back(e57::SourceDestBuffer(m_imf, "intensity", m_stagingBufs.uIntensity, m_stagingBufs.size));
    }
    if (proto.isDefined("colorRed"))
    {
        sourceBuffers.push_back(e57::SourceDestBuffer(m_imf, "colorRed", m_stagingBufs.colorRed, m_stagingBufs.size));
        sourceBuffers.push_back(e57::SourceDestBuffer(m_imf, "colorGreen", m_stagingBufs.colorGreen, m_stagingBufs.size));
        sourceBuffers.push_back(e57::SourceDestBuffer(m_imf, "colorBlue", m_stagingBufs.colorBlue, m_stagingBufs.size));
    }

    // Create the writer from the sourceBuffers and the "points" node
    m_storedWriter = new CVWriterWrapper{ points.writer(sourceBuffers), m_scanHeader.format };

    return true;
}

bool E57FileWriter::addPoints(PointXYZIRGB const* srcBuf, uint64_t srcSize)
{
    if (m_storedWriter == nullptr)
            return false;
    try
    {
        uint64_t srcOffset = 0;
        while (srcOffset < srcSize)
        {
            uint64_t writeSize = std::min(m_stagingBufs.size, srcSize - srcOffset);
            for (uint64_t n = 0; n < writeSize; ++n)
            {
                m_stagingBufs.cartesianX[n] = srcBuf[srcOffset + n].x;
                m_stagingBufs.cartesianY[n] = srcBuf[srcOffset + n].y;
                m_stagingBufs.cartesianZ[n] = srcBuf[srcOffset + n].z;
                m_stagingBufs.uIntensity[n] = srcBuf[srcOffset + n].i;
                m_stagingBufs.colorRed[n] = srcBuf[srcOffset + n].r;
                m_stagingBufs.colorGreen[n] = srcBuf[srcOffset + n].g;
                m_stagingBufs.colorBlue[n] = srcBuf[srcOffset + n].b;
            }

            m_storedWriter->cvw.write(writeSize);
            srcOffset += writeSize;
        }
        m_scanPointCount += srcSize;
    }
    catch (e57::E57Exception& ex) {
        ex.report(__FILE__, __LINE__, __FUNCTION__, std::cerr);
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

bool E57FileWriter::mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const TransformationModule& src_transfo, tls::PointFormat src_format)
{
    if (m_storedWriter == nullptr)
        return false;

    if (scan_transfo == src_transfo)
    {
        // check point format ??
        addPoints(srcBuf, srcSize);
    }
    else
    {
        glm::dmat4 total_transfo = scan_transfo.getInverseTransformation() * src_transfo.getTransformation();

        typedef PointXYZIRGB(*convert_fn_t)(const PointXYZIRGB&, const glm::dmat4&);
        convert_fn_t convert_fn = convert_transfo;

        if (src_format == m_scanHeader.format)
        {
            convert_fn = convert_transfo;
        }
        else if (src_format == tls::PointFormat::TL_POINT_XYZ_RGB)
        {
            convert_fn = convert_RGB_to_I_transfo;
        }
        else if (src_format == tls::PointFormat::TL_POINT_XYZ_I)
        {
            convert_fn = convert_I_to_RGB_transfo;
        }
        else
            return false;

        uint64_t srcOffset = 0;
        while (srcOffset < srcSize)
        {
            uint64_t writeSize = std::min(m_stagingBufs.size, srcSize - srcOffset);
            for (uint64_t n = 0; n < writeSize; ++n)
            {
                PointXYZIRGB newPoint = convert_fn(srcBuf[srcOffset + n], total_transfo);

                m_stagingBufs.cartesianX[n] = newPoint.x;
                m_stagingBufs.cartesianY[n] = newPoint.y;
                m_stagingBufs.cartesianZ[n] = newPoint.z;
                m_stagingBufs.uIntensity[n] = newPoint.i;
                m_stagingBufs.colorRed[n] = newPoint.r;
                m_stagingBufs.colorGreen[n] = newPoint.g;
                m_stagingBufs.colorBlue[n] = newPoint.b;

                updateBoundingBox(newPoint);
            }

            m_storedWriter->cvw.write(writeSize);
            srcOffset += writeSize;
        }
    }
    m_scanPointCount += srcSize;
    return true;
}

void E57FileWriter::addTranslation(const glm::dvec3& t)
{
    e57::StructureNode root = m_imf.root();
    e57::VectorNode data3D(root.get("/data3D"));
    e57::StructureNode pointCloud(data3D.get(data3D.childCount() - 1));
    e57::StructureNode translation_node(pointCloud.get("pose/translation"));
    translation_node.set("x", e57::FloatNode(m_imf, scan_transfo.getCenter().x + t.x));
    translation_node.set("y", e57::FloatNode(m_imf, scan_transfo.getCenter().y + t.y));
    translation_node.set("z", e57::FloatNode(m_imf, scan_transfo.getCenter().z + t.z));
}

void E57FileWriter::updateBoundingBox(const PointXYZIRGB& point)
{
    if (point.x < m_bbox.xMin)
        m_bbox.xMin = point.x;
    if (point.x > m_bbox.xMax)
        m_bbox.xMax = point.x;
    if (point.y < m_bbox.yMin)
        m_bbox.yMin = point.y;
    if (point.y > m_bbox.yMax)
        m_bbox.yMax = point.y;
    if (point.z < m_bbox.zMin)
        m_bbox.zMin = point.z;
    if (point.z > m_bbox.zMax)
        m_bbox.zMax = point.z;
}

bool E57FileWriter::flushWrite()
{
    try
    {
        // Add "cartesianBounds"
        e57::VectorNode data3D(m_imf.root().get("/data3D"));
        e57::StructureNode pointCloud(data3D.get(data3D.childCount() - 1));
        e57::StructureNode bbox(pointCloud.get("cartesianBounds"));
        bbox.set("xMinimum", e57::FloatNode(m_imf, m_bbox.xMin));
        bbox.set("xMaximum", e57::FloatNode(m_imf, m_bbox.xMax));
        bbox.set("yMinimum", e57::FloatNode(m_imf, m_bbox.yMin));
        bbox.set("yMaximum", e57::FloatNode(m_imf, m_bbox.yMax));
        bbox.set("zMinimum", e57::FloatNode(m_imf, m_bbox.zMin));
        bbox.set("zMaximum", e57::FloatNode(m_imf, m_bbox.zMax));

        m_totalPointCount += m_scanPointCount;
        if (m_storedWriter != nullptr)
        {
            if (m_storedWriter->cvw.isOpen())
                m_storedWriter->cvw.close();
            delete m_storedWriter;
            m_storedWriter = nullptr;
        }
        return true;
    }
    catch (e57::E57Exception& ex) {
        ex.report(__FILE__, __LINE__, __FUNCTION__, std::cerr);
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
}