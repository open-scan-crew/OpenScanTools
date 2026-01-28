#include "pointCloudEngine/EmbeddedScan.h"
#include "pointCloudEngine/OctreeRayTracing.h"
#include "vulkan/VulkanManager.h"
#include "utils/Logger.h"
#include "models/graph/TransformationModule.h"

#include "tls_impl.h"

#include <algorithm>
#include <cmath>
#include <chrono>
#include <future>
#include <set>
#include <unordered_map>
#ifndef PORTABLE
#include "io/exports/IScanFileWriter.h"
#endif

float EmbeddedScan::pointer_time_ = 0.f;
float EmbeddedScan::decode_time_ = 0.f;
float EmbeddedScan::merge_time_ = 0.f;

using namespace std::chrono;

EmbeddedScan::EmbeddedScan(std::filesystem::path const& filepath)
    : pt_format_(tls::PointFormat::TL_POINT_FORMAT_UNDEFINED)
    , pt_precision_(tls::PrecisionType::TL_PRECISION_UNDEFINED)
    , m_deleteFileWhenDestroyed(false)
    , m_lastFrameUse(0)
    , m_pCellBuffers(nullptr)
{
    // open tls file
    if (!tls_img_file_.open(filepath, tls::usage::read))
    {
        Logger::log(IOLog) << "An error occured while opening the TLS file '" << filepath << "'" << Logger::endl;
        return;
    }

    tls_point_cloud_ = tls_img_file_.getImagePointCloud(0);

    if(!tls_point_cloud_.getOctreeBase(*(tls::OctreeBase*)this))
    {
        Logger::log(IOLog) << "Failed to read the octree part in '" << tls_img_file_.getPath() << "'" << Logger::endl;
        return;
    }

    tls::ScanHeader infos = tls_img_file_.getPointCloudHeader(0);
    pt_format_ = infos.format;
    pt_precision_ = infos.precision;

    // Initialize the buffers
    m_pCellBuffers = new SmartBuffer[m_cellCount];
    memset(m_pCellBuffers, 0, m_cellCount * sizeof(SmartBuffer));

    // Load the instance data
    size_t data_size = 0;
    // get the data size by passing a null buffer
    tls_point_cloud_.getCellRenderData(nullptr, data_size);
    if (data_size == 0)
        return;
    char* data_buffer = new char[data_size];
    tls_point_cloud_.getCellRenderData(data_buffer, data_size);

    VulkanManager& vkManager = VulkanManager::getInstance();
    vkManager.allocSimpleBuffer(data_size, m_instanceBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize offset = 0;
    vkManager.loadInSimpleBuffer(m_instanceBuffer, data_size, data_buffer, offset, 4);

    delete[] data_buffer;
}

EmbeddedScan::~EmbeddedScan()
{
    checkDataState();

    VulkanManager& vkManager = VulkanManager::getInstance();

    if (m_instanceBuffer.alloc != nullptr)
        vkManager.freeAllocation(m_instanceBuffer);

    for (uint32_t i = 0; i < m_cellCount; ++i)
    {
        if (m_pCellBuffers[i].alloc != nullptr)
            vkManager.freeAllocation(m_pCellBuffers[i]);
    }
    delete[] m_pCellBuffers;

    if (m_deleteFileWhenDestroyed)
    {
        std::filesystem::path filepath = tls_img_file_.getPath();
        tls_img_file_.close();

        if (std::filesystem::remove(filepath) == true)
        {
            Logger::log(IOLog) << "INFO - the file " << filepath << " has been successfully removed." << Logger::endl;
        }
        else
        {
            Logger::log(IOLog) << "WARNING - the file " << filepath << " cannot be removed from the file system. The file may be accessed elsewhere." << Logger::endl;
        }
    }
}

tls::ScanGuid EmbeddedScan::getGuid() const
{
    return tls_img_file_.getPointCloudHeader(0).guid;
}

tls::FileGuid EmbeddedScan::getFileGuid() const
{
    return tls_img_file_.getFileHeader().guid;
}

void EmbeddedScan::getInfo(tls::ScanHeader &info) const
{
    info = tls_img_file_.getPointCloudHeader(0);
}

std::filesystem::path EmbeddedScan::getPath() const
{
    return tls_img_file_.getPath();
}

void EmbeddedScan::setPath(const std::filesystem::path& newPath)
{
    tls_point_cloud_.reset();
    tls_img_file_.close();
    if (!tls_img_file_.open(newPath, tls::usage::read))
        assert(0);
    tls_point_cloud_ = tls_img_file_.getImagePointCloud(0);
}

bool EmbeddedScan::getGlobalDrawInfo(TlScanDrawInfo& scanDrawInfo)
{
    m_lastFrameUse.store(VulkanManager::getInstance().getCurrentFrameIndex());
    if (m_instanceBuffer.buffer == VK_NULL_HANDLE)
        return false;

    scanDrawInfo.instanceBuffer = m_instanceBuffer.buffer;
    scanDrawInfo.format = pt_format_;
    return true;
}

// return:
// - false if the drawing is incomplete due to missing cells.
// - true otherwise
bool EmbeddedScan::viewOctree(std::vector<TlCellDrawInfo>& _cellDrawInfo, const TlProjectionInfo& _projInfo)
{
    // Init the tests of the sides to false
    TlFrustumTest frustumTest;
    memset(&frustumTest.test, 0, sizeof(TestInside));
    frustumTest.cube = HCube(_projInfo.frustumMat * _projInfo.modelMat, m_rootPosition, m_rootSize);

    std::vector<uint32_t> missingCells;
    getVisibleTree_impl(m_uRootCell, _cellDrawInfo, _projInfo, frustumTest, missingCells);

    // Store the missing cells for the streaming thread-safely
    std::lock_guard<std::mutex> lock(m_streamMutex);
    m_missingCells = missingCells;

    return (missingCells.size() == 0);
}

// return:
// - false if the drawing is incomplete due to missing cells.
// - true otherwise
bool EmbeddedScan::viewOctreeCB(std::vector<TlCellDrawInfo>& _cellDrawInfo, std::vector<TlCellDrawInfo_multiCB>& _cellDrawInfoCB, const TlProjectionInfo& _projInfo, const ClippingAssembly& _clippingAssembly)
{
    // Init the tests of the sides to false
    TlFrustumTest frustumTest;
    memset(&frustumTest.test, 0, sizeof(TestInside));
    frustumTest.cube = HCube(_projInfo.frustumMat * _projInfo.modelMat, m_rootPosition, m_rootSize);

    ClippingAssembly localAssembly = _clippingAssembly;
    localAssembly.clearMatrix();
    localAssembly.addTransformation(_projInfo.modelMat);

    // Start the recursive treatment
    std::vector<uint32_t> missingCells;
    getVisibleTreeMultiClip_impl(m_uRootCell, _cellDrawInfo, _cellDrawInfoCB, _projInfo, frustumTest, localAssembly, missingCells);

    // Store the missing cells for the streaming thread-safely
    std::lock_guard<std::mutex> lock(m_streamMutex);
    m_missingCells = missingCells;

    return (missingCells.size() == 0);
}

void clipIndividualPoints(const std::vector<PointXYZIRGB>& inPoints, std::vector<PointXYZIRGB>& outPoints, const ClippingAssembly& clippingAssembly)
{
    for (const PointXYZIRGB& point : inPoints)
    {
        glm::dvec4 pos = { point.x, point.y, point.z, 1.0 };
        if (clippingAssembly.testPoint(pos))
            outPoints.push_back(point);
    }
}

bool EmbeddedScan::testPointsClippedOut(const TransformationModule& src_transfo, const ClippingAssembly& _clippingAssembly) const
{
    ClippingAssembly localAssembly = _clippingAssembly;
    localAssembly.clearMatrix();
    glm::dmat4 src_transfo_mat = src_transfo.getTransformation();
    localAssembly.addTransformation(src_transfo_mat);

    std::vector<uint32_t> partially_clipped_cells;
    if (testFullyClippedOutCells(m_uRootCell, localAssembly, partially_clipped_cells))
        return true;

    bool points_clipped_out = false;
    for (uint32_t cell : partially_clipped_cells)
    {
        std::vector<PointXYZIRGB> points;
        points.resize(tls_point_cloud_.getCellPointCount(cell));
        if (!tls_point_cloud_.getCellPoints(cell, reinterpret_cast<tls::Point*>(points.data()), points.size()))
            continue;
        std::vector<PointXYZIRGB> points_clipped_in;
        clipIndividualPoints(points, points_clipped_in, localAssembly);
        if (points_clipped_in.size() < points.size())
            return true;
    }
    return false;
}

bool EmbeddedScan::clipAndWrite(const TransformationModule& src_transfo, const ClippingAssembly& _clippingAssembly, IScanFileWriter* _writer, const ProgressCallback& progress)
{
    ClippingAssembly localAssembly = _clippingAssembly;
    localAssembly.clearMatrix();
    glm::dmat4 src_transfo_mat = src_transfo.getTransformation();
    localAssembly.addTransformation(src_transfo_mat);

    // Do the clipping in 3 phases:
    //   a. Determine the cells inside the clipping (whole and partial)
    //   b. Read the points from the hdd
    //   c. Insert them in the file writer.

    // Store the TreeCell to export and a boolean that indicate if micro-clipping must be done.
    std::vector<std::pair<uint32_t, bool>> cells;
    getClippedCells_impl(m_uRootCell, localAssembly, cells);

    bool resultOk = true;
    const size_t totalCells = cells.size();
    if (progress && totalCells > 0)
        progress(0, totalCells);
    for (size_t cellIndex = 0; cellIndex < cells.size(); ++cellIndex)
    {
        const std::pair<uint32_t, bool>& cell = cells[cellIndex];
        // NOTE(robin) - A propos des référenciels :
        //   (*) Il est préférable de travailler dans l'espace local du scan en cours.
        //   (*) Les matrices de transformation des clippings peuvent être changées une fois pour tout les points.
        //   (*) Si on a un export simple de point, on ne doit pas transformer les coord lors du addPoints.
        //   (*) Si l'export est une fusion, la matrice de transfo doit être fournie à mergePoints().
        std::chrono::steady_clock::time_point tp_0 = std::chrono::steady_clock::now();
        std::vector<PointXYZIRGB> points;
        points.resize(tls_point_cloud_.getCellPointCount(cell.first));
        if (!tls_point_cloud_.getCellPoints(cell.first, reinterpret_cast<tls::Point*>(points.data()), points.size()))
        {
            if (progress)
                progress(cellIndex + 1, totalCells);
            continue;
        }
        std::chrono::steady_clock::time_point tp_1 = std::chrono::steady_clock::now();

        // Do the micro-clipping if necessary
        if (cell.second)
        {
            std::vector<PointXYZIRGB> pointsClipped;
            clipIndividualPoints(points, pointsClipped, localAssembly);
            resultOk &= _writer->mergePoints(pointsClipped.data(), pointsClipped.size(), src_transfo, pt_format_);
        }
        else
        {
            resultOk &= _writer->mergePoints(points.data(), points.size(), src_transfo, pt_format_);
        }
        std::chrono::steady_clock::time_point tp_2 = std::chrono::steady_clock::now();
        decode_time_ += std::chrono::duration<float, std::ratio<1>>(tp_1 - tp_0).count();
        merge_time_ += std::chrono::duration<float, std::ratio<1>>(tp_2 - tp_1).count();

        if (progress)
            progress(cellIndex + 1, totalCells);
    }
    return (resultOk);
}

namespace
{
    struct RunningStats
    {
        uint64_t count = 0;
        double mean = 0.0;
        double m2 = 0.0;

        void add(double value)
        {
            count++;
            double delta = value - mean;
            mean += delta / static_cast<double>(count);
            double delta2 = value - mean;
            m2 += delta * delta2;
        }

        double stddev() const
        {
            if (count < 2)
                return 0.0;
            double variance = m2 / static_cast<double>(count - 1);
            return sqrt(variance);
        }
    };

    struct GridIndex
    {
        int x;
        int y;
        int z;
    };

    int64_t packGridIndex(const GridIndex& index)
    {
        return (static_cast<int64_t>(index.x) << 42) ^ (static_cast<int64_t>(index.y) << 21) ^ static_cast<int64_t>(index.z);
    }

    GridIndex computeGridIndex(const glm::dvec3& point, const glm::dvec3& origin, double voxelSize)
    {
        return GridIndex{
            static_cast<int>(std::floor((point.x - origin.x) / voxelSize)),
            static_cast<int>(std::floor((point.y - origin.y) / voxelSize)),
            static_cast<int>(std::floor((point.z - origin.z) / voxelSize))
        };
    }

    bool computeMeanNeighborDistanceGrid(const std::vector<glm::dvec3>& points, const std::unordered_map<int64_t, std::vector<size_t>>& grid, const glm::dvec3& origin, double voxelSize, size_t index, size_t kNeighbors, double maxRadius, double& meanDistance)
    {
        if (kNeighbors == 0 || points.empty())
            return false;

        const glm::dvec3& base = points[index];
        GridIndex baseIndex = computeGridIndex(base, origin, voxelSize);
        int ring = 0;
        std::vector<size_t> candidateIndices;

        while (ring * voxelSize <= maxRadius)
        {
            candidateIndices.clear();
            for (int dx = -ring; dx <= ring; ++dx)
            {
                for (int dy = -ring; dy <= ring; ++dy)
                {
                    for (int dz = -ring; dz <= ring; ++dz)
                    {
                        GridIndex query{ baseIndex.x + dx, baseIndex.y + dy, baseIndex.z + dz };
                        int64_t key = packGridIndex(query);
                        auto it = grid.find(key);
                        if (it == grid.end())
                            continue;
                        candidateIndices.insert(candidateIndices.end(), it->second.begin(), it->second.end());
                    }
                }
            }

            if (candidateIndices.size() > kNeighbors)
                break;
            ring++;
        }

        if (candidateIndices.empty())
            return false;

        std::vector<double> distances;
        distances.reserve(candidateIndices.size());
        for (size_t candidate : candidateIndices)
        {
            if (candidate == index)
                continue;
            double dist = glm::length(base - points[candidate]);
            if (dist > 0.0)
                distances.push_back(dist);
        }

        if (distances.empty())
            return false;

        size_t target = std::min(kNeighbors, distances.size());
        std::nth_element(distances.begin(), distances.begin() + static_cast<long>(target - 1), distances.end());
        distances.resize(target);

        double sum = 0.0;
        for (double value : distances)
            sum += value;
        meanDistance = sum / static_cast<double>(distances.size());
        return true;
    }
}

bool EmbeddedScan::computeOutlierStats(const TransformationModule& src_transfo, const ClippingAssembly& clippingAssembly, int kNeighbors, int samplingPercent, double beta, OutlierStats& stats, const ProgressCallback& progress)
{
    ClippingAssembly localAssembly = clippingAssembly;
    localAssembly.clearMatrix();
    glm::dmat4 src_transfo_mat = src_transfo.getTransformation();
    localAssembly.addTransformation(src_transfo_mat);

    std::vector<std::pair<uint32_t, bool>> cells;
    getClippedCells_impl(m_uRootCell, localAssembly, cells);

    RunningStats running;
    const size_t neighborCount = std::max(1, kNeighbors);
    const double samplingValue = std::clamp(static_cast<double>(samplingPercent), 1.0, 100.0);
    const size_t sampleStep = std::max<size_t>(1, static_cast<size_t>(std::round(100.0 / samplingValue)));

    const size_t totalCells = cells.size();
    if (progress && totalCells > 0)
        progress(0, totalCells);
    for (size_t cellIndex = 0; cellIndex < cells.size(); ++cellIndex)
    {
        const std::pair<uint32_t, bool>& cell = cells[cellIndex];
        std::vector<PointXYZIRGB> points;
        points.resize(tls_point_cloud_.getCellPointCount(cell.first));
        if (!tls_point_cloud_.getCellPoints(cell.first, reinterpret_cast<tls::Point*>(points.data()), points.size()))
        {
            if (progress)
                progress(cellIndex + 1, totalCells);
            continue;
        }

        std::vector<PointXYZIRGB> visiblePoints;
        if (cell.second)
        {
            clipIndividualPoints(points, visiblePoints, localAssembly);
        }
        else
        {
            visiblePoints.swap(points);
        }

        if (visiblePoints.empty())
        {
            if (progress)
                progress(cellIndex + 1, totalCells);
            continue;
        }

        glm::dvec3 cellOrigin(m_vTreeCells[cell.first].m_position[0], m_vTreeCells[cell.first].m_position[1], m_vTreeCells[cell.first].m_position[2]);
        double cellSize = m_vTreeCells[cell.first].m_size;
        double avgSpacing = std::cbrt((cellSize * cellSize * cellSize) / std::max<size_t>(visiblePoints.size(), 1));
        double voxelSize = std::max(cellSize / 128.0, avgSpacing * 2.0);
        double maxRadius = std::clamp(beta * avgSpacing, cellSize / 8.0, cellSize);

        std::vector<glm::dvec3> localPoints;
        localPoints.reserve(visiblePoints.size());
        for (const PointXYZIRGB& point : visiblePoints)
            localPoints.emplace_back(point.x, point.y, point.z);

        std::unordered_map<int64_t, std::vector<size_t>> grid;
        grid.reserve(localPoints.size());
        for (size_t i = 0; i < localPoints.size(); ++i)
        {
            GridIndex index = computeGridIndex(localPoints[i], cellOrigin, voxelSize);
            grid[packGridIndex(index)].push_back(i);
        }

        for (size_t i = 0; i < localPoints.size(); i += sampleStep)
        {
            double meanDistance = 0.0;
            if (computeMeanNeighborDistanceGrid(localPoints, grid, cellOrigin, voxelSize, i, neighborCount, maxRadius, meanDistance))
                running.add(meanDistance);
        }

        if (progress)
            progress(cellIndex + 1, totalCells);
    }

    stats.count = running.count;
    stats.mean = running.mean;
    stats.stddev = running.stddev();

    return true;
}

bool EmbeddedScan::filterOutliersAndWrite(const TransformationModule& src_transfo, const ClippingAssembly& clippingAssembly, int kNeighbors, const OutlierStats& stats, double nSigma, double beta, IScanFileWriter* writer, uint64_t& removedPoints, const ProgressCallback& progress)
{
    ClippingAssembly localAssembly = clippingAssembly;
    localAssembly.clearMatrix();
    glm::dmat4 src_transfo_mat = src_transfo.getTransformation();
    localAssembly.addTransformation(src_transfo_mat);

    std::vector<std::pair<uint32_t, bool>> cells;
    getClippedCells_impl(m_uRootCell, localAssembly, cells);

    const size_t neighborCount = std::max(1, kNeighbors);
    double threshold = stats.mean + nSigma * stats.stddev;
    bool resultOk = true;
    removedPoints = 0;

    const size_t totalCells = cells.size();
    if (progress && totalCells > 0)
        progress(0, totalCells);
    for (size_t cellIndex = 0; cellIndex < cells.size(); ++cellIndex)
    {
        const std::pair<uint32_t, bool>& cell = cells[cellIndex];
        std::vector<PointXYZIRGB> points;
        points.resize(tls_point_cloud_.getCellPointCount(cell.first));
        if (!tls_point_cloud_.getCellPoints(cell.first, reinterpret_cast<tls::Point*>(points.data()), points.size()))
        {
            if (progress)
                progress(cellIndex + 1, totalCells);
            continue;
        }

        std::vector<PointXYZIRGB> visiblePoints;
        if (cell.second)
        {
            clipIndividualPoints(points, visiblePoints, localAssembly);
        }
        else
        {
            visiblePoints.swap(points);
        }

        std::vector<PointXYZIRGB> filtered;
        filtered.reserve(visiblePoints.size());

        if (visiblePoints.empty())
        {
            if (progress)
                progress(cellIndex + 1, totalCells);
            continue;
        }

        glm::dvec3 cellOrigin(m_vTreeCells[cell.first].m_position[0], m_vTreeCells[cell.first].m_position[1], m_vTreeCells[cell.first].m_position[2]);
        double cellSize = m_vTreeCells[cell.first].m_size;
        double avgSpacing = std::cbrt((cellSize * cellSize * cellSize) / std::max<size_t>(visiblePoints.size(), 1));
        double voxelSize = std::max(cellSize / 128.0, avgSpacing * 2.0);
        double maxRadius = std::clamp(beta * avgSpacing, cellSize / 8.0, cellSize);

        std::vector<glm::dvec3> localPoints;
        localPoints.reserve(visiblePoints.size());
        for (const PointXYZIRGB& point : visiblePoints)
            localPoints.emplace_back(point.x, point.y, point.z);

        std::unordered_map<int64_t, std::vector<size_t>> grid;
        grid.reserve(localPoints.size());
        for (size_t i = 0; i < localPoints.size(); ++i)
        {
            GridIndex index = computeGridIndex(localPoints[i], cellOrigin, voxelSize);
            grid[packGridIndex(index)].push_back(i);
        }

        for (size_t i = 0; i < localPoints.size(); ++i)
        {
            double meanDistance = 0.0;
            if (!computeMeanNeighborDistanceGrid(localPoints, grid, cellOrigin, voxelSize, i, neighborCount, maxRadius, meanDistance))
            {
                filtered.push_back(visiblePoints[i]);
                continue;
            }

            if (meanDistance <= threshold || stats.count == 0)
                filtered.push_back(visiblePoints[i]);
        }

        removedPoints += visiblePoints.size() - filtered.size();
        resultOk &= writer->mergePoints(filtered.data(), filtered.size(), src_transfo, pt_format_);

        if (progress)
            progress(cellIndex + 1, totalCells);
    }

    return resultOk;
}

namespace
{
    uint8_t clampToByte(double value)
    {
        if (value < 0.0)
            return 0;
        if (value > 255.0)
            return 255;
        return static_cast<uint8_t>(std::lround(value));
    }

    double computeLuma(const PointXYZIRGB& point)
    {
        return 0.2126 * static_cast<double>(point.r) +
               0.7152 * static_cast<double>(point.g) +
               0.0722 * static_cast<double>(point.b);
    }
}

bool EmbeddedScan::computeColorBalanceHistogram(const TransformationModule& src_transfo, const ClippingAssembly& clippingAssembly, double cellSize, bool useColor, bool useIntensity, ColorBalanceHistogramMap& histogram, const ProgressCallback& progress)
{
    if (!useColor && !useIntensity)
        return false;

    ClippingAssembly localAssembly = clippingAssembly;
    localAssembly.clearMatrix();
    glm::dmat4 src_transfo_mat = src_transfo.getTransformation();
    localAssembly.addTransformation(src_transfo_mat);

    std::vector<std::pair<uint32_t, bool>> cells;
    getClippedCells_impl(m_uRootCell, localAssembly, cells);

    const size_t totalCells = cells.size();
    if (progress && totalCells > 0)
        progress(0, totalCells);
    for (size_t cellIndex = 0; cellIndex < cells.size(); ++cellIndex)
    {
        const std::pair<uint32_t, bool>& cell = cells[cellIndex];
        std::vector<PointXYZIRGB> points;
        points.resize(tls_point_cloud_.getCellPointCount(cell.first));
        if (!tls_point_cloud_.getCellPoints(cell.first, reinterpret_cast<tls::Point*>(points.data()), points.size()))
        {
            if (progress)
                progress(cellIndex + 1, totalCells);
            continue;
        }

        std::vector<PointXYZIRGB> visiblePoints;
        if (cell.second)
        {
            clipIndividualPoints(points, visiblePoints, localAssembly);
        }
        else
        {
            visiblePoints.swap(points);
        }

        for (const PointXYZIRGB& point : visiblePoints)
        {
            glm::dvec4 globalPos = src_transfo_mat * glm::dvec4(point.x, point.y, point.z, 1.0);
            ColorBalanceCellKey key = colorBalanceCellKeyFromGlobal(glm::dvec3(globalPos), cellSize);
            ColorBalanceCellHistogram& cellHist = histogram[key];
            cellHist.count++;
            if (useColor)
            {
                uint8_t luma = clampToByte(computeLuma(point));
                cellHist.lumaHist[luma]++;
            }
            if (useIntensity)
            {
                cellHist.intensityHist[point.i]++;
            }
        }

        if (progress)
            progress(cellIndex + 1, totalCells);
    }

    return true;
}

bool EmbeddedScan::applyColorBalanceAndWrite(const TransformationModule& src_transfo, const ClippingAssembly& clippingAssembly, const std::vector<ColorBalanceCorrectionMap>& correctionLevels, double baseCellSize, double beta, bool applyColor, bool applyIntensity, IScanFileWriter* writer, const ProgressCallback& progress)
{
    if (correctionLevels.empty())
        return false;

    ClippingAssembly localAssembly = clippingAssembly;
    localAssembly.clearMatrix();
    glm::dmat4 src_transfo_mat = src_transfo.getTransformation();
    localAssembly.addTransformation(src_transfo_mat);

    std::vector<std::pair<uint32_t, bool>> cells;
    getClippedCells_impl(m_uRootCell, localAssembly, cells);

    std::vector<double> levelWeights;
    levelWeights.reserve(correctionLevels.size());
    for (size_t i = 0; i < correctionLevels.size(); ++i)
        levelWeights.push_back(std::pow(beta, static_cast<int>(i)));

    bool resultOk = true;
    const size_t totalCells = cells.size();
    if (progress && totalCells > 0)
        progress(0, totalCells);
    for (size_t cellIndex = 0; cellIndex < cells.size(); ++cellIndex)
    {
        const std::pair<uint32_t, bool>& cell = cells[cellIndex];
        std::vector<PointXYZIRGB> points;
        points.resize(tls_point_cloud_.getCellPointCount(cell.first));
        if (!tls_point_cloud_.getCellPoints(cell.first, reinterpret_cast<tls::Point*>(points.data()), points.size()))
        {
            if (progress)
                progress(cellIndex + 1, totalCells);
            continue;
        }

        std::vector<PointXYZIRGB> visiblePoints;
        if (cell.second)
        {
            clipIndividualPoints(points, visiblePoints, localAssembly);
        }
        else
        {
            visiblePoints.swap(points);
        }

        std::vector<PointXYZIRGB> corrected;
        corrected.reserve(visiblePoints.size());

        for (const PointXYZIRGB& point : visiblePoints)
        {
            glm::dvec4 globalPos = src_transfo_mat * glm::dvec4(point.x, point.y, point.z, 1.0);
            ColorBalanceCellKey baseKey = colorBalanceCellKeyFromGlobal(glm::dvec3(globalPos), baseCellSize);

            double sumWeightL = 0.0;
            double sumGainL = 0.0;
            double sumOffsetL = 0.0;
            double sumWeightI = 0.0;
            double sumGainI = 0.0;
            double sumOffsetI = 0.0;

            for (size_t level = 0; level < correctionLevels.size(); ++level)
            {
                ColorBalanceCellKey levelKey = colorBalanceParentCellKey(baseKey, static_cast<int>(level));
                auto it = correctionLevels[level].find(levelKey);
                if (it == correctionLevels[level].end())
                    continue;
                const ColorBalanceCellCorrection& correction = it->second;
                double weightBase = levelWeights[level];
                if (applyColor && correction.hasColor && correction.confidenceL > 0.0f)
                {
                    double weight = static_cast<double>(correction.confidenceL) * weightBase;
                    sumWeightL += weight;
                    sumGainL += weight * correction.gainL;
                    sumOffsetL += weight * correction.offsetL;
                }
                if (applyIntensity && correction.hasIntensity && correction.confidenceI > 0.0f)
                {
                    double weight = static_cast<double>(correction.confidenceI) * weightBase;
                    sumWeightI += weight;
                    sumGainI += weight * correction.gainI;
                    sumOffsetI += weight * correction.offsetI;
                }
            }

            PointXYZIRGB correctedPoint = point;
            if (applyColor && (pt_format_ == tls::PointFormat::TL_POINT_XYZ_RGB || pt_format_ == tls::PointFormat::TL_POINT_XYZ_I_RGB))
            {
                double gain = sumWeightL > 0.0 ? (sumGainL / sumWeightL) : 1.0;
                double offset = sumWeightL > 0.0 ? (sumOffsetL / sumWeightL) : 0.0;
                double luma = computeLuma(point);
                double lumaCorrected = gain * luma + offset;
                double delta = lumaCorrected - luma;
                correctedPoint.r = clampToByte(static_cast<double>(point.r) + delta);
                correctedPoint.g = clampToByte(static_cast<double>(point.g) + delta);
                correctedPoint.b = clampToByte(static_cast<double>(point.b) + delta);
            }
            if (applyIntensity && (pt_format_ == tls::PointFormat::TL_POINT_XYZ_I || pt_format_ == tls::PointFormat::TL_POINT_XYZ_I_RGB))
            {
                double gain = sumWeightI > 0.0 ? (sumGainI / sumWeightI) : 1.0;
                double offset = sumWeightI > 0.0 ? (sumOffsetI / sumWeightI) : 0.0;
                correctedPoint.i = clampToByte(gain * static_cast<double>(point.i) + offset);
            }
            corrected.push_back(correctedPoint);
        }

        resultOk &= writer->mergePoints(corrected.data(), corrected.size(), src_transfo, pt_format_);

        if (progress)
            progress(cellIndex + 1, totalCells);
    }

    return resultOk;
}

void EmbeddedScan::logClipAndWriteTimings()
{
    SubLogger& logger = Logger::log(LoggerMode::DataLog);
    logger << "********* EmbeddedScan::clipAndWrite ********\n";
    logger << " Total time worked on: \n";
    logger << "   (*) Get ptr  : " << pointer_time_ << "\n";
    logger << "   (*) Decode   : " << decode_time_ << "\n";
    logger << "   (*) Merge    : " << merge_time_ << "\n";
    logger << Logger::endl;
}


void EmbeddedScan::decodePointCoord(uint32_t cellId, std::vector<glm::dvec3>& dstPoints, uint32_t layerDepth, bool transformToGlobal)
{
    assert(layerDepth < 16);
    if (layerDepth > 15u)
        return;

    std::chrono::steady_clock::time_point tp_0 = std::chrono::steady_clock::now();

    uint64_t cell_point_count = tls_point_cloud_.getCellPointCount(cellId);
    std::vector<tls::Point> tmp_pts;
    tmp_pts.resize(cell_point_count);
    tls_point_cloud_.getCellPoints(cellId, tmp_pts.data(), tmp_pts.size());

    uint64_t layer_point_count(m_vTreeCells[cellId].m_layerIndexes[layerDepth]);
    dstPoints.reserve(dstPoints.size() + layer_point_count);

    std::chrono::steady_clock::time_point tp_1 = std::chrono::steady_clock::now();

    for (uint64_t n(0); n < layer_point_count; n++)
    {
        const glm::dvec3 dpts = glm::dvec3(tmp_pts[n].x, tmp_pts[n].y, tmp_pts[n].z);
        if (transformToGlobal)
            dstPoints.emplace_back(m_rotationToGlobal * dpts + m_translationToGlobal);
        else
            dstPoints.emplace_back(dpts);
    }

    std::chrono::steady_clock::time_point tp_2 = std::chrono::steady_clock::now();
    pointer_time_ += std::chrono::duration<float, std::ratio<1>>(tp_1 - tp_0).count();
    decode_time_ += std::chrono::duration<float, std::ratio<1>>(tp_2 - tp_1).count();
}

// static
bool EmbeddedScan::movePointBuffers(EmbeddedScan& dstScan, EmbeddedScan& srcScan, const std::vector<uint32_t>& correspCellId)
{
    assert(correspCellId.size() == srcScan.m_vTreeCells.size());

    for (uint32_t srcCellId = 0; srcCellId < srcScan.m_vTreeCells.size(); ++srcCellId)
    {
        uint32_t dstCellId = correspCellId[srcCellId];
        if (dstCellId == NO_CHILD)
            continue;
        assert(dstCellId < dstScan.m_cellCount);
        // Weak test that this is the same cell
        uint32_t dstDataSize = dstScan.m_vTreeCells[dstCellId].m_dataSize;
        uint32_t srcDataSize = srcScan.m_vTreeCells[srcCellId].m_dataSize;
        if (dstCellId != NO_CHILD && (dstDataSize == srcDataSize))
        {
            // TODO - Ça pourrait être bien de faire un std::move pour le SmartBuffer
            dstScan.m_pCellBuffers[dstCellId].alloc = srcScan.m_pCellBuffers[srcCellId].alloc;
            dstScan.m_pCellBuffers[dstCellId].buffer = srcScan.m_pCellBuffers[srcCellId].buffer;
            dstScan.m_pCellBuffers[dstCellId].isLocalMem = srcScan.m_pCellBuffers[srcCellId].isLocalMem;
            dstScan.m_pCellBuffers[dstCellId].state.store(srcScan.m_pCellBuffers[srcCellId].state.load());
            dstScan.m_pCellBuffers[dstCellId].lastUseFrameIndex.store(srcScan.m_pCellBuffers[srcCellId].lastUseFrameIndex.load());
            dstScan.m_pCellBuffers[dstCellId].ongoingProcesses.store(srcScan.m_pCellBuffers[srcCellId].ongoingProcesses.load());

            srcScan.m_pCellBuffers[srcCellId].alloc = VK_NULL_HANDLE;
            srcScan.m_pCellBuffers[srcCellId].buffer = VK_NULL_HANDLE;
            srcScan.m_pCellBuffers[srcCellId].isLocalMem = false;
            srcScan.m_pCellBuffers[srcCellId].state.store(TlDataState::NOT_LOADED);
            srcScan.m_pCellBuffers[srcCellId].lastUseFrameIndex.store(0);
            srcScan.m_pCellBuffers[srcCellId].ongoingProcesses.store(0);
        }
    }
    return true;
}

void EmbeddedScan::setComputeTransfo(const glm::dvec3& t, const glm::dquat& q)
{
    double xx = q.x * q.x;
    double yy = q.y * q.y;
    double zz = q.z * q.z;
    double ww = q.w * q.w;

    double xy = q.x * q.y;
    double xz = q.x * q.z;
    double xw = q.x * q.w;
    double yz = q.y * q.z;
    double yw = q.y * q.w;
    double zw = q.z * q.w;

    m_rotationToGlobal = {
        ww + xx - yy - zz,  2.0 * (zw + xy),    2.0 * (xz - yw),
        2.0 * (xy - zw),    ww - xx + yy - zz,  2.0 * (xw + yz),
        2.0 * (yw + xz),    2.0 * (yz - xw),    ww - xx - yy + zz
    };

    m_translationToGlobal = t;

    m_matrixToGlobal = {
        ww + xx - yy - zz,  2.0 * (zw + xy),    2.0 * (xz - yw),    0.0,
        2.0 * (xy - zw),    ww - xx + yy - zz,  2.0 * (xw + yz),    0.0,
        2.0 * (yw + xz),    2.0 * (yz - xw),    ww - xx - yy + zz,  0.0,
        t.x,                t.y,                t.z,                1.0
    };
}

BoundingBox EmbeddedScan::getLocalBoundingBox() const
{
    tls::Limits lim = tls_img_file_.getPointCloudHeader(0).limits;
    return { lim.xMin, lim.xMax, lim.yMin, lim.yMax, lim.zMin, lim.zMax };
}

struct TlTransferInfo {
    uint64_t fileOffset;
    uint64_t dataSize;
    SmartBuffer& sbuf;
    // bool staged; // if true, finish the transfer from the char* buffer to the VkBuffer 

    bool operator< (const TlTransferInfo& rhs) const
    {
        return fileOffset < rhs.fileOffset;
    }
};

bool EmbeddedScan::startStreamingAll(char* _stageBuf, uint64_t _stageSize, uint64_t& _stageOffset, std::vector<TlStagedTransferInfo>& gpuTransfers)
{
    VulkanManager& vkManager = VulkanManager::getInstance();
    bool continueStreaming = true;
    uint64_t previewOffset = _stageOffset;
    // Optimization: avoid to stream a scan that is not in the draw pass anymore
    // the '+1' unsure to cover the small interval between the start of a frame and moment a scan is call for drawing.
    if (vkManager.getCurrentFrameIndex() > m_lastFrameUse + 1)
        return true;

    // Choose some cell missing
    std::vector<uint32_t> copyMissingCells;
    {
        std::lock_guard<std::mutex> lock(m_streamMutex);
        copyMissingCells = m_missingCells;
    }

    std::set<TlTransferInfo> sortedCPUTransfers;
    std::set<TlTransferInfo> sortedStagedTransfers;
    for (uint32_t id : copyMissingCells)
    {
        TlDataState state = TlDataState::NOT_LOADED;
        std::set<TlTransferInfo>& sortedDst = m_vTreeCells[id].m_isLeaf ? sortedCPUTransfers : sortedStagedTransfers;
        VkMemoryPropertyFlags flags = m_vTreeCells[id].m_isLeaf ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        bool not_loaded = m_pCellBuffers[id].state.compare_exchange_strong(state, TlDataState::LOADING);

        if (!not_loaded)
            continue;

        if (m_vTreeCells[id].m_isLeaf == false)
        {
            previewOffset += m_vTreeCells[id].m_dataSize;
            if (previewOffset > _stageSize)
            {
                continueStreaming = false;
                // reset the buffer state to not loaded
                m_pCellBuffers[id].state.store(TlDataState::NOT_LOADED); // or compare_exchange ?
                break;
            }
        }

        if (vkManager.allocSmartBuffer(m_vTreeCells[id].m_dataSize, m_pCellBuffers[id], VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, flags) == VkResult::VK_SUCCESS)
        {
            sortedDst.insert({ m_vTreeCells[id].m_dataOffset, m_vTreeCells[id].m_dataSize, m_pCellBuffers[id] });
        }
        else
        {
            m_pCellBuffers[id].state.store(TlDataState::NOT_LOADED);
            continueStreaming = false;
        }
    }

    if (sortedCPUTransfers.empty() && sortedStagedTransfers.empty())
        return (continueStreaming);

    // Pack the transfer from the hdd by proximity
    // while there are remaining transfers
    for (auto it_tsf = sortedStagedTransfers.begin(); it_tsf != sortedStagedTransfers.end(); )
    {
        uint64_t srcOffset = it_tsf->fileOffset;
        uint64_t dataSize = it_tsf->dataSize;
        uint64_t subStageOffset = _stageOffset;

        gpuTransfers.push_back({ subStageOffset, it_tsf->dataSize, it_tsf->sbuf });
        subStageOffset += it_tsf->dataSize;

        // Try to concat adjecent transfers
        while (++it_tsf != sortedStagedTransfers.end())
        {
            if (it_tsf->fileOffset == (srcOffset + dataSize))
            {
                dataSize += it_tsf->dataSize;
                gpuTransfers.push_back({ subStageOffset, it_tsf->dataSize, it_tsf->sbuf });
                subStageOffset += it_tsf->dataSize;
            }
            else
                break;
        }

        if (tls_point_cloud_.getData(srcOffset, _stageBuf + _stageOffset, dataSize) == false)
            return false;

        _stageOffset += dataSize;
    }

    size_t miniBufSize = 2 * 1024 * 1024;
    char* miniBuf = new char[miniBufSize];
    // No limit in the amount of transfer
    for (auto it_tsf = sortedCPUTransfers.begin(); it_tsf != sortedCPUTransfers.end(); )
    {
        std::vector<TlStagedTransferInfo> groupedTransfers;
        uint64_t srcOffset = it_tsf->fileOffset;
        uint64_t dataSize = it_tsf->dataSize;
        uint64_t subStageOffset = 0;

        groupedTransfers.push_back({ subStageOffset, it_tsf->dataSize, it_tsf->sbuf });
        subStageOffset += it_tsf->dataSize;

        while (++it_tsf != sortedCPUTransfers.end() && subStageOffset + it_tsf->dataSize < miniBufSize)
        {
            if (it_tsf->fileOffset == (srcOffset + dataSize))
            {
                dataSize += it_tsf->dataSize;
                groupedTransfers.push_back({ subStageOffset, it_tsf->dataSize, it_tsf->sbuf });
                subStageOffset += it_tsf->dataSize;
            }
            else
                break;
        }

        // We transfer from the file the data locally continuous
        if (tls_point_cloud_.getData(srcOffset, miniBuf, dataSize) == false)
            return false;

        for (TlStagedTransferInfo sti : groupedTransfers)
        {
            char* pMapped = (char*)vkManager.getMappedPointer(sti.sbuf);
            memcpy(pMapped, miniBuf + sti.stageOffset, sti.dataSize);
            sti.sbuf.state.store(TlDataState::LOADED);
        }
    }
    delete[] miniBuf;

    return (continueStreaming);
}

// No more use of:
//   TreeCell::m_dataSize
//   TreeCell::m_dataOffset
//   EmbeddedScan::m_pointDataOffset
bool EmbeddedScan::startStreamingAll_k(void* _stageBuf, uint64_t _stageSize, uint64_t& _stageOffset, std::vector<TlStagedTransferInfo>& gpuTransfers)
{
    VulkanManager& vkManager = VulkanManager::getInstance();
    bool continueStreaming = true;
    // Optimization: avoid to stream a scan that is not in the draw pass anymore
    // the '+1' unsure to cover the small interval between the start of a frame and moment a scan is call for drawing.
    if (vkManager.getCurrentFrameIndex() > m_lastFrameUse + 1)
        return true;

    // Choose some cell missing
    std::vector<uint32_t> copyMissingCells;
    {
        std::lock_guard<std::mutex> lock(m_streamMutex);
        copyMissingCells = m_missingCells;
    }

    for (uint32_t id : copyMissingCells)
    {
        TlDataState state = TlDataState::NOT_LOADED;
        VkMemoryPropertyFlags flags = m_vTreeCells[id].m_isLeaf ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        bool not_loaded = m_pCellBuffers[id].state.compare_exchange_strong(state, TlDataState::LOADING);

        if (!not_loaded)
            continue;

        // Get the size of the data to be downloaded by the file lib
        uint64_t data_size = 0;
        tls_point_cloud_.getPointsRenderData(id, nullptr, data_size);

        if ((flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0)
        {
            if (_stageOffset + data_size > _stageSize)
            {
                continueStreaming = false;
                // reset the buffer state to not loaded
                m_pCellBuffers[id].state.store(TlDataState::NOT_LOADED); // or compare_exchange ?
                break;
            }
        }

        if (vkManager.allocSmartBuffer(data_size, m_pCellBuffers[id], VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, flags) != VkResult::VK_SUCCESS)
        {
            m_pCellBuffers[id].state.store(TlDataState::NOT_LOADED);
            continueStreaming = false;
        }

        if ((flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
        {
            char* pMapped = (char*)vkManager.getMappedPointer(m_pCellBuffers[id]);
            tls_point_cloud_.getPointsRenderData(id, pMapped, data_size);
            m_pCellBuffers[id].state.store(TlDataState::LOADED);
        }
        else
        {
            tls_point_cloud_.getPointsRenderData(id, (char*)_stageBuf + _stageOffset, data_size);
            gpuTransfers.push_back({ _stageOffset, data_size, m_pCellBuffers[id] });
            _stageOffset += data_size;
        }
    }

    return (continueStreaming);
}


void EmbeddedScan::ConcatCellStates::reset()
{
    memset(countByState, 0, sizeof(countByState));
    memset(sizeByState, 0, sizeof(sizeByState));
}

void EmbeddedScan::checkDataState()
{
    if (m_vTreeCells.empty())
        return;

    ConcatCellStates concatStates;
    concatStates.reset();
    checkCellDataState(m_uRootCell, concatStates, 0);

    SubLogger& log = Logger::log(DataLog);
    log << "***** Octree analytics for " << tls_img_file_.getPointCloudHeader(0).name << " *****\n";
    log << "----- Cells Loading state -----\n";
    log << "NOT_LOADED    = " << concatStates.countByState[(size_t)TlDataState::NOT_LOADED] << " | " << concatStates.sizeByState[(size_t)TlDataState::NOT_LOADED] << " octets\n";
    log << "LOADING       = " << concatStates.countByState[(size_t)TlDataState::LOADING] << " | " << concatStates.sizeByState[(size_t)TlDataState::LOADING] << " octets\n";
    log << "LOADED        = " << concatStates.countByState[(size_t)TlDataState::LOADED] << " | " << concatStates.sizeByState[(size_t)TlDataState::LOADED] << " octets\n";  

    log << Logger::endl;
}

void EmbeddedScan::checkCellDataState(uint32_t _cellId, ConcatCellStates& _concatStates, uint32_t _depth)
{
    assert(_cellId != NO_CHILD && _cellId < m_vTreeCells.size());
    TreeCell cell = m_vTreeCells[_cellId];

    size_t state = (size_t)m_pCellBuffers[_cellId].state.load();
    VkDeviceSize size = m_pCellBuffers[_cellId].size;
    _concatStates.countByState[state]++;
    _concatStates.sizeByState[state] += size;

    for (int j = 0; j < 8; j++)
    {
        if (cell.m_children[j] != NO_CHILD)
            checkCellDataState(cell.m_children[j], _concatStates, _depth + 1);
    }
}

bool EmbeddedScan::canBeDeleted()
{
    return (m_lastFrameUse.load() <= VulkanManager::getInstance().getSafeFrameIndex());
}

void EmbeddedScan::deleteFileWhenDestroyed(bool deletePhysicalFile)
{
    m_deleteFileWhenDestroyed = deletePhysicalFile;
}

//-------------------------------------+
//     Clipping Utility functions      |
//-------------------------------------+

//box
bool lessThanXmin(const glm::dvec4& vector) { return vector.x < -vector.w; }
bool moreThanXmax(const glm::dvec4& vector) { return vector.x > vector.w; }
bool lessThanYmin(const glm::dvec4& vector) { return vector.y < -vector.w; }
bool moreThanYmax(const glm::dvec4& vector) { return vector.y > vector.w; }
bool lessThanZmin(const glm::dvec4& vector) { return vector.z < -vector.w; }
bool moreThanZmax(const glm::dvec4& vector) { return vector.z > vector.w; }
//view
bool lessThanWmin(const glm::dvec4& vector) { return vector.w < 0.f; }

typedef bool(*compareBound)(const glm::dvec4&);

const compareBound compareBounds_view[6] = { lessThanXmin, moreThanXmax, lessThanYmin, moreThanYmax, moreThanZmax, lessThanWmin };

// Return true if the cube is clipped outside the box [[[-1, 1]]] in heterogeneous coordinates
void testCubeClipped(const HCube& _cube, TestInside& _result, compareBound const* fctList)
{
    // Test if we already know that the HCube is inside the unit cube [[[-1, 1]]]
    if (_result.allInside)
        return;

    // Reset 'allInside' to true because we want to apply a logical AND.
    _result.allInside = true;
    for (int s = 0; s < 6; ++s)
    {
        // We test the HCube against all the side which are not resolved
        if (_result.inside[s])
            continue;

        // Temporary value to know if all corners are outside
        bool outside = true;
        _result.inside[s] = true;
        for (int c = 0; c < 8; ++c)
        {
            // 'b' indicate if the corner 'c' is outside the side 's' of the unit cube
            bool b = fctList[s](_cube.corner[c]);
            outside &= b;
            _result.inside[s] &= !b;
        }
        _result.oneOutside |= outside;
        _result.allInside &= _result.inside[s];
    }
}

//-------------------------------------+
//          Viewing functions          |
//-------------------------------------+
bool EmbeddedScan::getVisibleTree_impl(uint32_t _cellId, std::vector<TlCellDrawInfo>& _result, const TlProjectionInfo& _projInfo, const TlFrustumTest& _frustumTest, std::vector<uint32_t>& _missingCells)
{
    assert(_cellId != NO_CHILD && _cellId < m_vTreeCells.size());

    const TreeCell& cell = m_vTreeCells[_cellId];

    // ------------------------------------------------
    //   Clip the cube representing the cell boundary
    // ------------------------------------------------

    TlFrustumTest childFrustumTest;
    memcpy(&childFrustumTest, &_frustumTest, sizeof(TlFrustumTest));
    // Test if the cube is clipped outside of the bound
    // If not, also return in 'allInside' if the cube is totally inside.
    testCubeClipped(childFrustumTest.cube, childFrustumTest.test, compareBounds_view);
    if (childFrustumTest.test.oneOutside)
        // Stop the recurssion no draw added
        return true;

    // ------------------------------------------------
    //   Determine the point count to draw 
    // ------------------------------------------------

    // Result needed
    uint32_t renderDepth = cell.m_depthSPT;
    bool enoughPoints = false;
    // if the cell cross the (z = 0) bound, then the camera is inside the cell.
    // In this case we should not look for a minimum resolution.
    // Simply render with the maximum points available or try to render the children
    if (childFrustumTest.test.inside[5] && _projInfo.decimationRatio > 0.f)
    {
        uint64_t minResolution = getMinimumPointsResolution(childFrustumTest.cube, _projInfo.width, _projInfo.height, (_projInfo.octreePointSize + _projInfo.deltaFilling) / sqrt(_projInfo.decimationRatio));

        // In our SPT, we have pow2(depth) points in each dimension
        // Now we need to find the minimum depth that get us the minimum points required

        uint32_t maxResoAvailable = (1u << cell.m_depthSPT);

        if (cell.m_isLeaf)
        {
            for (uint32_t d = cell.m_depthSPT + 1; d-- > 0; )
            {
                double depthCorrection = ((1u << (cell.m_depthSPT - d)) + 1);
                if (minResolution <= maxResoAvailable / depthCorrection)
                {
                    renderDepth = d;
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            if (minResolution <= maxResoAvailable)
                enoughPoints = true;
        }
    }

    // -----------------------------------------------------------
    //   Continue the view traversal over the children if needed
    // -----------------------------------------------------------

    // Check the children for the node with not enough points
    if ((cell.m_isLeaf == false) && (enoughPoints == false))
    {
        bool allDataPresent = true;
        // Do the recursion on the children
        for (int j = 0; j < 8; j++)
        {
            if (cell.m_children[j] == NO_CHILD)
                continue;

            // Compute the child coordinates
            childFrustumTest.cube = HCube(_frustumTest.cube, j);

            // check if the children are drawable
            allDataPresent &= getVisibleTree_impl(cell.m_children[j], _result, _projInfo, childFrustumTest, _missingCells);
        }

        if (allDataPresent)
        {
            // No drawable child is missing, we can add the draw commands
            return true;
        }
    }

    // --------------------------------------------------------------------------
    //   Draw the current cell on condition of the correct data synchronization
    // --------------------------------------------------------------------------

    // touch the buffer to notify that it must not be unloaded during this frame
    // Check that the buffer is loaded
    if (VulkanManager::getInstance().touchSmartBuffer(m_pCellBuffers[_cellId]))
    {
        _result.push_back({ m_pCellBuffers[_cellId].buffer, _cellId, cell.m_layerIndexes[renderDepth], cell.m_iOffset, cell.m_rgbOffset });
        return true;
    }
    else
    {
        // Mark the cell as missing (for DL)
        _missingCells.push_back(_cellId);
        return false;
    }
}

// The default behavior for multiple clipping boxes is the union.
// if a test with one of the clipping succed we can skip all the other CB.
// if a test with one of the clipping fail, we exclued it from the next test.
//    * if this is the last clipping box remaining then all the CB have failed and the draw is not emitted.
bool EmbeddedScan::getVisibleTreeMultiClip_impl(uint32_t _cellId, std::vector<TlCellDrawInfo>&_result, std::vector<TlCellDrawInfo_multiCB>&_resultCB, const TlProjectionInfo & _projInfo, const TlFrustumTest & _frustumTest, const ClippingAssembly & clippingAssembly, std::vector<uint32_t>&_missingCells)
{
    assert(_cellId != NO_CHILD && _cellId < m_vTreeCells.size());

    const TreeCell& cell = m_vTreeCells[_cellId];

    // ------------------------------------------------
    //   Clip the cube representing the cell boundary
    // ------------------------------------------------

    TlFrustumTest childFrustumTest;
    memcpy(&childFrustumTest, &_frustumTest, sizeof(TlFrustumTest));
    // Test if the cube is clipped outside of the bound
    testCubeClipped(_frustumTest.cube, childFrustumTest.test, compareBounds_view);
    if (childFrustumTest.test.oneOutside)
        return true;

    bool acceptAssembly = false;
    bool rejectAssembly = false;
    ClippingAssembly childAssembly;
    glm::dvec3 minCorner(cell.m_position[0], cell.m_position[1], cell.m_position[2]);
    clippingAssembly.testCube(minCorner, cell.m_size, acceptAssembly, rejectAssembly, childAssembly);

    assert(acceptAssembly || rejectAssembly || !(childAssembly.clippingUnion.empty() && childAssembly.clippingIntersection.empty() && childAssembly.rampActives.empty()));
    assert(!acceptAssembly || (childAssembly.clippingUnion.empty() && childAssembly.clippingIntersection.empty() && childAssembly.rampActives.empty()));

    if (rejectAssembly)
        return true;

    // -------------------------------------
    //   Determine the point count to draw
    // -------------------------------------

    // Result needed
    uint32_t renderDepth = cell.m_depthSPT;
    bool enoughPoints = false;

    // if the cell cross the (z = 0) bound, then the camera is inside the cell.
    // In this case we should not look for a minimum resolution.
    // Simply render with the maximum points available or try to render the children
    if (childFrustumTest.test.inside[5] && _projInfo.decimationRatio > 0.f)
    {
        uint64_t minResolution = getMinimumPointsResolution(_frustumTest.cube, _projInfo.width, _projInfo.height, (_projInfo.octreePointSize + _projInfo.deltaFilling) / sqrt(_projInfo.decimationRatio));

        // In our SPT, we have pow2(depth) points in each dimension
        // Now we need to find the minimum depth that get us the minimum points required

        uint32_t maxResoAvailable = (1u << cell.m_depthSPT);

        if (cell.m_isLeaf)
        {
            for (uint32_t d = cell.m_depthSPT + 1; d-- > 0; )
            {
                double depthCorrection = ((1u << (cell.m_depthSPT - d)) + 1);
                if (minResolution <= maxResoAvailable / depthCorrection)
                {
                    renderDepth = d;
                    enoughPoints = true;
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            if (minResolution <= maxResoAvailable)
                enoughPoints = true;
        }
    }

    // -----------------------------------------------------------
    //   Continue the view traversal over the children if needed
    // -----------------------------------------------------------

    // Check the children for the node with not enough points
    if ((cell.m_isLeaf == false) && (enoughPoints == false))
    {
        bool allDataPresent = true;

        // Do the recursion on the children
        for (int j = 0; j < 8; j++)
        {
            if (cell.m_children[j] == NO_CHILD)
                continue;
            // Compute the child coordinates
            childFrustumTest.cube = HCube(_frustumTest.cube, j);

            if (acceptAssembly)
            {
                // When no more clipping must be performed, we fall back on normal tree view
                allDataPresent &= getVisibleTree_impl(cell.m_children[j], _result, _projInfo, childFrustumTest, _missingCells);
            }
            else
            {
                // check if the children are drawable
                allDataPresent &= getVisibleTreeMultiClip_impl(cell.m_children[j], _result, _resultCB, _projInfo, childFrustumTest, childAssembly, _missingCells);
            }
        }

        if (allDataPresent)
        {
            // No drawable child is missing, we can return without drawing this node
            return true;
        }
    }

    // --------------------------------------------------------------------------
    //   Draw the current cell on condition of the correct data synchronization
    // --------------------------------------------------------------------------

    // touch the buffer to notify that it must not be unloaded during this frame
    // Check that the buffer is loaded
    if (VulkanManager::getInstance().touchSmartBuffer(m_pCellBuffers[_cellId]))
    {
        if (acceptAssembly)
            _result.push_back({ m_pCellBuffers[_cellId].buffer, _cellId, cell.m_layerIndexes[renderDepth], cell.m_iOffset, cell.m_rgbOffset });
        else
        {
            std::vector<ClippingGpuId> gpuClip;
            for (std::shared_ptr<IClippingGeometry>& cg : childAssembly.clippingIntersection)
                gpuClip.push_back(cg->gpuDrawId);
            for (std::shared_ptr<IClippingGeometry>& cg : childAssembly.clippingUnion)
                gpuClip.push_back(cg->gpuDrawId);
            for (std::shared_ptr<IClippingGeometry>& cg : childAssembly.rampActives)
                gpuClip.push_back(cg->gpuDrawId);

            _resultCB.push_back({ m_pCellBuffers[_cellId].buffer, _cellId, cell.m_layerIndexes[renderDepth], cell.m_iOffset, cell.m_rgbOffset, gpuClip });
        }
        return true;
    }
    else
    {
        _missingCells.push_back(_cellId);
        return false;
    }
}

const uint8_t cst_edgeIndex[12][2] = {
    {0, 1}, {2, 3}, {4, 5}, {6, 7},  // edges along (Oz)
    {0, 2}, {1, 3}, {4, 6}, {5, 7},  // edges along (Oy)
    {0, 4}, {1, 5}, {2, 6}, {3, 7}   // edges along (Ox)
};

const uint8_t cst_faceIndex[6][4] = {
    {0, 1, 3, 2},
    {0, 2, 6, 4},
    {0, 1, 5, 4},
    {2, 3, 7, 6},
    {4, 5, 7, 6},
    {1, 3, 7, 5}
};

uint64_t EmbeddedScan::getMinimumPointsResolution(const HCube& _cube, uint64_t _width, uint64_t _height, float _pointSize)
{
    double halfW = _width / 2.0;
    double halfH = _height / 2.0;

    // Compute all the corner position in the frame space (FS)
    glm::dvec2 cubeFS[8];
    for (uint32_t c = 0; c < 8; ++c)
    {
        // if (w = 0) we return a very high resolution because we cannot do any computation
        if (_cube.corner[c].w == 0.f)
            return 4000u;

        cubeFS[c] = glm::dvec2(_cube.corner[c].x / _cube.corner[c].w * halfW, _cube.corner[c].y / _cube.corner[c].w * halfH);
    }

    double maxResolution = 0.0;
    for (uint32_t e = 0; e < 12; ++e)
    {
        glm::dvec2 edge = cubeFS[cst_edgeIndex[e][1]] - cubeFS[cst_edgeIndex[e][0]];

        double maxEdge = std::max(abs(edge.x), abs(edge.y));
        maxResolution = std::max(maxEdge, maxResolution);
    }

    for (uint32_t f = 0; f < 6; ++f)
    {
        glm::dvec2 diag_0 = cubeFS[cst_faceIndex[f][2]] - cubeFS[cst_faceIndex[f][0]];
        glm::dvec2 diag_1 = cubeFS[cst_faceIndex[f][3]] - cubeFS[cst_faceIndex[f][1]];

        double maxDiag_0 = std::max(abs(diag_0.x), abs(diag_0.y));
        double maxDiag_1 = std::max(abs(diag_1.x), abs(diag_1.y));

        double minFace = std::min(maxDiag_0, maxDiag_1);
        maxResolution = std::max(minFace, maxResolution);
    }

    return (uint64_t)(maxResolution / _pointSize + 1.0); // add 1.f to get the ceil value
}

bool EmbeddedScan::testFullyClippedOutCells(uint32_t _cellId, const ClippingAssembly& _clippingAssembly, std::vector<uint32_t>& _partially_clipped_cells) const
{
    assert(_cellId != NO_CHILD && _cellId < m_vTreeCells.size());

    const TreeCell& cell = m_vTreeCells[_cellId];

    // ------------------------------------------------
    //   Clip the cube representing the cell boundary
    // ------------------------------------------------

    double radius = cell.m_size * sqrt(3.0) / 2.0;
    glm::dvec4 center(cell.m_position[0] + cell.m_size / 2.0,
        cell.m_position[1] + cell.m_size / 2.0,
        cell.m_position[2] + cell.m_size / 2.0, 1.0);

    bool acceptAllClipping;
    bool rejectAllClipping;
    ClippingAssembly forwardAssembly;
    _clippingAssembly.testSphere(center, radius, acceptAllClipping, rejectAllClipping, forwardAssembly);

    // Early ending when one cell is clipped out
    if (rejectAllClipping)
        return true;

    // -----------------------------------------------------------
    //   Continue the view traversal over the children if needed
    // -----------------------------------------------------------

    // Check the children for the node with not enough points
    if (cell.m_isLeaf)
    {
        if (!acceptAllClipping) // && !rejectAllClipping
            _partially_clipped_cells.push_back(_cellId);
    }
    else
    {
        // Do the recursion on the children
        for (int j = 0; j < 8; j++)
        {
            if (cell.m_children[j] == NO_CHILD)
                continue;

            // One result to 'true' end the recursion
            if (testFullyClippedOutCells(cell.m_children[j], forwardAssembly, _partially_clipped_cells))
                return true;
        }
    }
    return false;
}

void EmbeddedScan::getClippedCells_impl(uint32_t _cellId, const ClippingAssembly& _clippingAssembly, std::vector<std::pair<uint32_t, bool>>& _result) const
{
    assert(_cellId != NO_CHILD && _cellId < m_vTreeCells.size());

    const TreeCell& cell = m_vTreeCells[_cellId];

    // ------------------------------------------------
    //   Clip the cube representing the cell boundary
    // ------------------------------------------------

    double radius = cell.m_size * sqrt(3.0) / 2.0;
    glm::dvec4 center(cell.m_position[0] + cell.m_size / 2.0,
        cell.m_position[1] + cell.m_size / 2.0,
        cell.m_position[2] + cell.m_size / 2.0, 1.0);

    bool acceptAllClipping;
    bool rejectAllClipping;
    ClippingAssembly forwardAssembly;
    _clippingAssembly.testSphere(center, radius, acceptAllClipping, rejectAllClipping, forwardAssembly);

    if (rejectAllClipping)
        return;

    // -----------------------------------------------------------
    //   Continue the view traversal over the children if needed
    // -----------------------------------------------------------

    // Check the children for the node with not enough points
    if (cell.m_isLeaf)
    {
        _result.push_back({ _cellId, !acceptAllClipping });
    }
    else
    {
        // Do the recursion on the children
        for (int j = 0; j < 8; j++)
        {
            if (cell.m_children[j] == NO_CHILD)
                continue;

            getClippedCells_impl(cell.m_children[j], forwardAssembly, _result);
        }
    }
}

void EmbeddedScan::getDecodedPoints(const std::vector<uint32_t>& leavesId, std::vector<glm::dvec3>& retPoints, bool transformToGlobal)
{
    if (leavesId.empty())
        return;

    retPoints.clear();
    for (uint32_t id : leavesId)
    {
        decodePointCoord(id, retPoints, 15u, transformToGlobal);
    }
}

void EmbeddedScan::samplePointsByStep(float samplingStep, const std::vector<uint32_t>& leavesId, std::vector<glm::dvec3>& sampledPoints)
{
    float precisionValue = tls::getPrecisionValue(pt_precision_);
    uint32_t maxLayerDelta = std::max(1u, (uint32_t)(std::floorf(std::log2(samplingStep / precisionValue))));
    // !!! Attention !!!
    // Suite à une erreur dans la définition initiale du format tls, le layer 16 n’existe pas.
    // Seuls les layers [0, 15] peuvent être sauvegardés dans 'layer_indexes'
    // Mais pourtant le layer 16 devrait contenir la précision maximale.
    // !!!
    uint32_t layerDepth = 16u - maxLayerDelta; // Cette formule est juste mais n’autorise pas la valeur 'maxLayerDelta' = 0

    sampledPoints.clear();
    for (uint32_t id : leavesId)
    {
        decodePointCoord(id, sampledPoints, layerDepth, true && "global");
    }
}

void EmbeddedScan::samplePointsByQuota(size_t quotaMax, const std::vector<uint32_t>& leavesId, std::vector<glm::dvec3>& sampledPoints)
{
    //*** On cherche le bon niveau de sampling pour attendre le quota au plus proche ***
    // On souhaite que toutes les cellules soient au même niveau de sampling.
    uint32_t sumLayers[16] = { 0u }; // Note : le layer 16 n'est pas sauvegardé.
    for (uint32_t id : leavesId)
    {
        TreeCell cell = m_vTreeCells[id];
        for (int d = 0; d < 16; ++d)
        {
            sumLayers[d] += cell.m_layerIndexes[d];
        }
    }

    int preferedDepth = 0;
    for (int d = 0; d < 16; ++d)
    {
        if (sumLayers[d] <= quotaMax)
            preferedDepth = d;
        else
            break;
    }

    sampledPoints.clear();
    sampledPoints.reserve(sumLayers[preferedDepth]);
    for (uint32_t id : leavesId)
    {
        decodePointCoord(id, sampledPoints, preferedDepth, true);
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//              Lucas' Functions
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// NOTE(robin) - Here we can make the treatment faster by selecting the best in local coordinates, then return it in global coordinates.
bool EmbeddedScan::beginRayTracing(const glm::dvec3& globalRay, const glm::dvec3& globalRayOrigin, glm::dvec3& bestPoint, const double& cosAngleThreshold, const ClippingAssembly& clippingAssembly, const bool& isOrtho)
{
    TreeCell root = m_vTreeCells[m_uRootCell];
    double rootSize = root.m_size;
    glm::dvec3 localRay = glm::inverse(m_rotationToGlobal) * glm::dvec3(globalRay.x, globalRay.y, globalRay.z);
    glm::dvec3 localRayOrigin = getLocalCoord(globalRayOrigin);

    // Rayon local non altérée pour la détection
    glm::dvec3 trueLocalRay = localRay / glm::length(localRay);
    glm::dvec3 trueLocalRayOrigin = localRayOrigin;
    ClippingAssembly localAssembly = clippingAssembly;
    localAssembly.clearMatrix();
    localAssembly.addTransformation(m_matrixToGlobal);

    int rayModifier = updateRay(localRay, localRayOrigin, rootSize);


    double tx0, ty0, tz0, tx1, ty1, tz1;
    tx0 = (root.m_position[0] - localRayOrigin.x) / localRay.x;
    ty0 = (root.m_position[1] - localRayOrigin.y) / localRay.y;
    tz0 = (root.m_position[2] - localRayOrigin.z) / localRay.z;
    tx1 = (root.m_position[0] + rootSize - localRayOrigin.x) / localRay.x;
    ty1 = (root.m_position[1] + rootSize - localRayOrigin.y) / localRay.y;
    tz1 = (root.m_position[2] + rootSize - localRayOrigin.z) / localRay.z;
	//if a coordinate of local ray is 0, we want still want this times to be definite
	if (std::fabs(abs(localRay.x)) <= std::numeric_limits<double>::epsilon())
	{
		if (root.m_position[0] > localRayOrigin.x)
			tx0 = DBL_MAX;
		else tx0 = -DBL_MAX;
		if (root.m_position[0] + rootSize - localRayOrigin.x>0)
			tx1 = DBL_MAX;
		else tx1 = -DBL_MAX;
	}
	if (std::fabs(abs(localRay.y)) <= std::numeric_limits<double>::epsilon())
	{
		if (root.m_position[1] > localRayOrigin.y)
			ty0 = DBL_MAX;
		else ty0 = -DBL_MAX;
		if (root.m_position[1] + rootSize - localRayOrigin.y > 0)
			ty1 = DBL_MAX;
		else ty1 = -DBL_MAX;
	}
	if (std::fabs(abs(localRay.z)) <= std::numeric_limits<double>::epsilon())
	{
		if (root.m_position[2] > localRayOrigin.z)
			tz0 = DBL_MAX;
		else tz0 = -DBL_MAX;
		if (root.m_position[2] + rootSize - localRayOrigin.z > 0)
			tz1 = DBL_MAX;
		else tz1 = -DBL_MAX;
	}

    double max0, min1;
    max0 = tx0;
    if (ty0 > max0) { max0 = ty0; }
    if (tz0 > max0) { max0 = tz0; }
    min1 = tx1;
    if (ty1 < min1) { min1 = ty1; }
    if (tz1 < min1) { min1 = tz1; }

    std::vector<uint32_t> leafList;
    if (max0 < min1) { traceRay(tx0, ty0, tz0, tx1, ty1, tz1, m_uRootCell, rayModifier, leafList, localAssembly, localRayOrigin); }

    //leafList has been computed, now make the list of points 
	double rayRadius = 0.0015;

	//second vesion : decode leaves one by one
	bool success = false;
	//glm::dvec3 targetGlobal = findBestPointIterative(leafList, globalRay / glm::length(globalRay), globalRayOrigin, cosAngleThreshold, rayRadius, clippingAssembly, isOrtho, success);

    // NOTE(robin) - On utilise les rayons et les clippingAssembly dans l'espace local du scan
	glm::dvec3 targetGlobal = findBestPointIterative(leafList, trueLocalRay, trueLocalRayOrigin, cosAngleThreshold, rayRadius, localAssembly, isOrtho, success);

    bestPoint = targetGlobal;

    return success;
}

glm::dvec3 EmbeddedScan::findBestPointIterative(const std::vector<uint32_t>& leafList, const glm::dvec3& rayDirection, const glm::dvec3& rayOrigin, const double& cosAngleThreshold, const double& rayRadius, const ClippingAssembly& localClippingAssembly, const bool& isOrtho, bool& success)
{
	double dMin(DBL_MAX), bestCosAngle(-1), currCosAngle(0), currDistance(0), distanceThreshold(0.01);
	success = false;
	double resultDistance(0), distanceBestAnglePoint(0), realBestCosAngle(0);
	if (isOrtho)
		bestCosAngle = cosAngleThreshold + 1;
	bool hasGoodAngle(false), oneMoreLeaf(true);
	glm::dvec3 bestAnglePoint;
	std::vector<glm::dvec3> goodAnglePoints;

	glm::dvec3 result(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN());

	for (int leafStep = 0; leafStep < (int)leafList.size(); leafStep++)
	{
        std::vector<glm::dvec3> decodedPoints;
        getDecodedPoints({ leafList[leafStep] }, decodedPoints, false);

        for (const glm::dvec3& point : decodedPoints)
        {
            if (!localClippingAssembly.testPoint(glm::dvec4(point, 1.0)))
            {
                continue;
            }

            glm::dvec3 pointRay(point - rayOrigin);
            currDistance = glm::length(pointRay);
            if (glm::dot(pointRay, rayDirection) < 0)
            {
                continue;
            }
            success = true;

            glm::dvec3 proj = glm::dot(pointRay, rayDirection)*rayDirection - pointRay;
            currCosAngle = isOrtho ?
                glm::length(glm::cross(rayDirection, pointRay)) / glm::length(rayDirection) :
                glm::dot(rayDirection, pointRay / currDistance);

            double projLength = glm::length(proj);
            proj = proj / projLength;


            if (!isOrtho)
            {
                if ((rayRadius / projLength) >= 1.0) {
                    hasGoodAngle = true;
                    currCosAngle = rayRadius / projLength;
                    if (currDistance < (dMin*1.05))
                        goodAnglePoints.push_back(point);
                    if (currDistance < dMin)
                    {
                        dMin = currDistance;
                    }
                }
                else {
                    pointRay = pointRay + proj * rayRadius;
                    pointRay = pointRay / glm::length(pointRay);
                    currCosAngle = glm::dot(rayDirection, pointRay);

                    if (currCosAngle > cosAngleThreshold)
                    {
                        hasGoodAngle = true;
                        if (currDistance < (dMin * 1.05))
                            goodAnglePoints.push_back(point);
                        if (currDistance < dMin)
                        {
                            dMin = currDistance;
                        }
                    }
                }
                if ((currCosAngle > bestCosAngle) && (!isOrtho))
                {
                    bestCosAngle = currCosAngle;
                    bestAnglePoint = point;
                    distanceBestAnglePoint = currDistance;
                    realBestCosAngle = currCosAngle;
                }
            }

            //same thing is orthographic mode

            if ((currCosAngle < cosAngleThreshold) && (isOrtho))
            {
                hasGoodAngle = true;
                if (currDistance < (dMin + distanceThreshold))
                    goodAnglePoints.push_back(point);
                if (currDistance < dMin)
                {
                    dMin = currDistance;
                }
            }
            if ((currCosAngle < bestCosAngle) && (isOrtho))
            {
                bestCosAngle = currCosAngle;
                bestAnglePoint = point;
                distanceBestAnglePoint = currDistance;
            }
        }
		if (!oneMoreLeaf)
			break;
		else if (hasGoodAngle)
			oneMoreLeaf = false;
	}

	if (!hasGoodAngle)
	{
		result = bestAnglePoint;
	}
	else
	{
		result = bestAnglePoint;
		bestCosAngle = -1;
        double currScore, bestScore;
		if (isOrtho)
			bestCosAngle = cosAngleThreshold + 1;
		for (int i = 0; i < (int)goodAnglePoints.size(); i++)
		{
			glm::dvec3 point = goodAnglePoints[i];
			glm::dvec3 pointRay(point - rayOrigin);
			currDistance = glm::length(pointRay);
			glm::dvec3 proj = glm::dot(pointRay, rayDirection)*rayDirection - pointRay;
			currCosAngle = glm::dot(rayDirection, pointRay / currDistance);
			if(isOrtho)
				currCosAngle = glm::length(glm::cross(rayDirection, pointRay)) / glm::length(rayDirection);
			if (((rayRadius / glm::length(proj)) >= 1)&&(!isOrtho)) {
				currCosAngle = rayRadius / glm::length(proj);
			}
			/*if ((currDistance < (dMin * 1.05)) && (currCosAngle > bestCosAngle) && (!isOrtho))
			{
				result = point;
				bestCosAngle = currCosAngle;
				resultDistance = currDistance;
			}*/
            currScore = 1.7 * glm::length(proj) + currDistance;
            if (i == 0)
            {
                bestScore = currScore;
                result = point;
            }
            if ((currScore < bestScore)&&(!isOrtho))
            {
                bestScore = currScore;
                result = point;
            }

			if ((currDistance < (dMin+distanceThreshold)) && (currCosAngle < bestCosAngle) && (isOrtho))
			{
				result = point;
				bestCosAngle = currCosAngle;
				resultDistance = currDistance;
			}
		}
	}
	return getGlobalCoord(result);
}

int EmbeddedScan::updateRay(glm::dvec3& localRay, glm::dvec3& localRayOrigin, const double& rootSize)
{
    int result(0);
    TreeCell root = m_vTreeCells[m_uRootCell];
    double norm = glm::length(localRay);
    localRay = localRay / norm;
    for (int loop = 0; loop < 3; loop++)
    {
        if (localRay[loop] < 0)
        {
            localRay[loop] = -localRay[loop];
            localRayOrigin[loop] = 2 * root.m_position[loop] + rootSize - localRayOrigin[loop];
            result += 1 << (2 - loop);
        }
    }
    return result;
}

void EmbeddedScan::traceRay(const double& tx0, const double& ty0, const double& tz0, const double& tx1, const double& ty1, const double& tz1, const uint32_t& cellId, const int& rayModifier, std::vector<uint32_t>& leafList, const ClippingAssembly& clippingAssembly, const glm::dvec3& localRayOrigin)
{
    TreeCell& cell = m_vTreeCells[cellId];
    bool isNodeInteresting(false);

    double txm, tym, tzm;
    int currNode;

    if ((tx1 < 0.0) || (ty1 < 0.0) || (tz1 < 0.0)) { return; }
    if (cell.m_isLeaf)
    {
        glm::dvec4 center(cell.m_position[0] + cell.m_size / 2, cell.m_position[1] + cell.m_size / 2, cell.m_position[2] + cell.m_size / 2, 1.0);
        glm::dvec3 corner(cell.m_position[0], cell.m_position[1], cell.m_position[2]);
        double radius = cell.m_size * sqrt(3) / 2.0;
        bool accept = false;
        bool reject = false;
        ClippingAssembly leftoverAssembly; // can be used to test the cell's points with fewer clippings
        clippingAssembly.testCube(corner, cell.m_size, accept, reject, leftoverAssembly);
        if (reject == false)
            leafList.push_back(cellId);
        return;
    }
    txm = 0.5*(tx0 + tx1);
    tym = 0.5*(ty0 + ty1);
    tzm = 0.5*(tz0 + tz1);
	if ((std::fabs(abs(txm)) <= std::numeric_limits<double>::epsilon()) && (abs(tx0) > (0.5*DBL_MAX)) && (abs(tx1) > (0.5*DBL_MAX)))
	{
		if (localRayOrigin.x < (cell.m_position[0] + 0.5*cell.m_size))
			txm = DBL_MAX;
		else
			txm = -DBL_MAX;
	}
		
	if ((std::fabs(abs(tym)) <= std::numeric_limits<double>::epsilon()) && (abs(ty0) > (0.5*DBL_MAX)) && (abs(ty1) > (0.5*DBL_MAX)))
	{
		if (localRayOrigin.y < (cell.m_position[1] + 0.5*cell.m_size))
			tym = DBL_MAX;
		else
			tym = -DBL_MAX;
	}

	if ((std::fabs(abs(tzm)) <= std::numeric_limits<double>::epsilon()) && (abs(tz0) > (0.5*DBL_MAX)) && (abs(tz1) > (0.5*DBL_MAX)))
	{
		if (localRayOrigin.z < (cell.m_position[2] + 0.5*cell.m_size))
			tzm = DBL_MAX;
		else
			tzm = -DBL_MAX;
	}
    currNode = OctreeRayTracing::firstNode(tx0, ty0, tz0, txm, tym, tzm);

    while (currNode < 8)
    {
        int ourIndex = rayModifier ^ currNode;
        bool hasChild(cell.m_children[ourIndex] != NO_CHILD);

        switch (currNode)
        {
        case 0:
        {
            if (hasChild) {
                traceRay(tx0, ty0, tz0, txm, tym, tzm, cell.m_children[ourIndex], rayModifier, leafList, clippingAssembly, localRayOrigin);
            }
            currNode = OctreeRayTracing::new_node(txm, tym, tzm, 4, 2, 1);
            break;
        }
        case 1:
        {
            if (hasChild) {
                traceRay(tx0, ty0, tzm, txm, tym, tz1, cell.m_children[ourIndex], rayModifier, leafList, clippingAssembly, localRayOrigin);
            }
            currNode = OctreeRayTracing::new_node(txm, tym, tz1, 5, 3, 8);
            break;
        }
        case 2:
        {
            if (hasChild) {
                traceRay(tx0, tym, tz0, txm, ty1, tzm, cell.m_children[ourIndex], rayModifier, leafList, clippingAssembly, localRayOrigin);
            }
            currNode = OctreeRayTracing::new_node(txm, ty1, tzm, 6, 8, 3);
            break;
        }
        case 3:
        {
            if (hasChild) {
                traceRay(tx0, tym, tzm, txm, ty1, tz1, cell.m_children[ourIndex], rayModifier, leafList, clippingAssembly, localRayOrigin);
            }
            currNode = OctreeRayTracing::new_node(txm, ty1, tz1, 7, 8, 8);
            break;
        }
        case 4:
        {
            if (hasChild) {
                traceRay(txm, ty0, tz0, tx1, tym, tzm, cell.m_children[ourIndex], rayModifier, leafList, clippingAssembly, localRayOrigin);
            }
            currNode = OctreeRayTracing::new_node(tx1, tym, tzm, 8, 6, 5);
            break;
        }
        case 5:
        {
            if (hasChild) {
                traceRay(txm, ty0, tzm, tx1, tym, tz1, cell.m_children[ourIndex], rayModifier, leafList, clippingAssembly, localRayOrigin);
            }
            currNode = OctreeRayTracing::new_node(tx1, tym, tz1, 8, 7, 8);
            break;
        }
        case 6:
        {
            if (hasChild) {
                traceRay(txm, tym, tz0, tx1, ty1, tzm, cell.m_children[ourIndex], rayModifier, leafList, clippingAssembly, localRayOrigin);
            }
            currNode = OctreeRayTracing::new_node(tx1, ty1, tzm, 8, 8, 7);
            break;
        }
        case 7:
        {
            if (hasChild) {
                traceRay(txm, tym, tzm, tx1, ty1, tz1, cell.m_children[ourIndex], rayModifier, leafList, clippingAssembly, localRayOrigin);
            }
            currNode = 8;
            break;
        }
        }
    }
}

bool EmbeddedScan::findNeighborsBucketsDirected(const glm::dvec3& globalSeedPoint, const glm::dvec3& directedPoint, const double& radius, std::vector<std::vector<glm::dvec3>>& neighborList, const int& numberOfBuckets, const ClippingAssembly& globalClippingAssembly)
{
	glm::dvec3 localDirectedPoint = getLocalCoord(directedPoint);
	glm::dvec3 localSeedPoint = getLocalCoord(globalSeedPoint);
    ClippingAssembly localAssembly = globalClippingAssembly;
    localAssembly.clearMatrix(); // #423
    localAssembly.addTransformation(m_matrixToGlobal);

	std::vector<uint32_t> cellList;
	cellList.push_back(m_uRootCell);
	std::vector<uint32_t> leafList;
	int i = 0;
	while (i < (int)cellList.size())
	{
		for (int j = 0; j < 8; j++)
		{
			uint32_t currCell = m_vTreeCells[cellList[i]].m_children[j];
			if (!(currCell == NO_CHILD))
			{
				if (distancePointFromCell(localSeedPoint, currCell) < radius)
				{
					if (m_vTreeCells[currCell].m_isLeaf)
						leafList.push_back(currCell);
					else
						cellList.push_back(currCell);
				}
			}
		}
		i++;
	}
	/////////////relevant leaf listed, now iterate through the points ///////////////
	//Logger::log(LoggerMode::rayTracingLog) << "leaflist done,starting reading points : number of leafs : " << leafList.size() << Logger::endl;

	//if too many leafs, select some at random
	srand((unsigned int)time(NULL));
	int maxNumberOfLeaves(500);
	if (leafList.size() > maxNumberOfLeaves)
	{
		uint32_t seedCellId;
		if (pointToCell(localSeedPoint, seedCellId))
		{
			std::vector<uint32_t> newLeafList;
			newLeafList.push_back(seedCellId);
			double p = (double)maxNumberOfLeaves / (double)leafList.size();
			for (int i = 0; i < (int)leafList.size(); i++)
			{
				int v3 = rand() % maxNumberOfLeaves;
				if ((v3 / (double)maxNumberOfLeaves) < p)
					newLeafList.push_back(leafList[i]);

			}
			leafList = newLeafList;
		}
	}
    std::vector<double> bucketsLimitRatios;
    if (numberOfBuckets > 3)
    {
        bucketsLimitRatios.push_back(0.1);
        bucketsLimitRatios.push_back(0.2);
        bucketsLimitRatios.push_back(0.3);
        bucketsLimitRatios.push_back(0.5);
        bucketsLimitRatios.push_back(1);
    }

    std::vector<glm::dvec3> decodedPoints;
    getDecodedPoints(leafList, decodedPoints, false);

	for (const glm::dvec3& localPoint : decodedPoints)
	{
        glm::dvec4 point4 = glm::dvec4(localPoint, 1.0);
        if (!localAssembly.testPoint(point4))
        {
            continue;
        }
		if (glm::dot(localDirectedPoint - localPoint, localDirectedPoint - localSeedPoint) < 0)
			continue;
		double distance = glm::length(localPoint - localSeedPoint);
        if (numberOfBuckets < 4)
        {
            if (distance < radius)
            {
                int bucket = (int)((double)(distance * numberOfBuckets) / radius);
                /*if ((bucket == 0) && (numberOfBuckets > 3))
                {
                    if (((int)((double)(distance * numberOfBuckets * 3) / radius)) < 1)
                        neighborList[bucket].push_back(getGlobalCoord(localPoint));
                    else
                        neighborList[1].push_back(getGlobalCoord(localPoint));
                }
                else */
                neighborList[bucket].push_back(getGlobalCoord(localPoint));
            }
        }
        else
        {
            for (int bucket = 0; bucket < numberOfBuckets; bucket++)
            {
                if (distance < (bucketsLimitRatios[bucket] * radius))
                {
                    neighborList[bucket].push_back(getGlobalCoord(localPoint));
                    break;
                }
            }
        }
	}

	// display buckets : //
	/*Logger::log(LoggerMode::rayTracingLog) << "DISPLAYING BUCKETS : " << Logger::endl;
	for (int loop = 0; loop < neighborList.size(); loop++)
	{
		Logger::log(LoggerMode::rayTracingLog) << "bucket " << loop << " : number of points : " << neighborList[loop].size() << Logger::endl;
	}*/

	return true;
}
bool EmbeddedScan::findNeighborsBuckets(const glm::dvec3& globalSeedPoint, const double& radius, std::vector<std::vector<glm::dvec3>>& neighborList, const int& numberOfBuckets, const ClippingAssembly& globalClippingAssembly)
{
	glm::dvec3 localSeedPoint = getLocalCoord(globalSeedPoint);
    ClippingAssembly localAssembly = globalClippingAssembly;
    localAssembly.clearMatrix(); // #423
    localAssembly.addTransformation(m_matrixToGlobal);
	std::vector<uint32_t> cellList;
	cellList.push_back(m_uRootCell);
	std::vector<uint32_t> leafList;
	int i = 0;
	while (i < (int)cellList.size())
	{
		for (int j = 0; j < 8; j++)
		{
			uint32_t currCell = m_vTreeCells[cellList[i]].m_children[j];
			if (!(currCell == NO_CHILD))
			{
				if (distancePointFromCell(localSeedPoint, currCell) < radius)
				{
					if (m_vTreeCells[currCell].m_isLeaf)
						leafList.push_back(currCell);
					else
						cellList.push_back(currCell);
				}
			}
		}
		i++;
	}

	/////////////relevant leaf listed, now iterate through the points ///////////////
	//Logger::log(LoggerMode::rayTracingLog) << "leaflist done,starting reading points : number of leafs : " << leafList.size() << Logger::endl;

	//if too many leafs, select some at random
	srand((unsigned int)time(NULL));
	int maxNumberOfLeaves(500);

	if (leafList.size() > maxNumberOfLeaves)
	{
		uint32_t seedCellId;
		if (pointToCell(localSeedPoint, seedCellId))
		{
			std::vector<uint32_t> newLeafList;
			newLeafList.push_back(seedCellId);
			double p = (double)maxNumberOfLeaves / (double)leafList.size();
			for (int i = 0; i < (int)leafList.size(); i++)
			{
				int v3 = rand() % maxNumberOfLeaves;
				if ((v3 / (double)maxNumberOfLeaves) < p)
					newLeafList.push_back(leafList[i]);
			}
			leafList = newLeafList;
		}

	}

    std::vector<glm::dvec3> decodedPoints;
    getDecodedPoints(leafList, decodedPoints, false);

	double maxDist(0);
    for (const glm::dvec3& localPoint : decodedPoints)
    {
        glm::dvec4 point4 = glm::dvec4(localPoint, 1.0);
        if (!localAssembly.testPoint(point4))
        {
            continue;
        }
        double distance = glm::length(localPoint - localSeedPoint);
		if (distance > maxDist)
			maxDist = distance;
        if (distance < radius)
        {
            int bucket = (int)((double)(distance * numberOfBuckets) / radius);
			if ((bucket == 0)&&(numberOfBuckets>3))
			{
				if (((int)((double)(distance * numberOfBuckets*3) / radius)) < 1)
					neighborList[bucket].push_back(getGlobalCoord(localPoint));
				else
					neighborList[1].push_back(getGlobalCoord(localPoint));
			}
            else neighborList[bucket].push_back(getGlobalCoord(localPoint));
        }
    }

    // display buckets : //
    /*Logger::log(LoggerMode::rayTracingLog) << "DISPLAYING BUCKETS : " << Logger::endl;
    for (int loop = 0; loop < neighborList.size(); loop++)
    {
        Logger::log(LoggerMode::rayTracingLog) << "bucket " << loop << " : number of points : " << neighborList[loop].size() << Logger::endl;
    }*/

    return true;
}

//for testing//
bool EmbeddedScan::findNeighborsBucketsTest(const glm::dvec3& globalSeedPoint, const double& radius, std::vector<std::vector<glm::dvec3>>& neighborList, const int& numberOfBuckets, const ClippingAssembly& globalClippingAssembly, std::vector<glm::dquat>& rotations, std::vector<glm::dvec3>& positions, std::vector<double>& scales)
{
	glm::dvec3 localSeedPoint = getLocalCoord(globalSeedPoint);
    ClippingAssembly localAssembly = globalClippingAssembly;
    localAssembly.clearMatrix(); // #423
    localAssembly.addTransformation(m_matrixToGlobal);
	std::vector<uint32_t> cellList;
	cellList.push_back(m_uRootCell);
	std::vector<uint32_t> leafList;
	int i = 0;
	while (i < (int)cellList.size())
	{
		for (int j = 0; j < 8; j++)
		{
			uint32_t currCell = m_vTreeCells[cellList[i]].m_children[j];
			if (!(currCell == NO_CHILD))
			{
				if (distancePointFromCell(localSeedPoint, currCell) < radius)
				{
					if (m_vTreeCells[currCell].m_isLeaf)
						leafList.push_back(currCell);
					else
						cellList.push_back(currCell);
				}
			}
		}
		i++;
	}
	
	/////////////relevant leaf listed, now iterate through the points ///////////////
	//Logger::log(LoggerMode::rayTracingLog) << "leaflist done,starting reading points : number of leafs : " << leafList.size() << Logger::endl;

	//if too many leafs, select some at random
	srand((unsigned int)time(NULL));
	int maxNumberOfLeaves(500);

	if (leafList.size() > maxNumberOfLeaves)
	{
		uint32_t seedCellId;
		if (pointToCell(localSeedPoint, seedCellId))
		{
			std::vector<uint32_t> newLeafList;
			newLeafList.push_back(seedCellId);
			double p = (double)maxNumberOfLeaves / (double)leafList.size();
			for (int i = 0; i < (int)leafList.size(); i++)
			{
				int v3 = rand() % maxNumberOfLeaves;
				if ((v3 / (double)maxNumberOfLeaves) < p)
					newLeafList.push_back(leafList[i]);
			}
			leafList = newLeafList;
		}
		
	}
	//Logger::log(LoggerMode::rayTracingLog) << "compressed leafList " << leafList.size() << Logger::endl;
	createBoxFromLeaves(leafList, rotations, positions, scales);

    std::vector<glm::dvec3> decodedPoints;
    getDecodedPoints(leafList, decodedPoints, false);

    double maxDist(0);
    for (const glm::dvec3& localPoint : decodedPoints)
    {
        glm::dvec4 point4 = glm::dvec4(localPoint, 1.0);
        if (!localAssembly.testPoint(point4))
        {
            continue;
        }

		double distance = glm::length(localPoint - localSeedPoint);
		if (distance > maxDist)
			maxDist = distance;
		if (distance < radius)
		{
			int bucket = (int)((double)(distance * numberOfBuckets) / radius);
			if ((bucket == 0) && (numberOfBuckets > 3))
			{
				if (((int)((double)(distance * numberOfBuckets*0.5) / radius)) < 1)
					neighborList[bucket].push_back(getGlobalCoord(localPoint));
				else
					neighborList[1].push_back(getGlobalCoord(localPoint));
			}
			else neighborList[bucket].push_back(getGlobalCoord(localPoint));
		}
	}

	// display buckets : //
	/*Logger::log(LoggerMode::rayTracingLog) << "DISPLAYING BUCKETS : " << Logger::endl;
	for (int loop = 0; loop < neighborList.size(); loop++)
	{
		Logger::log(LoggerMode::rayTracingLog) << "bucket " << loop << " : number of points : " << neighborList[loop].size() << Logger::endl;
	}*/

	return true;
}
bool EmbeddedScan::findNeighbors(const glm::dvec3& globalSeedPoint, const double& radius, std::vector<glm::dvec3>& neighborList, const ClippingAssembly& clippingAssembly)
{
    //Logger::log(LoggerMode::rayTracingLog) << "findNeighbor start" << Logger::endl;

    std::vector<std::vector<glm::dvec3>> temp(1);
    if (findNeighborsBuckets(globalSeedPoint, radius, temp, 1, clippingAssembly))
    {
        neighborList.insert(neighborList.end(), temp[0].begin(), temp[0].end());
        //Logger::log(LoggerMode::rayTracingLog) << "findNeighbor end" << Logger::endl;

        return true;
    }
    else return false;
}

bool EmbeddedScan::findNeighborsTowardsPoint(const glm::dvec3& globalSeedPoint, const glm::dvec3& targetPoint, const double& radius, std::vector<glm::dvec3>& neighborList, const ClippingAssembly& clippingAssembly)
{
    glm::dvec3 localSeedPoint = getLocalCoord(globalSeedPoint+0.05*(targetPoint-globalSeedPoint));
    glm::dvec3 localTargetPoint = getLocalCoord(targetPoint);
    ClippingAssembly localAssembly = clippingAssembly;
    localAssembly.clearMatrix(); // #423
    localAssembly.addTransformation(m_matrixToGlobal);
    std::vector<uint32_t> cellList;
    cellList.push_back(m_uRootCell);
    std::vector<uint32_t> leafList;
    int i = 0;
    while (i < (int)cellList.size())
    {
        for (int j = 0; j < 8; j++)
        {
            uint32_t currCell = m_vTreeCells[cellList[i]].m_children[j];
            if (!(currCell == NO_CHILD))
            {
                if (distancePointFromCell(localSeedPoint, currCell) < radius)
                {
                    if (m_vTreeCells[currCell].m_isLeaf)
                        leafList.push_back(currCell);
                    else
                        cellList.push_back(currCell);
                }
            }
        }
        i++;
    }

    /////////////relevant leaf listed, now iterate through the points ///////////////
    //Logger::log(LoggerMode::rayTracingLog) << "leaflist done,starting reading points : number of leafs : " << leafList.size() << Logger::endl;

    //if too many leafs, select some at random
    srand((unsigned int)time(NULL));
    int maxNumberOfLeaves(500);

    if (leafList.size() > maxNumberOfLeaves)
    {
        uint32_t seedCellId;
        if (pointToCell(localSeedPoint, seedCellId))
        {
            std::vector<uint32_t> newLeafList;
            newLeafList.push_back(seedCellId);
            double p = (double)maxNumberOfLeaves / (double)leafList.size();
            for (int i = 0; i < (int)leafList.size(); i++)
            {
                int v3 = rand() % maxNumberOfLeaves;
                if ((v3 / (double)maxNumberOfLeaves) < p)
                    newLeafList.push_back(leafList[i]);
            }
            leafList = newLeafList;
        }

    }

    std::vector<glm::dvec3> decodedPoints;
    getDecodedPoints(leafList, decodedPoints, false);

    double maxDist(0);
    glm::dvec3 dir(localTargetPoint - localSeedPoint);
    dir = dir / glm::length(dir);

    for (const glm::dvec3& localPoint : decodedPoints)
    {
        glm::dvec4 point4 = glm::dvec4(localPoint, 1.0);
        if (!localAssembly.testPoint(point4))
        {
            continue;
        }
        double distance = glm::length(localPoint - localSeedPoint);
        if (distance > maxDist)
            maxDist = distance;
        if (distance < radius)
        {
            glm::dvec3 currDir(localPoint - localSeedPoint);
            currDir = currDir / glm::length(currDir);
            if (glm::dot(dir, currDir) > 0.8)
                neighborList.push_back(getGlobalCoord(localPoint));
        }
    }

    // display buckets : //
    /*Logger::log(LoggerMode::rayTracingLog) << "DISPLAYING BUCKETS : " << Logger::endl;
    for (int loop = 0; loop < neighborList.size(); loop++)
    {
        Logger::log(LoggerMode::rayTracingLog) << "bucket " << loop << " : number of points : " << neighborList[loop].size() << Logger::endl;
    }*/

    return true;
    
}

bool EmbeddedScan::nearestNeighbor(const glm::dvec3& globalPoint, glm::dvec3& result)
{
    uint32_t cellId_0, cellId_1;
    uint32_t decodedIndex(0);
    glm::dvec3 localPoint = getLocalCoord(globalPoint);
    glm::dvec3 localPoint1 = pointInNeighborCell(localPoint);
    bool test = pointToCell(localPoint, cellId_0);
    bool test1 = pointToCell(localPoint1, cellId_1);
	bool success = false;
    if (!test) {
        Logger::log(LoggerMode::rayTracingLog) << "nearest neighbor is false" << Logger::endl;
        return false;
    }
    std::vector<uint32_t> cellsId;
    cellsId.push_back(cellId_0);
    cellsId.push_back(cellId_1);
    double minDistance(DBL_MAX);

    std::vector<glm::dvec3> localPointList;
    getDecodedPoints(cellsId, localPointList, false);

	for (const glm::dvec3& localCoord : localPointList)
	{
		double distance = glm::length(localCoord - localPoint);
		if (distance < minDistance)
		{
			minDistance = distance;
			result = getGlobalCoord(localCoord);
			success = true;
		}
	}

    return success;
}

bool EmbeddedScan::nearestNeighbor2(const glm::dvec3& globalPoint, glm::dvec3& result, const ClippingAssembly& clippingAssembly)
{
	bool success = false;
	double radius = 0.01;
	std::vector<glm::dvec3> neighborList;
	findNeighbors(globalPoint, radius, neighborList, clippingAssembly);
	if (neighborList.size() > 0)
	{
		success = true;
		double minDist = DBL_MAX;
		for (int i = 0; i < (int)neighborList.size(); i++)
		{
			if (glm::length(globalPoint - neighborList[i]) < minDist)
			{
				result = neighborList[i];
				minDist = glm::length(globalPoint - neighborList[i]);
			}
		}
	}
	return success;
}

bool EmbeddedScan::nearestBendNeighbor(const glm::dvec3& globalPoint, glm::dvec3& result, const glm::dvec3& normalVector)
{
    uint32_t cellId;
    glm::dvec3 localPoint = getLocalCoord(globalPoint);
    if (!pointToCell(localPoint, cellId)) { return false; }

    double traversalDistanceThreshold(0.01), maxBendDistance(-0.03);

    std::vector<uint32_t> leaves = { cellId };
    std::vector<glm::dvec3> decodedPoints;
    getDecodedPoints(leaves, decodedPoints, false);

    /*for (int j = 0; j < cell.m_layerIndexes[cell.m_depthSPT]; j++)
    {
        glm::dvec3 currPoint(localPoint_[j].x, localPoint_[j].y, localPoint_[j].z);
        double distance = glm::length(currPoint - localPoint);
        if (distance < minDistance)
        {
            minDistance = distance;
        }
    }
    for (int j = 0; j < cell.m_layerIndexes[cell.m_depthSPT]; j++)
    {
        glm::dvec3 currPoint(localPoint_[j].x, localPoint_[j].y, localPoint_[j].z);
        double distance = glm::length(currPoint - localPoint);
        if (distance < 10 * minDistance)
        {
            double traversalDistance = glm::dot(normalVector, currPoint - localPoint);
            if (traversalDistance < minTransversalDistance)
            {
                minTransversalDistance = traversalDistance;
            }

        }
    }
    bool success(false);
    for (int j = 0; j < cell.m_layerIndexes[cell.m_depthSPT]; j++)
    {
        glm::dvec3 currPoint(localPoint_[j].x, localPoint_[j].y, localPoint_[j].z);
        double distance = glm::length(currPoint - localPoint);
        double bendDistance = glm::dot(currPoint - localPoint, normalVector);
        double traversalDistance =glm::length(currPoint-localPoint-bendDistance*normalVector);
        if ((distance < 2 * minDistance) && (traversalDistance < 0.03))
        {
            if (bendDistance > maxBendDistance)
            {
                success = true;
                maxBendDistance = bendDistance;
                result = pointLocalToGlobal(currPoint, m_globalMatrix);
            }
        }
    }*/
    bool success(false), partialSuccess(false);
    for (const glm::dvec3& currPoint : decodedPoints)
    {
        double bendDistance = glm::dot(currPoint - localPoint, normalVector);
        double traversalDistance = glm::length(currPoint - localPoint - bendDistance * normalVector);
        //double distance = sqrt(bendDistance*bendDistance + alpha * traversalDistance*traversalDistance);
        //double distance = alpha * traversalDistance - bendDistance;
        if (traversalDistance < traversalDistanceThreshold)
        {
            partialSuccess = true;
            if (bendDistance > maxBendDistance)
            {
                success = true;
                maxBendDistance = bendDistance;
                result = getGlobalCoord(currPoint);
            }
        }
    }
    if ((partialSuccess) && (!success))
    {
        Logger::log(LoggerMode::rayTracingLog) << "partial success" << Logger::endl;
        //displayCellInfo(cell);
    }
    return success;
}

glm::dvec3 EmbeddedScan::pointInNeighborCell(const glm::dvec3& localPoint)
{
    uint32_t cellId;
	if (!pointToCell(localPoint, cellId))
		return localPoint;
    const TreeCell& cell = m_vTreeCells[cellId];
    double distanceToBorder(DBL_MAX);
    int whichCase(0);
    if ((localPoint.x - cell.m_position[0]) < distanceToBorder)
    {
        distanceToBorder = localPoint.x - cell.m_position[0];
    }
    if ((-localPoint.x + cell.m_position[0] + cell.m_size) < distanceToBorder)
    {
        distanceToBorder = -localPoint.x + cell.m_position[0] + cell.m_size;
        whichCase = 1;
    }
    if ((localPoint.y - cell.m_position[1]) < distanceToBorder)
    {
        distanceToBorder = localPoint.y - cell.m_position[1];
        whichCase = 2;
    }
    if ((-localPoint.y + cell.m_position[1] + cell.m_size) < distanceToBorder)
    {
        distanceToBorder = -localPoint.y + cell.m_position[1] + cell.m_size;
        whichCase = 3;
    }
    if ((localPoint.z - cell.m_position[2]) < distanceToBorder)
    {
        distanceToBorder = localPoint.z - cell.m_position[2];
        whichCase = 4;
    }
    if ((-localPoint.z + cell.m_position[2] + cell.m_size) < distanceToBorder)
    {
        distanceToBorder = -localPoint.z + cell.m_position[2] + cell.m_size;
        whichCase = 5;
    }
    glm::dvec3 result;
    switch (whichCase) {
    case 0:
	{
		result = localPoint - (distanceToBorder + 0.002)*glm::dvec3(1.0, 0.0, 0.0);
		break;
	}
    case 1:
	{
		result = localPoint + (distanceToBorder + 0.002)*glm::dvec3(1.0, 0.0, 0.0);
		break;
	}
    case 2:
	{
		result = localPoint - (distanceToBorder + 0.002)*glm::dvec3(0.0, 1.0, 0.0);
		break;
	}
    case 3:
	{
		result = localPoint + (distanceToBorder + 0.002)*glm::dvec3(0.0, 1.0, 0.0);
		break;
	}
    case 4:
	{
		result = localPoint - (distanceToBorder + 0.002)*glm::dvec3(0.0, 0.0, 1.0);
		break;
	}
    case 5:
        result = localPoint + (distanceToBorder + 0.002)*glm::dvec3(0.0, 0.0, 1.0);
    }
    Logger::log(LoggerMode::rayTracingLog) << "neighborCell : case " << whichCase << Logger::endl;

    return result;
}

bool EmbeddedScan::pointToCell(const glm::dvec3& localPoint, uint32_t& resultCellId) const
{
    resultCellId = m_uRootCell;  //startFromRoot

    if (!OctreeRayTracing::isPointInCell(localPoint, m_vTreeCells[resultCellId]))
    {
        //Logger::log(LoggerMode::FunctionLog) << "problem : point outside octree" << Logger::endl;
        return false;
    }
    else {

        while (!m_vTreeCells[resultCellId].m_isLeaf)
        {
            const TreeCell& currentCell = m_vTreeCells[resultCellId];
            int childIndex(0);

            if (localPoint.x > ((double)currentCell.m_position[0] + currentCell.m_size / 2.f))
                { childIndex += 4; }
            if (localPoint.y > ((double)currentCell.m_position[1] + currentCell.m_size / 2.f))
                { childIndex += 2; }
            if (localPoint.z > ((double)currentCell.m_position[2] + currentCell.m_size / 2.f))
                { childIndex += 1; }

            resultCellId = currentCell.m_children[childIndex];
            if (resultCellId == NO_CHILD)
            {
                //Logger::log(LoggerMode::FunctionLog) << "problem in PointToCell : " << Logger::endl;

                return false;
            }
        }
    }
    return true;
}

void EmbeddedScan::getPointsInLeaf(const glm::dvec3& globalPoint, std::vector<glm::dvec3>& retPoints)
{
    glm::dvec3 localPoint = getLocalCoord(globalPoint);
    uint32_t cellId;
    pointToCell(localPoint, cellId);

    getDecodedPoints({ cellId }, retPoints, true /*global*/);
}

std::vector<glm::dvec3> EmbeddedScan::getPointsInBox(const glm::dvec3& seedPoint, const glm::dvec3& beamDir, const glm::dvec3& orthoDir, const glm::dvec3& normalDir, const std::vector<std::vector<double>>& xyRange, const double& heightMax)
{
    std::vector<glm::dvec3> result;
    std::vector<glm::dvec3> samplePoints;
    double normalStep(0.01);

    int dirIndex, orthoIndex, heightIndex;
    dirIndex = (int)((xyRange[0][1] - xyRange[0][0]) / normalStep);
    orthoIndex = (int)((xyRange[1][1] - xyRange[1][0]) / normalStep);
    heightIndex = (int)(heightMax / normalStep);
    for (int dir = 0; dir < dirIndex; dir++)
    {
        for (int ortho = 0; ortho < orthoIndex; ortho++)
        {
            for (int height = 0; height < heightIndex; height++)
            {
                glm::dvec3 currPoint = seedPoint + ((xyRange[0][1] - xyRange[0][0])*dir / (double)dirIndex + xyRange[0][0])*beamDir + ((xyRange[1][1] - xyRange[1][0])*dir / (double)orthoIndex + xyRange[1][0])*orthoDir + (height / (double)heightIndex *heightMax)*normalDir;
                samplePoints.push_back(currPoint);
            }
        }
    }
    std::vector<uint32_t> leafList;
    std::vector<glm::dvec3> currPoints;
    for (int i = 0; i < (int)samplePoints.size(); i++)
    {
        glm::dvec3 localPoint = getLocalCoord(samplePoints[i]);
        bool redundantPoint(false);
        for (int i = 0; i < leafList.size(); i++)
        {
            if (OctreeRayTracing::isPointInCell(samplePoints[i], m_vTreeCells[leafList[i]]))
            {
                redundantPoint = true;
                break;
            }
        }
        if (!redundantPoint)
        {
            uint32_t cellId;
            if (pointToCell(localPoint, cellId))
                leafList.push_back(cellId);
        }
    }

    //if too many leafs, select some at random
    srand((unsigned int)time(NULL));
    int maxNumberOfLeaves(500);
    if (leafList.size() > maxNumberOfLeaves)
    {
        std::vector<uint32_t> newLeafList;
        double p = (double)maxNumberOfLeaves / (double)leafList.size();
        for (int i = 1; i < (int)leafList.size(); i++)
        {
            int v3 = rand() % maxNumberOfLeaves;
            if ((v3 / (double)maxNumberOfLeaves) < p)
                newLeafList.push_back(leafList[i]);

        }
        leafList = newLeafList;
    }

    std::vector<glm::dvec3> points;
    getDecodedPoints(leafList, points, false);

    for (const glm::dvec3& localCoord : points)
    {
        glm::dvec3 currPoint = getGlobalCoord(localCoord);
        double pointHeight, pointDir, pointOrtho;
        pointHeight = glm::dot(normalDir, currPoint - seedPoint);
        pointDir = glm::dot(beamDir, currPoint - seedPoint);
        pointOrtho = glm::dot(orthoDir, currPoint - seedPoint);
        if ((pointHeight > 0) && (pointHeight < heightMax) && (pointDir > xyRange[0][0]) && (pointDir < xyRange[0][1]) && (pointOrtho > xyRange[1][0]) && (pointOrtho < xyRange[1][1]))
            result.push_back(currPoint);
    }
    return result;
}

std::vector<glm::dvec3> EmbeddedScan::getPointsInGeometricBox(const GeometricBox& box, const ClippingAssembly& clippingAssembly)
{
    glm::dvec3 seedPoint = box.getCenter()-box.getRadius()*box.getDirZ();
    std::vector<std::vector<double>> xyRange;
    std::vector<double> temp(0);
    temp.push_back(-box.getRadius());
    temp.push_back(box.getRadius());
    xyRange.push_back(temp);
    xyRange.push_back(temp);

    std::vector<glm::dvec3> result(0);
    result = getPointsInBox(seedPoint, box.getDirX(), box.getDirY(), box.getDirZ(), xyRange, box.getRadius());
    return result;

    /*std::vector<glm::dvec3> result;

    double step(0.01);
    std::vector<glm::dvec3> samplePoints;
    glm::dvec3 dirX(box.getDirX()), dirY(box.getDirY()), dirZ(box.getDirZ());
    int sizeX((int)(glm::length(box.m_corners[1] - box.m_corners[0]) / step)), sizeY((int)(glm::length(box.m_corners[2] - box.m_corners[0]) / step)), sizeZ((int)(glm::length(box.m_corners[3] - box.m_corners[0]) / step));

    for (int i=0;i<sizeX;i++)
    {
        for (int j=0;j<sizeY;j++)
        {
            for (int k=0;k<sizeZ;k++)
            {
                glm::dvec3 currPoint = box.m_corners[0] + step * ((double)i * dirX + (double)j * dirY + (double)k * dirZ);
                samplePoints.push_back(currPoint);
            }
        }
    }
    std::vector<uint32_t> leafList;
    std::vector<glm::dvec3> currPoints;
    for (int i = 0; i < (int)samplePoints.size(); i++)
    {
        glm::dvec3 localPoint = getLocalCoord(samplePoints[i]);
        bool redundantPoint(false);
        for (int j = 0; j < leafList.size(); j++)
        {
            if (OctreeRayTracing::isPointInCell(localPoint, m_vTreeCells[leafList[j]]))
            {
                redundantPoint = true;
                break;
            }
        }
        if (!redundantPoint)
        {
            uint32_t cellId;
            if (pointToCell(localPoint, cellId))
                leafList.push_back(cellId);
        }
    }

    //if too many leafs, select some at random
    srand((unsigned int)time(NULL));
    int maxNumberOfLeaves(500);
    if (leafList.size() > maxNumberOfLeaves)
    {
        std::vector<uint32_t> newLeafList;
        double p = (double)maxNumberOfLeaves / (double)leafList.size();
        for (int i = 1; i < (int)leafList.size(); i++)
        {
            int v3 = rand() % maxNumberOfLeaves;
            if ((v3 / (double)maxNumberOfLeaves) < p)
                newLeafList.push_back(leafList[i]);

        }
        leafList = newLeafList;
    }

    std::vector<glm::dvec3> points;
    getDecodedPoints(leafList, points, false);

    for (const glm::dvec3& localCoord : points)
    {
        glm::dvec3 currPoint = getGlobalCoord(localCoord);
        if(box.isInside(currPoint))
            result.push_back(currPoint);
    }
    return result;*/
}

void EmbeddedScan::createBoxFromLeaves(const std::vector<uint32_t>& leafList, std::vector<glm::dquat>& rotations, std::vector<glm::dvec3>& positions, std::vector<double>& scales)
{
	for (int i = 0; i < (int)leafList.size(); i++)
	{
		TreeCell currCell = m_vTreeCells[leafList[i]];
		positions.push_back(getGlobalCoord( glm::dvec3(currCell.m_position[0]+0.5*currCell.m_size, currCell.m_position[1] + 0.5*currCell.m_size, currCell.m_position[2] + 0.5*currCell.m_size)));
		rotations.push_back(getGlobalCoord(glm::dvec3(0, 0, 1)));
		scales.push_back(currCell.m_size);
	}
}

double EmbeddedScan::distancePointFromCell(const glm::dvec3& localPoint, const uint32_t& cellId)
{
	TreeCell cell = m_vTreeCells[cellId];
	double dx, dy, dz;
	dx = std::max(localPoint[0]-cell.m_position[0]-cell.m_size, cell.m_position[0]-localPoint[0]);
	dx = std::max(dx, (double)0);
	dy = std::max(localPoint[1] - cell.m_position[1] - cell.m_size, cell.m_position[1] - localPoint[1]);
	dy = std::max(dy, (double)0);
	dz = std::max(localPoint[2] - cell.m_position[2] - cell.m_size, cell.m_position[2] - localPoint[2]);
	dz = std::max(dz, (double)0);
	return sqrt(dx*dx + dy * dy + dz * dz);
}

glm::dvec3 EmbeddedScan::getLocalCoord(const glm::dvec3& globalCoord) const
{
    return (glm::inverse(m_rotationToGlobal) * (globalCoord - m_translationToGlobal));
}

glm::dvec3 EmbeddedScan::getGlobalCoord(const glm::dvec3& localCoord) const
{
    return (m_rotationToGlobal * localCoord + m_translationToGlobal);
}



glm::dmat4 EmbeddedScan::getMatrixToLocal() const
{
    return (glm::dmat4(glm::inverse(m_rotationToGlobal)) *
        glm::dmat4(1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            -m_translationToGlobal.x, -m_translationToGlobal.y, -m_translationToGlobal.z, 1.0));
}

bool EmbeddedScan::updateVoxelGrid(const ClippingAssembly& clippingAssembly, VoxelGrid& voxelGrid, const int& scanNumber)
{
    //run through the octree
    std::vector<uint32_t> cellList;
    cellList.push_back(m_uRootCell);
    std::vector<uint32_t> leavesToDecode;
    int i = 0;
    int numberOfLeaves(0);
    while (i < (int)cellList.size())
    {
        for (int j = 0; j < 8; j++)
        {
            uint32_t currCellId = m_vTreeCells[cellList[i]].m_children[j];
            if (!(currCellId == NO_CHILD))
            {
                if (m_vTreeCells[currCellId].m_isLeaf)
                {
                    numberOfLeaves++;
                    //update voxelGrid with this leaf
                    int ambiguousVoxels(0);
                    if (!updateVoxelGridWithLeaf(voxelGrid, currCellId, scanNumber, ambiguousVoxels, clippingAssembly))
                    {
                        leavesToDecode.push_back(currCellId);
                    }                        
                    if (numberOfLeaves % 1000 == 0)
                        Logger::log(LoggerMode::rayTracingLog) << "leavesClassified : " << numberOfLeaves << Logger::endl;
                }
                else
                    cellList.push_back(currCellId);
            }
        }
        i++;
    }

  
    //decode the leaves one by one, stop when all ambiguous voxels are populated
   
    double meanPointPerLeaf(0), meanDecodedPointPerLeaf(0);
    //Logger::log(LoggerMode::rayTracingLog) << "total leaves to decode : " << (int)leavesToDecodeOneByOne.size() << Logger::endl;

    for (int i = 0; i < (int)leavesToDecode.size(); i++)
    {
        int ambiguousVoxels(0), solvedVoxels(0);
        if (!updateVoxelGridWithLeaf(voxelGrid, leavesToDecode[i], scanNumber,ambiguousVoxels, clippingAssembly))
        {
            std::vector<glm::dvec3> decodedPoints;
            std::vector<uint32_t> decodingLeaf(0);
            decodingLeaf.push_back(leavesToDecode[i]);
            samplePointsByStep((float)voxelGrid.m_voxelSize * (float)0.5, decodingLeaf, decodedPoints);
            int occupiedVoxels(0);
            for (int k = 0; k < (int)decodedPoints.size(); k++)
            {
                if (updateVoxelGridWithPoint(voxelGrid, decodedPoints[k], scanNumber,clippingAssembly))
                {
                    solvedVoxels++;
                    if (solvedVoxels==ambiguousVoxels)
                    {
                        break;
                    }
                } 

            }
        }
    }
 
    

    return true;
}

GeometricBox EmbeddedScan::createGeometricBoxFromLeaf(const uint32_t& leafId, const ClippingAssembly& clippingAssembly)
{
    TreeCell& leaf = m_vTreeCells[leafId];
    glm::dvec3 position = glm::dvec3(leaf.m_position[0], leaf.m_position[1], leaf.m_position[2]);
    double size = leaf.m_size;
    std::vector<glm::dvec3> corners;

    corners.push_back(OctreeRayTracing::getTransformationCoord(getGlobalCoord(position),clippingAssembly));
    corners.push_back(OctreeRayTracing::getTransformationCoord(getGlobalCoord(position + size * glm::dvec3(1.0, 0.0, 0.0)),clippingAssembly));
    corners.push_back(OctreeRayTracing::getTransformationCoord(getGlobalCoord(position + size * glm::dvec3(0.0, 1.0, 0.0)), clippingAssembly));
    corners.push_back(OctreeRayTracing::getTransformationCoord(getGlobalCoord(position + size * glm::dvec3(0.0, 0.0, 1.0)), clippingAssembly));
    corners.push_back(OctreeRayTracing::getTransformationCoord(getGlobalCoord(position + size * glm::dvec3(1.0, 1.0, 0.0)), clippingAssembly));
    corners.push_back(OctreeRayTracing::getTransformationCoord(getGlobalCoord(position + size * glm::dvec3(1.0, 0.0, 1.0)), clippingAssembly));
    corners.push_back(OctreeRayTracing::getTransformationCoord(getGlobalCoord(position + size * glm::dvec3(0.0, 1.0, 1.0)), clippingAssembly));
    corners.push_back(OctreeRayTracing::getTransformationCoord(getGlobalCoord(position + size * glm::dvec3(1.0, 1.0, 1.0)), clippingAssembly));
    GeometricBox leafBox(corners);
    return leafBox;
}

bool EmbeddedScan::updateVoxelGridWithLeaf(VoxelGrid& voxelGrid, const uint32_t& leafId, const int& scanNumber, int& numberOfambiguousVoxels, const ClippingAssembly& clippingAssembly)
{
    //returns false if leaf should be decoded, true otherwise
    //should ignore if the leaf is too far away from the origin
    //maybe not ? otherwise, points far away could be false negatives, since we don't tag an empty voxel as dynamic
    GeometricBox leafBox = createGeometricBoxFromLeaf(leafId,clippingAssembly);
    glm::dvec3 scanOrigin = OctreeRayTracing::getTransformationCoord(getGlobalCoord(glm::dvec3(0.0, 0.0, 0.0)),clippingAssembly);
    //if (glm::length(leafBox.getCenter() - scanOrigin) - leafBox.getRadius() > 12)
    //    return true;
    bool result = true;
    TreeCell& leaf = m_vTreeCells[leafId];
    glm::dvec3 position = glm::dvec3(leaf.m_position[0], leaf.m_position[1], leaf.m_position[2]);
    glm::dvec3 globalPosition = OctreeRayTracing::getTransformationCoord(getGlobalCoord(position),clippingAssembly);
    
    double xMin, xMax, yMin, yMax, zMin, zMax;
    
    xMin = globalPosition[0];
    xMax = globalPosition[0];
    yMin = globalPosition[1];
    yMax = globalPosition[1];
    zMin = globalPosition[2];
    zMax = globalPosition[2];

    for (int t = 0; t < 8; t++)
    {
        if (leafBox.m_corners[t][0] < xMin)
            xMin = leafBox.m_corners[t][0];
        if (leafBox.m_corners[t][0] > xMax)
            xMax = leafBox.m_corners[t][0];
        if (leafBox.m_corners[t][1] < yMin)
            yMin = leafBox.m_corners[t][1];
        if (leafBox.m_corners[t][1] > yMax)
            yMax = leafBox.m_corners[t][1];
        if (leafBox.m_corners[t][2] < zMin)
            zMin = leafBox.m_corners[t][2];
        if (leafBox.m_corners[t][2] > zMax)
            zMax = leafBox.m_corners[t][2];
    }

    int xPosMin, xPosMax, yPosMin, yPosMax, zPosMin, zPosMax;
    double voxelSize = voxelGrid.m_voxelSize;
    
    xPosMin = (int)((xMin - voxelGrid.m_xMin) / voxelSize);
    xPosMax = (int)((xMax - voxelGrid.m_xMin) / voxelSize);
    yPosMin = (int)((yMin - voxelGrid.m_yMin) / voxelSize);
    yPosMax = (int)((yMax - voxelGrid.m_yMin) / voxelSize);
    zPosMin = (int)((zMin - voxelGrid.m_zMin) / voxelSize);
    zPosMax = (int)((zMax - voxelGrid.m_zMin) / voxelSize);
    int cellsToTry = (xPosMax - xPosMin) * (yPosMax - yPosMin) * (zPosMax - zPosMin);
    if (cellsToTry > 50)
    {
        numberOfambiguousVoxels = cellsToTry;
        return false;
    }
    for (int i = xPosMin; i < (xPosMax + 1); i++)
    {
        if ((i < 0) || (i >= voxelGrid.m_sizeX))
            continue;
        else {
            for (int j = yPosMin; j < (yPosMax + 1); j++)
                if ((j < 0) || (j >= voxelGrid.m_sizeY))
                    continue;
                else {
                    for (int k = zPosMin; k < (zPosMax + 1); k++)
                        if ((k < 0) || (k >= voxelGrid.m_sizeZ))
                            continue;
                        else
                        {
                            //ignore if the voxel is already tagged as occupied
                            if (voxelGrid.isVoxelOccupied(i, j, k, scanNumber))
                                continue;
                            
                            GeometricBox gridBox = voxelGrid.getBoxFromCoordinates(i, j, k);
                            int relationResult = OctreeRayTracing::relateBoxToOtherBox(leafBox, gridBox);
                            switch (relationResult)
                            {
                            case 0:
                            {
                                //leafBox inside gridBox

                                //tag this voxel as occupied
                                voxelGrid.m_grid[i][j][k] = voxelGrid.m_grid[i][j][k]|(1 << scanNumber);
                                break;
                            }
                            case 1:
                            {
                                //gridBox inside leafBox
                                //add leaf to leavesToDecode
                                numberOfambiguousVoxels++;
                                result = false;
                                break;
                            }
                            case 2:
                            {
                                //boxes intersect
                                //add leaf to leavesToDecode
                                numberOfambiguousVoxels++;
                                result = false;
                                break;
                            }
                            case 3:
                            {
                                //boxes don't intersect
                                break;
                            }
                            }
                        }
                }
        }
    }
    return result;
}

bool EmbeddedScan::updateVoxelGridWithPoint(VoxelGrid& voxelGrid, const glm::dvec3& point, const int& scanNumber, const ClippingAssembly& clippingAssembly)
{
    //returns true iff it modified voxelGrid
    int i, j, k;
    glm::dvec3 truePoint = OctreeRayTracing::getTransformationCoord(point, clippingAssembly);
    voxelGrid.voxelCoordinatesOfPoint(truePoint, i, j, k);
    if(voxelGrid.areVoxelCoordinatesValid(i,j,k))
        if (!voxelGrid.isVoxelOccupied(i, j, k, scanNumber))
        {
            voxelGrid.m_grid[i][j][k] += 1 << scanNumber;
            return true;
        }
    return false;
}

bool EmbeddedScan::classifyVoxelsByScan(VoxelGrid& voxelGrid, const ClippingAssembly& clippingAssembly, std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, const int& scanNumber)
{
    std::vector<bool> temp(voxelGrid.m_sizeZ,false);
    std::vector<std::vector<bool>> temp1(voxelGrid.m_sizeY,temp);
    std::vector<std::vector<std::vector<bool>>> overrideDynamic(voxelGrid.m_sizeX,temp1);
    glm::dvec3 scanOrigin = OctreeRayTracing::getTransformationCoord(getGlobalCoord(glm::dvec3(0.0, 0.0, 0.0)),clippingAssembly);
    double voxelSize = voxelGrid.m_voxelSize;
    int xCenter, yCenter, zCenter;
    voxelGrid.voxelCoordinatesOfPoint(scanOrigin, xCenter, yCenter, zCenter);
    //run through the octree
    std::vector<uint32_t> cellList;
    cellList.push_back(m_uRootCell);
    std::vector<uint32_t> leavesToDecode;
    int i = 0;
    int leavesClassified(0);
    int totalDecodedPoints(0);
    auto start = high_resolution_clock::now();

    while (i < (int)cellList.size())
    {
        for (int j = 0; j < 8; j++)
        {
            uint32_t currCellId = m_vTreeCells[cellList[i]].m_children[j];
            if (!(currCellId == NO_CHILD))
            {
                if (m_vTreeCells[currCellId].m_isLeaf)
                {
                    //classify voxels with this leaf
                    totalDecodedPoints+=classifyVoxelsWithLeaf(currCellId, voxelGrid, dynamicVoxels, overrideDynamic, scanNumber,scanOrigin, clippingAssembly);
                    leavesClassified++;
                    if (leavesClassified % 1000 == 0)
                        Logger::log(LoggerMode::rayTracingLog) << "leavesClassified : " << leavesClassified << Logger::endl;

                }
                else
                    cellList.push_back(currCellId);
            }
        }
        i++;
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);
    Logger::log(LoggerMode::rayTracingLog) << "total decoded points : " << totalDecodedPoints << Logger::endl;

    Logger::log(LoggerMode::rayTracingLog) << "points decoded /time : " << (double)(totalDecodedPoints/duration.count()) << Logger::endl;

    //change dynamic to take into account the override
    int numberOfOverrides(0),numberOfDynamicVoxels(0);
   
    for (int i = 0; i < voxelGrid.m_sizeX; i++)
        for (int j = 0; j < voxelGrid.m_sizeY; j++)
            for (int k = 0; k < voxelGrid.m_sizeZ; k++)
            {
                if ((overrideDynamic[i][j][k]) && (dynamicVoxels[i][j][k]))
                {
                    dynamicVoxels[i][j][k] = false;
                    numberOfOverrides++;
                }
                if (dynamicVoxels[i][j][k])
                {
                    numberOfDynamicVoxels++;                   
                }
            }
    
    Logger::log(LoggerMode::rayTracingLog) << "number of dynamic voxels for scan "<< scanNumber << " : " << numberOfDynamicVoxels << Logger::endl;

    return true;
}

bool EmbeddedScan::classifyOctreeVoxelsByScan(OctreeVoxelGrid& octreeVoxelGrid, std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, const int& scanNumber)
{
    std::vector<bool> temp(1ull << octreeVoxelGrid.m_maxDepth, false);
    std::vector<std::vector<bool>> temp1(1ull << octreeVoxelGrid.m_maxDepth, temp);
    std::vector<std::vector<std::vector<bool>>> overrideDynamic(1ull << octreeVoxelGrid.m_maxDepth, temp1);
    glm::dvec3 scanOrigin = OctreeRayTracing::getTransformationCoord(getGlobalCoord(glm::dvec3(0.0, 0.0, 0.0)), octreeVoxelGrid.m_clippingAssembly);
    double voxelSize = octreeVoxelGrid.m_voxelSize;
    //run through the octree
    std::vector<uint32_t> cellList;
    cellList.push_back(m_uRootCell);
    std::vector<uint32_t> leavesToDecode;
    int i = 0;
    int leavesClassified(0);
    while (i < (int)cellList.size())
    {
        for (int j = 0; j < 8; j++)
        {
            uint32_t currCellId = m_vTreeCells[cellList[i]].m_children[j];
            if (!(currCellId == NO_CHILD))
            {
                if (m_vTreeCells[currCellId].m_isLeaf)
                {
                    //classify voxels with this leaf
                    classifyOctreeVoxelsWithLeaf(currCellId, octreeVoxelGrid, dynamicVoxels, overrideDynamic, scanNumber, scanOrigin);
                    leavesClassified++;
                    if (leavesClassified % 1000 == 0)
                        Logger::log(LoggerMode::rayTracingLog) << "leavesClassified : " << leavesClassified << Logger::endl;

                }
                else
                    cellList.push_back(currCellId);
            }
        }
        i++;
    }
    //change dynamic to take into account the override
    int numberOfOverrides(0), numberOfDynamicVoxels(0);

    for (int i = 0; i < (1 << octreeVoxelGrid.m_maxDepth); i++)
        for (int j = 0; j < (1 << octreeVoxelGrid.m_maxDepth); j++)
            for (int k = 0; k < (1 << octreeVoxelGrid.m_maxDepth); k++)
            {
                if ((overrideDynamic[i][j][k]) && (dynamicVoxels[i][j][k]))
                {
                    dynamicVoxels[i][j][k] = false;
                    numberOfOverrides++;
                }
                if (dynamicVoxels[i][j][k])
                {
                    numberOfDynamicVoxels++;
                }
            }

    Logger::log(LoggerMode::rayTracingLog) << "number of dynamic voxels for scan " << scanNumber << " : " << numberOfDynamicVoxels << Logger::endl;

    return true;
}


int EmbeddedScan::classifyVoxelsWithLeaf(const uint32_t& leafId, const VoxelGrid& voxelGrid, std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, std::vector<std::vector<std::vector<bool>>>& overrideDynamic, const int& scanNumber, const glm::dvec3& scanOrigin, const ClippingAssembly& clippingAssembly)
{
    //returns number of decoded points
    int result(0);
    //check if leaf intersects boundingBox
    GeometricBox leafBox = createGeometricBoxFromLeaf(leafId, clippingAssembly);
    std::vector<glm::dvec3> boxCorners;
    boxCorners.push_back(glm::dvec3(voxelGrid.m_xMin, voxelGrid.m_yMin, voxelGrid.m_zMin));
    boxCorners.push_back(glm::dvec3(voxelGrid.m_xMax, voxelGrid.m_yMin, voxelGrid.m_zMin));
    boxCorners.push_back(glm::dvec3(voxelGrid.m_xMin, voxelGrid.m_yMax, voxelGrid.m_zMin));
    boxCorners.push_back(glm::dvec3(voxelGrid.m_xMin, voxelGrid.m_yMin, voxelGrid.m_zMax));
    boxCorners.push_back(glm::dvec3(voxelGrid.m_xMax, voxelGrid.m_yMax, voxelGrid.m_zMin));
    boxCorners.push_back(glm::dvec3(voxelGrid.m_xMax, voxelGrid.m_yMin, voxelGrid.m_zMax));
    boxCorners.push_back(glm::dvec3(voxelGrid.m_xMin, voxelGrid.m_yMax, voxelGrid.m_zMax));
    boxCorners.push_back(glm::dvec3(voxelGrid.m_xMax, voxelGrid.m_yMax, voxelGrid.m_zMax));
    GeometricBox boundingBox(boxCorners);
    /*if (OctreeRayTracing::relateBoxToOtherBox(leafBox, boundingBox) == 3)
        return;*/

    double distanceToIntersection(0);
    //should check if ray will intersect the grid or not, to avoid decoding useless leaves
    if (!OctreeRayTracing::doBoxIntersectFromViewPoint(scanOrigin, leafBox, boundingBox, distanceToIntersection))
        return result;
    //should also check if the intersection is already too far away to matter
    
    if (distanceToIntersection > 12)
        return result;

    //we could check if every voxel that would intersect the leaf from the viewpoint is already accounted for
    //we could also batch the decoding, but unclear if thats worth it (probably not)

    //decode points
    std::vector<uint32_t> leavesId;
    leavesId.push_back(leafId);
    std::vector<glm::dvec3> decodedPoints;
    samplePointsByStep((float)0.02, leavesId, decodedPoints);
    result = (int)decodedPoints.size();
    //ray traversal to every points
    for (int i = 0; i < decodedPoints.size(); i++)
        rayTraversal(OctreeRayTracing::getTransformationCoord(decodedPoints[i],clippingAssembly), voxelGrid, dynamicVoxels, overrideDynamic, scanNumber, clippingAssembly);
    
    
    return result;
}

void EmbeddedScan::classifyOctreeVoxelsWithLeaf(const uint32_t& leafId, const OctreeVoxelGrid& octreeVoxelGrid, std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, std::vector<std::vector<std::vector<bool>>>& overrideDynamic, const int& scanNumber, const glm::dvec3& scanOrigin)
{
    //check if leaf intersects boundingBox
    GeometricBox leafBox = createGeometricBoxFromLeaf(leafId, octreeVoxelGrid.m_clippingAssembly);
    std::vector<glm::dvec3> boxCorners;
    double tMin(-octreeVoxelGrid.m_maxSize / 2), tMax(octreeVoxelGrid.m_maxSize / 2);
    boxCorners.push_back(glm::dvec3(tMin, tMin, tMin));
    boxCorners.push_back(glm::dvec3(tMax, tMin, tMin));
    boxCorners.push_back(glm::dvec3(tMin, tMax, tMin));
    boxCorners.push_back(glm::dvec3(tMin, tMin, tMax));
    boxCorners.push_back(glm::dvec3(tMax, tMax, tMin));
    boxCorners.push_back(glm::dvec3(tMax, tMin, tMax));
    boxCorners.push_back(glm::dvec3(tMin, tMax, tMax));
    boxCorners.push_back(glm::dvec3(tMax, tMax, tMax));
    GeometricBox boundingBox(boxCorners);
    /*if (OctreeRayTracing::relateBoxToOtherBox(leafBox, boundingBox) == 3)
        return;*/

    double distanceToIntersection(0);
    //should check if ray will intersect the grid or not, to avoid decoding useless leaves
    if (!OctreeRayTracing::doBoxIntersectFromViewPoint(scanOrigin, leafBox, boundingBox, distanceToIntersection))
        return;
    //should also check if the intersection is already too far away to matter

    if (distanceToIntersection > 12)
        return;

    //we could check if every voxel that would intersect the leaf from the viewpoint is already accounted for
    //we could also batch the decoding, but unclear if thats worth it (probably not)

    //decode points
    std::vector<uint32_t> leavesId;
    leavesId.push_back(leafId);
    std::vector<glm::dvec3> decodedPoints;
    samplePointsByStep((float)0.02, leavesId, decodedPoints);
    //ray traversal to every points
    for (int i = 0; i < decodedPoints.size(); i++)
        octreeRayTraversal(OctreeRayTracing::getTransformationCoord(decodedPoints[i], octreeVoxelGrid.m_clippingAssembly), octreeVoxelGrid, dynamicVoxels, overrideDynamic, scanNumber);


    return;
}

void EmbeddedScan::rayTraversal(const glm::dvec3& targetPoint, const VoxelGrid& voxelGrid, std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, std::vector<std::vector<std::vector<bool>>>& overrideDynamic, const int& scanNumber, const ClippingAssembly& clippingAssembly)
{
    //start by identifing the voxel containing the scan origin
    glm::dvec3 scanOrigin = OctreeRayTracing::getTransformationCoord(getGlobalCoord(glm::dvec3(0.0, 0.0, 0.0)),clippingAssembly);
    double voxelSize = voxelGrid.m_voxelSize;
    int xCenter, yCenter, zCenter;
    voxelGrid.voxelCoordinatesOfPoint(scanOrigin, xCenter, yCenter, zCenter);
    //get normalized ray
    glm::dvec3 ray = targetPoint - scanOrigin;
    ray = ray / glm::length(ray);
    int raySignCase(0);
    if (ray[0] < 0)
        raySignCase++;
    if (ray[1] < 0)
        raySignCase += 2;
    if (ray[2] < 0)
        raySignCase += 4;
    
    int currX(xCenter), currY(yCenter), currZ(zCenter);
    glm::dvec3 currPoint = scanOrigin;
    bool hasOverride(false);
    bool hasEnteredGrid(false);
    while (voxelGrid.areVoxelCoordinatesValid(currX, currY, currZ)||(!hasEnteredGrid))
    {
        //if we are past target point before entering the grid, we should override
        if (glm::dot(currPoint - targetPoint, targetPoint - scanOrigin) > 0)
            hasOverride = true;
        //if currPoint is far away from the scan, abort there, to hopefully reduce false positives
        if (glm::length(currPoint - scanOrigin) > 12)
            return;
        //if ray is too close to parallel to the grid axes, abort aswell
        if (std::max(std::max(abs(ray[0]), abs(ray[1])), abs(ray[2])) > 0.995)
            return;
        double xTime(0.0), yTime(0.0), zTime(0.0);
        if (voxelGrid.areVoxelCoordinatesValid(currX, currY, currZ))
        {
            hasEnteredGrid = true;
            if (hasOverride)
                overrideDynamic[currX][currY][currZ] = true;
            else
            {
                if (voxelGrid.isVoxelOccupied(currX, currY, currZ, scanNumber))
                {
                    //stop the ray traversal as soon as we encoutner an occupied voxel, unless the voxel is already dynamic
                                //but instead of stopping, we try to tag the remaining voxels as overriding the classifier.              
                    hasOverride = true;
                }
                else
                {
                    if ((voxelGrid.m_grid[currX][currY][currZ] != 0)&&(!overrideDynamic[currX][currY][currZ]))
                    {
                        if(voxelGrid.isVoxelOccupied(currX, currY, currZ, scanNumber)||hasOverride)
                            Logger::log(LoggerMode::rayTracingLog) << "problem" << Logger::endl;

                        dynamicVoxels[currX][currY][currZ] = true;

                    }
                }
                
            }
            
        }
        
        // go to next voxel 
        switch (raySignCase)
        {
        case 0:
        {
            //all positive
            xTime = (voxelGrid.m_xMin + (currX + 1) * voxelSize - currPoint[0]) / ray[0];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                xTime = DBL_MAX;
              
            yTime = (voxelGrid.m_yMin + (currY + 1) * voxelSize - currPoint[1]) / ray[1];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                yTime = DBL_MAX;
               
            zTime = (voxelGrid.m_zMin + (currZ + 1) * voxelSize - currPoint[2]) / ray[2];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                zTime = DBL_MAX;

            //minimal time defines next voxel, and next starting position

            if (xTime < yTime)
            {
                if (xTime < zTime)
                {
                    currX++;
                    currPoint = currPoint + xTime * ray;
                }
                else
                {
                    currZ++;
                    currPoint = currPoint + zTime * ray;
                }
            }
            else
            {
                if (yTime < zTime)
                {
                    currY++;
                    currPoint = currPoint + yTime * ray;
                }
                else
                {
                    currZ++;
                    currPoint = currPoint + zTime * ray;
                }
            }

            break;
        }
        case 1:
        {
            //x negative
            xTime = (voxelGrid.m_xMin + (currX - 1) * voxelSize - currPoint[0]) / ray[0];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                xTime = DBL_MAX;
               
            yTime = (voxelGrid.m_yMin + (currY + 1) * voxelSize - currPoint[1]) / ray[1];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                yTime = DBL_MAX;
                
            zTime = (voxelGrid.m_zMin + (currZ + 1) * voxelSize - currPoint[2]) / ray[2];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                zTime = DBL_MAX;

            //minimal time defines next voxel, and next starting position

            if (xTime < yTime)
            {
                if (xTime < zTime)
                {
                    currX--;
                    currPoint = currPoint + xTime * ray;
                }
                else
                {
                    currZ++;
                    currPoint = currPoint + zTime * ray;
                }
            }
            else
            {
                if (yTime < zTime)
                {
                    currY++;
                    currPoint = currPoint + yTime * ray;
                }
                else
                {
                    currZ++;
                    currPoint = currPoint + zTime * ray;
                }
            }
            break;
        }
        case 2:
        {
            // y negative
            xTime = (voxelGrid.m_xMin + (currX + 1) * voxelSize - currPoint[0]) / ray[0];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                xTime = DBL_MAX;
               
            yTime = (voxelGrid.m_yMin + (currY - 1) * voxelSize - currPoint[1]) / ray[1];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                yTime = DBL_MAX;
               
            zTime = (voxelGrid.m_zMin + (currZ + 1) * voxelSize - currPoint[2]) / ray[2];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                zTime = DBL_MAX;

            //minimal time defines next voxel, and next starting position

            if (xTime < yTime)
            {
                if (xTime < zTime)
                {
                    currX++;
                    currPoint = currPoint + xTime * ray;
                }
                else
                {
                    currZ++;
                    currPoint = currPoint + zTime * ray;
                }
            }
            else
            {
                if (yTime < zTime)
                {
                    currY--;
                    currPoint = currPoint + yTime * ray;
                }
                else
                {
                    currZ++;
                    currPoint = currPoint + zTime * ray;
                }
            }
            break;
        }
        case 3:
        {
            //x and y negative
            xTime = (voxelGrid.m_xMin + (currX - 1) * voxelSize - currPoint[0]) / ray[0];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                xTime = DBL_MAX;
              
            yTime = (voxelGrid.m_yMin + (currY - 1) * voxelSize - currPoint[1]) / ray[1];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                yTime = DBL_MAX;
               
            zTime = (voxelGrid.m_zMin + (currZ + 1) * voxelSize - currPoint[2]) / ray[2];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                zTime = DBL_MAX;

            //minimal time defines next voxel, and next starting position

            if (xTime < yTime)
            {
                if (xTime < zTime)
                {
                    currX--;
                    currPoint = currPoint + xTime * ray;
                }
                else
                {
                    currZ++;
                    currPoint = currPoint + zTime * ray;
                }
            }
            else
            {
                if (yTime < zTime)
                {
                    currY--;
                    currPoint = currPoint + yTime * ray;
                }
                else
                {
                    currZ++;
                    currPoint = currPoint + zTime * ray;
                }
            }
            break;
        }
        case 4:
        {
            //z negative
            xTime = (voxelGrid.m_xMin + (currX + 1) * voxelSize - currPoint[0]) / ray[0];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                xTime = DBL_MAX;
              
            yTime = (voxelGrid.m_yMin + (currY + 1) * voxelSize - currPoint[1]) / ray[1];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                yTime = DBL_MAX;
              
            zTime = (voxelGrid.m_zMin + (currZ - 1) * voxelSize - currPoint[2]) / ray[2];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                zTime = DBL_MAX;

            //minimal time defines next voxel, and next starting position

            if (xTime < yTime)
            {
                if (xTime < zTime)
                {
                    currX++;
                    currPoint = currPoint + xTime * ray;
                }
                else
                {
                    currZ--;
                    currPoint = currPoint + zTime * ray;
                }
            }
            else
            {
                if (yTime < zTime)
                {
                    currY++;
                    currPoint = currPoint + yTime * ray;
                }
                else
                {
                    currZ--;
                    currPoint = currPoint + zTime * ray;
                }
            }
            break;
        }
        case 5:
        {
            //x and z negative
            xTime = (voxelGrid.m_xMin + (currX - 1) * voxelSize - currPoint[0]) / ray[0];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                xTime = DBL_MAX;
              
            yTime = (voxelGrid.m_yMin + (currY + 1) * voxelSize - currPoint[1]) / ray[1];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                yTime = DBL_MAX;
              
            zTime = (voxelGrid.m_zMin + (currZ - 1) * voxelSize - currPoint[2]) / ray[2];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                zTime = DBL_MAX;

            //minimal time defines next voxel, and next starting position

            if (xTime < yTime)
            {
                if (xTime < zTime)
                {
                    currX--;
                    currPoint = currPoint + xTime * ray;
                }
                else
                {
                    currZ--;
                    currPoint = currPoint + zTime * ray;
                }
            }
            else
            {
                if (yTime < zTime)
                {
                    currY++;
                    currPoint = currPoint + yTime * ray;
                }
                else
                {
                    currZ--;
                    currPoint = currPoint + zTime * ray;
                }
            }
            break;
        }
        case 6:
        {
            //y and z negative
            xTime = (voxelGrid.m_xMin + (currX + 1) * voxelSize - currPoint[0]) / ray[0];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                xTime = DBL_MAX;
              
            yTime = (voxelGrid.m_yMin + (currY - 1) * voxelSize - currPoint[1]) / ray[1];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                yTime = DBL_MAX;
              
            zTime = (voxelGrid.m_zMin + (currZ - 1) * voxelSize - currPoint[2]) / ray[2];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                zTime = DBL_MAX;

            //minimal time defines next voxel, and next starting position

            if (xTime < yTime)
            {
                if (xTime < zTime)
                {
                    currX++;
                    currPoint = currPoint + xTime * ray;
                }
                else
                {
                    currZ--;
                    currPoint = currPoint + zTime * ray;
                }
            }
            else
            {
                if (yTime < zTime)
                {
                    currY--;
                    currPoint = currPoint + yTime * ray;
                }
                else
                {
                    currZ--;
                    currPoint = currPoint + zTime * ray;
                }
            }
            break;
        }
        case 7:
        {
            // all negative
            xTime = (voxelGrid.m_xMin + (currX - 1) * voxelSize - currPoint[0]) / ray[0];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                xTime = DBL_MAX;
               
            yTime = (voxelGrid.m_yMin + (currY - 1) * voxelSize - currPoint[1]) / ray[1];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                yTime = DBL_MAX;
                
            zTime = (voxelGrid.m_zMin + (currZ - 1) * voxelSize - currPoint[2]) / ray[2];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                zTime = DBL_MAX;

            //minimal time defines next voxel, and next starting position

            if (xTime < yTime)
            {
                if (xTime < zTime)
                {
                    currX--;
                    currPoint = currPoint + xTime * ray;
                }
                else
                {
                    currZ--;
                    currPoint = currPoint + zTime * ray;
                }
            }
            else
            {
                if (yTime < zTime)
                {
                    currY--;
                    currPoint = currPoint + yTime * ray;
                }
                else
                {
                    currZ--;
                    currPoint = currPoint + zTime * ray;
                }
            }
            break;
        }
        }
    }
}

void EmbeddedScan::octreeRayTraversal(const glm::dvec3& targetPoint, const OctreeVoxelGrid& octreeVoxelGrid, std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, std::vector<std::vector<std::vector<bool>>>& overrideDynamic, const int& scanNumber)
{
    //start by identifing the voxel containing the scan origin
    glm::dvec3 scanOrigin = OctreeRayTracing::getTransformationCoord(getGlobalCoord(glm::dvec3(0.0, 0.0, 0.0)), octreeVoxelGrid.m_clippingAssembly);
    double voxelSize = octreeVoxelGrid.m_voxelSize;
    int xCenter, yCenter, zCenter;
    octreeVoxelGrid.voxelCoordinatesOfPoint(scanOrigin, xCenter, yCenter, zCenter);

    //(point - maxSize/2)/voxelSize
    // 
    //get normalized ray
    glm::dvec3 ray = targetPoint - scanOrigin;
    ray = ray / glm::length(ray);
    int raySignCase(0);
    if (ray[0] < 0)
        raySignCase++;
    if (ray[1] < 0)
        raySignCase += 2;
    if (ray[2] < 0)
        raySignCase += 4;

    int currX(xCenter), currY(yCenter), currZ(zCenter);
    glm::dvec3 currPoint = scanOrigin;
    bool hasOverride(false);
    bool hasEnteredGrid(false);
    while (octreeVoxelGrid.areVoxelCoordinatesValid(currX, currY, currZ) || (!hasEnteredGrid))
    {
        //if we are past target point before entering the grid, we should override
        if (glm::dot(currPoint - targetPoint, targetPoint - scanOrigin) > 0)
            hasOverride = true;
        //if currPoint is far away from the scan, abort there, to hopefully reduce false positives
        if (glm::length(currPoint - scanOrigin) > 12)
            return;
        //if ray is too close to parallel to the grid axes, abort aswell
        if (std::max(std::max(abs(ray[0]), abs(ray[1])), abs(ray[2])) > 0.995)
            return;
        double xTime(0.0), yTime(0.0), zTime(0.0);
        if (octreeVoxelGrid.areVoxelCoordinatesValid(currX, currY, currZ))
        {
            hasEnteredGrid = true;
            if (hasOverride)
                overrideDynamic[currX][currY][currZ] = true;
            else
            {
                if (octreeVoxelGrid.isVoxelOccupied(currX, currY, currZ, scanNumber))
                {
                    //stop the ray traversal as soon as we encoutner an occupied voxel, unless the voxel is already dynamic
                                //but instead of stopping, we try to tag the remaining voxels as overriding the classifier.              
                    hasOverride = true;
                }
                else
                {
                    if ((!octreeVoxelGrid.isEmpty(currX,currY,currZ)) && (!overrideDynamic[currX][currY][currZ]))
                    {
                        if (octreeVoxelGrid.isVoxelOccupied(currX, currY, currZ, scanNumber) || hasOverride)
                            Logger::log(LoggerMode::rayTracingLog) << "problem" << Logger::endl;

                        dynamicVoxels[currX][currY][currZ] = true;

                    }
                }

            }

        }
        double tMin = -octreeVoxelGrid.m_maxSize / 2;

        // go to next voxel 
        switch (raySignCase)
        {
        case 0:
        {
            //all positive
            xTime = (tMin + (currX + 1) * voxelSize - currPoint[0]) / ray[0];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                xTime = DBL_MAX;

            yTime = (tMin + (currY + 1) * voxelSize - currPoint[1]) / ray[1];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                yTime = DBL_MAX;

            zTime = (tMin + (currZ + 1) * voxelSize - currPoint[2]) / ray[2];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                zTime = DBL_MAX;

            //minimal time defines next voxel, and next starting position

            if (xTime < yTime)
            {
                if (xTime < zTime)
                {
                    currX++;
                    currPoint = currPoint + xTime * ray;
                }
                else
                {
                    currZ++;
                    currPoint = currPoint + zTime * ray;
                }
            }
            else
            {
                if (yTime < zTime)
                {
                    currY++;
                    currPoint = currPoint + yTime * ray;
                }
                else
                {
                    currZ++;
                    currPoint = currPoint + zTime * ray;
                }
            }

            break;
        }
        case 1:
        {
            //x negative
            xTime = (tMin + (currX - 1) * voxelSize - currPoint[0]) / ray[0];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                xTime = DBL_MAX;

            yTime = (tMin + (currY + 1) * voxelSize - currPoint[1]) / ray[1];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                yTime = DBL_MAX;

            zTime = (tMin + (currZ + 1) * voxelSize - currPoint[2]) / ray[2];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                zTime = DBL_MAX;

            //minimal time defines next voxel, and next starting position

            if (xTime < yTime)
            {
                if (xTime < zTime)
                {
                    currX--;
                    currPoint = currPoint + xTime * ray;
                }
                else
                {
                    currZ++;
                    currPoint = currPoint + zTime * ray;
                }
            }
            else
            {
                if (yTime < zTime)
                {
                    currY++;
                    currPoint = currPoint + yTime * ray;
                }
                else
                {
                    currZ++;
                    currPoint = currPoint + zTime * ray;
                }
            }
            break;
        }
        case 2:
        {
            // y negative
            xTime = (tMin + (currX + 1) * voxelSize - currPoint[0]) / ray[0];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                xTime = DBL_MAX;

            yTime = (tMin + (currY - 1) * voxelSize - currPoint[1]) / ray[1];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                yTime = DBL_MAX;

            zTime = (tMin + (currZ + 1) * voxelSize - currPoint[2]) / ray[2];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                zTime = DBL_MAX;

            //minimal time defines next voxel, and next starting position

            if (xTime < yTime)
            {
                if (xTime < zTime)
                {
                    currX++;
                    currPoint = currPoint + xTime * ray;
                }
                else
                {
                    currZ++;
                    currPoint = currPoint + zTime * ray;
                }
            }
            else
            {
                if (yTime < zTime)
                {
                    currY--;
                    currPoint = currPoint + yTime * ray;
                }
                else
                {
                    currZ++;
                    currPoint = currPoint + zTime * ray;
                }
            }
            break;
        }
        case 3:
        {
            //x and y negative
            xTime = (tMin + (currX - 1) * voxelSize - currPoint[0]) / ray[0];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                xTime = DBL_MAX;

            yTime = (tMin + (currY - 1) * voxelSize - currPoint[1]) / ray[1];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                yTime = DBL_MAX;

            zTime = (tMin + (currZ + 1) * voxelSize - currPoint[2]) / ray[2];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                zTime = DBL_MAX;

            //minimal time defines next voxel, and next starting position

            if (xTime < yTime)
            {
                if (xTime < zTime)
                {
                    currX--;
                    currPoint = currPoint + xTime * ray;
                }
                else
                {
                    currZ++;
                    currPoint = currPoint + zTime * ray;
                }
            }
            else
            {
                if (yTime < zTime)
                {
                    currY--;
                    currPoint = currPoint + yTime * ray;
                }
                else
                {
                    currZ++;
                    currPoint = currPoint + zTime * ray;
                }
            }
            break;
        }
        case 4:
        {
            //z negative
            xTime = (tMin + (currX + 1) * voxelSize - currPoint[0]) / ray[0];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                xTime = DBL_MAX;

            yTime = (tMin + (currY + 1) * voxelSize - currPoint[1]) / ray[1];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                yTime = DBL_MAX;

            zTime = (tMin + (currZ - 1) * voxelSize - currPoint[2]) / ray[2];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                zTime = DBL_MAX;

            //minimal time defines next voxel, and next starting position

            if (xTime < yTime)
            {
                if (xTime < zTime)
                {
                    currX++;
                    currPoint = currPoint + xTime * ray;
                }
                else
                {
                    currZ--;
                    currPoint = currPoint + zTime * ray;
                }
            }
            else
            {
                if (yTime < zTime)
                {
                    currY++;
                    currPoint = currPoint + yTime * ray;
                }
                else
                {
                    currZ--;
                    currPoint = currPoint + zTime * ray;
                }
            }
            break;
        }
        case 5:
        {
            //x and z negative
            xTime = (tMin + (currX - 1) * voxelSize - currPoint[0]) / ray[0];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                xTime = DBL_MAX;

            yTime = (tMin + (currY + 1) * voxelSize - currPoint[1]) / ray[1];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                yTime = DBL_MAX;

            zTime = (tMin + (currZ - 1) * voxelSize - currPoint[2]) / ray[2];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                zTime = DBL_MAX;

            //minimal time defines next voxel, and next starting position

            if (xTime < yTime)
            {
                if (xTime < zTime)
                {
                    currX--;
                    currPoint = currPoint + xTime * ray;
                }
                else
                {
                    currZ--;
                    currPoint = currPoint + zTime * ray;
                }
            }
            else
            {
                if (yTime < zTime)
                {
                    currY++;
                    currPoint = currPoint + yTime * ray;
                }
                else
                {
                    currZ--;
                    currPoint = currPoint + zTime * ray;
                }
            }
            break;
        }
        case 6:
        {
            //y and z negative
            xTime = (tMin + (currX + 1) * voxelSize - currPoint[0]) / ray[0];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                xTime = DBL_MAX;

            yTime = (tMin + (currY - 1) * voxelSize - currPoint[1]) / ray[1];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                yTime = DBL_MAX;

            zTime = (tMin + (currZ - 1) * voxelSize - currPoint[2]) / ray[2];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                zTime = DBL_MAX;

            //minimal time defines next voxel, and next starting position

            if (xTime < yTime)
            {
                if (xTime < zTime)
                {
                    currX++;
                    currPoint = currPoint + xTime * ray;
                }
                else
                {
                    currZ--;
                    currPoint = currPoint + zTime * ray;
                }
            }
            else
            {
                if (yTime < zTime)
                {
                    currY--;
                    currPoint = currPoint + yTime * ray;
                }
                else
                {
                    currZ--;
                    currPoint = currPoint + zTime * ray;
                }
            }
            break;
        }
        case 7:
        {
            // all negative
            xTime = (tMin + (currX - 1) * voxelSize - currPoint[0]) / ray[0];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                xTime = DBL_MAX;

            yTime = (tMin + (currY - 1) * voxelSize - currPoint[1]) / ray[1];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                yTime = DBL_MAX;

            zTime = (tMin + (currZ - 1) * voxelSize - currPoint[2]) / ray[2];
            if (std::fabs(abs(ray.x)) <= std::numeric_limits<double>::epsilon())
                zTime = DBL_MAX;

            //minimal time defines next voxel, and next starting position

            if (xTime < yTime)
            {
                if (xTime < zTime)
                {
                    currX--;
                    currPoint = currPoint + xTime * ray;
                }
                else
                {
                    currZ--;
                    currPoint = currPoint + zTime * ray;
                }
            }
            else
            {
                if (yTime < zTime)
                {
                    currY--;
                    currPoint = currPoint + yTime * ray;
                }
                else
                {
                    currZ--;
                    currPoint = currPoint + zTime * ray;
                }
            }
            break;
        }
        }
    }
}

int EmbeddedScan::countPointsInBox(const GeometricBox& box, const ClippingAssembly& clippingAssembly)
{
    //run through the octree
    std::vector<uint32_t> cellList;
    cellList.push_back(m_uRootCell);
    int result(0);
    int i = 0;
    while (i < (int)cellList.size())
    {
        for (int j = 0; j < 8; j++)
        {
            uint32_t currCellId = m_vTreeCells[cellList[i]].m_children[j];
            if (!(currCellId == NO_CHILD))
            {
                if (m_vTreeCells[currCellId].m_isLeaf)
                {
                    //count points in box for that leaf
                    result+=countPointsInBoxByLeaf(currCellId, box, clippingAssembly);
                }
                else
                    cellList.push_back(currCellId);
            }
        }
        i++;
    }
    return result;
}

int EmbeddedScan::countPointsInBoxByLeaf(const uint32_t& leafId, const GeometricBox& box, const ClippingAssembly& clippingAssembly)
{
    int result = 0;
    GeometricBox leafBox = createGeometricBoxFromLeaf(leafId, clippingAssembly);
    int relation = OctreeRayTracing::relateBoxToOtherBox(box, leafBox);
    if (relation != 3)
    {
        std::vector<uint32_t> leafList;
        leafList.push_back(leafId);
        std::vector<glm::dvec3> points;
        getDecodedPoints(leafList, points, true);
        for (int i = 0; i < (int)points.size(); i++)
        {
            if (box.isInside(points[i]))
                result++;
        }
    }
    return result;
}

//////

bool EmbeddedScan::updateOctreeVoxelGrid(OctreeVoxelGrid& octreeVoxelGrid, const int& scanNumber)
{
    //run through the octree
    std::vector<uint32_t> cellList;
    cellList.push_back(m_uRootCell);
    std::vector<uint32_t> leavesToDecode;
    int i = 0;
    int numberOfLeaves(0);
    while (i < (int)cellList.size())
    {
        for (int j = 0; j < 8; j++)
        {
            uint32_t currCellId = m_vTreeCells[cellList[i]].m_children[j];
            if (!(currCellId == NO_CHILD))
            {
                if (m_vTreeCells[currCellId].m_isLeaf)
                {
                    numberOfLeaves++;
                    //update voxelGrid with this leaf
                    int ambiguousVoxels(0);
                    if (!updateOctreeVoxelGridWithLeaf(octreeVoxelGrid, currCellId, scanNumber, ambiguousVoxels))
                    {
                        leavesToDecode.push_back(currCellId);
                    }
                }
                else
                    cellList.push_back(currCellId);
            }
        }
        i++;
    }


    //decode the leaves one by one, stop when all ambiguous voxels are populated

    double meanPointPerLeaf(0), meanDecodedPointPerLeaf(0);
    //Logger::log(LoggerMode::rayTracingLog) << "total leaves to decode : " << (int)leavesToDecodeOneByOne.size() << Logger::endl;

    for (int i = 0; i < (int)leavesToDecode.size(); i++)
    {
        int ambiguousVoxels(0), solvedVoxels(0);
        if (!updateOctreeVoxelGridWithLeaf(octreeVoxelGrid, leavesToDecode[i], scanNumber, ambiguousVoxels))
        {
            std::vector<glm::dvec3> decodedPoints;
            std::vector<uint32_t> decodingLeaf(0);
            decodingLeaf.push_back(leavesToDecode[i]);
            samplePointsByStep((float)octreeVoxelGrid.m_voxelSize * (float)0.5, decodingLeaf, decodedPoints);
            int occupiedVoxels(0);
            for (int k = 0; k < (int)decodedPoints.size(); k++)
            {
                if (updateOctreeVoxelGridWithPoint(octreeVoxelGrid, decodedPoints[k], scanNumber))
                {
                    solvedVoxels++;
                    if (solvedVoxels == ambiguousVoxels)
                    {
                        break;
                    }
                }

            }
        }
    }



    return true;
}

bool EmbeddedScan::updateOctreeVoxelGridWithLeaf(OctreeVoxelGrid& octreeVoxelGrid, const uint32_t& leafId, const int& scanNumber, int& numberOfambiguousVoxels)
{
    //returns false if leaf should be decoded, true otherwise
    //should ignore if the leaf is too far away from the origin
    //maybe not ? otherwise, points far away could be false negatives, since we don't tag an empty voxel as dynamic
    GeometricBox leafBox = createGeometricBoxFromLeaf(leafId, octreeVoxelGrid.m_clippingAssembly);
    glm::dvec3 scanOrigin = OctreeRayTracing::getTransformationCoord(getGlobalCoord(glm::dvec3(0.0, 0.0, 0.0)), octreeVoxelGrid.m_clippingAssembly);
    //if (glm::length(leafBox.getCenter() - scanOrigin) - leafBox.getRadius() > 12)
    //    return true;
    bool result = true;
    TreeCell& leaf = m_vTreeCells[leafId];
    glm::dvec3 position = glm::dvec3(leaf.m_position[0], leaf.m_position[1], leaf.m_position[2]);
    glm::dvec3 globalPosition = OctreeRayTracing::getTransformationCoord(getGlobalCoord(position), octreeVoxelGrid.m_clippingAssembly);

    double xMin, xMax, yMin, yMax, zMin, zMax;

    xMin = globalPosition[0];
    xMax = globalPosition[0];
    yMin = globalPosition[1];
    yMax = globalPosition[1];
    zMin = globalPosition[2];
    zMax = globalPosition[2];

    for (int t = 0; t < 8; t++)
    {
        if (leafBox.m_corners[t][0] < xMin)
            xMin = leafBox.m_corners[t][0];
        if (leafBox.m_corners[t][0] > xMax)
            xMax = leafBox.m_corners[t][0];
        if (leafBox.m_corners[t][1] < yMin)
            yMin = leafBox.m_corners[t][1];
        if (leafBox.m_corners[t][1] > yMax)
            yMax = leafBox.m_corners[t][1];
        if (leafBox.m_corners[t][2] < zMin)
            zMin = leafBox.m_corners[t][2];
        if (leafBox.m_corners[t][2] > zMax)
            zMax = leafBox.m_corners[t][2];
    }

    int xPosMin, xPosMax, yPosMin, yPosMax, zPosMin, zPosMax;
    double voxelSize = octreeVoxelGrid.m_voxelSize;
    int maxIndex = (int)pow(2, octreeVoxelGrid.m_maxDepth);
    xPosMin = std::max((int)((xMin + octreeVoxelGrid.m_maxSize / 2) / voxelSize),0);
    xPosMax = std::min((int)((xMax + octreeVoxelGrid.m_maxSize / 2) / voxelSize),maxIndex);
    yPosMin = std::max((int)((yMin + octreeVoxelGrid.m_maxSize / 2) / voxelSize),0);
    yPosMax = std::min((int)((yMax + octreeVoxelGrid.m_maxSize / 2) / voxelSize), maxIndex);
    zPosMin = std::max((int)((zMin + octreeVoxelGrid.m_maxSize / 2) / voxelSize),0);
    zPosMax = std::min((int)((zMax + octreeVoxelGrid.m_maxSize / 2) / voxelSize), maxIndex);
  
    int cellsToTry = (xPosMax - xPosMin) * (yPosMax - yPosMin) * (zPosMax - zPosMin);
    for (int i = xPosMin; i < (xPosMax + 1); i++)
        for (int j = yPosMin; j < (yPosMax + 1); j++)                
            for (int k = zPosMin; k < (zPosMax + 1); k++)       
            {
                //if there are too many cells to check abort to save time
                if (cellsToTry > 50)
                {
                    numberOfambiguousVoxels = cellsToTry;
                    return false;
                }
                

                //ignore if the voxel is already tagged as occupied
                if (octreeVoxelGrid.isVoxelOccupied(i, j, k, scanNumber))
                    continue;

                GeometricBox gridBox = octreeVoxelGrid.getBoxFromCoordinates(i, j, k);
                int relationResult = OctreeRayTracing::relateBoxToOtherBox(leafBox, gridBox);
                switch (relationResult)
                {
                case 0:
                {
                    //leafBox inside gridBox

                    //tag this voxel as occupied
                    //voxelGrid.m_grid[i][j][k] = voxelGrid.m_grid[i][j][k] | (1 << scanNumber);
                    octreeVoxelGrid.addValue(i, j, k, scanNumber);
                    break;
                }
                case 1:
                {
                    //gridBox inside leafBox
                    //add leaf to leavesToDecode
                    numberOfambiguousVoxels++;
                    result = false;
                    break;
                }
                case 2:
                {
                    //boxes intersect
                    //add leaf to leavesToDecode
                    numberOfambiguousVoxels++;
                    result = false;
                    break;
                }
                case 3:
                {
                    //boxes don't intersect
                    break;
                }
                }
            }
            
    //Logger::log(LoggerMode::rayTracingLog) << "leaf done" << Logger::endl;

    return result;
}

bool EmbeddedScan::updateOctreeVoxelGridWithPoint(OctreeVoxelGrid& octreeVoxelGrid, const glm::dvec3& point, const int& scanNumber)
{
    //returns true iff it modified voxelGrid

    glm::dvec3 truePoint = OctreeRayTracing::getTransformationCoord(point, octreeVoxelGrid.m_clippingAssembly);
    int x, y, z;
    octreeVoxelGrid.voxelCoordinatesOfPoint(truePoint, x, y, z);
    if (octreeVoxelGrid.areVoxelCoordinatesValid(x, y, z))
    {
        if (!octreeVoxelGrid.isVoxelOccupied(x, y, z, scanNumber))
        {
            octreeVoxelGrid.addValue(x, y, z, scanNumber);
            return true;
        }
    }
   
    return false;
}
