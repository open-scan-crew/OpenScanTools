#ifndef TL_SCAN_OVERSEER_H
#define TL_SCAN_OVERSEER_H

#include <atomic>
#include <filesystem>
#include <functional>
#include <list>
#include <mutex>
#include <set>
#include <unordered_map>

#include "tls_def.h"
#include "models/pointCloud/PointCloudInstance.h"
#include "pointCloudEngine/PCE_stream.h"
#include "models/data/clipping/ClippingGeometry.h"
#include "models/graph/TransformationModule.h"
#include "models/3d/Measures.h"

#include "pointCloudEngine/OctreeRayTracing.h"
#include "pointCloudEngine/OutlierStats.h"
#include "models/pointCloud/PointXYZIRGB.h"

/*
template<typename T>
class PoolAllocator
{
public:
    PoolAllocator(uint32_t firstBlockCapacity);
    ~PoolAllocator();
    void Clear();
    T* Alloc();
    void Free(T* ptr);

private:
    union Item
    {
        uint32_t NextFreeIndex;
        T Value;
    };

    struct ItemBlock
    {
        Item* pItems;
        uint32_t Capacity;
        uint32_t FirstFreeIndex;
    };

    const uint32_t m_firstBlockCapacity;
    std::vector<ItemBlock> m_itemBlocks;

    ItemBlock& CreateNewBlock();
};

template<typename T>
PoolAllocator<T>::PoolAllocator(uint32_t firstBlockCapacity)
    : m_firstBlockCapacity(firstBlockCapacity)
    , m_itemBlocks(std::vector<ItemBlock>())
{
    assert(m_firstBlockCapacity > 1);
}

template<typename T>
PoolAllocator<T>::~PoolAllocator()
{
    Clear();
}

template<typename T>
void PoolAllocator<T>::Clear()
{
    for (size_t i = m_itemBlocks.size(); i > 0; --i)
    {
        for (size_t j = m_itemBlocks[i].Capacity; j--; )
        {
            m_itemBlocks[i].pItems[j].~Item();
        }
        delete m_itemBlocks[i].pItems;
    }
    m_itemBlocks.clear();
}

template<typename T>
void PoolAllocator<T>::Free(T* ptr)
{
    for (size_t i = m_itemBlocks.size(); i > 0; --i)
    {
        ItemBlock& block = m_itemBlocks[i];

        Item* pItemPtr;
        memcpy(&pItemPtr, &ptr, sizeof(pItemPtr));

        if ((pItemPtr >= block.pItems) && (pItemPtr < block.pItems + block.Capacity))
        {
            const uint32_t index = static_cast<uint32_t>(pItemPtr - block.pItems);
            pItemPtr->NextFreeIndex = block.FirstFreeIndex;
            block.FirstFreeIndex = index;
            return;
        }
    }
    assert(0 && "Pointer doesn't belong to this memory pool.");
}
*/

class EmbeddedScan;
class IScanFileWriter;

// Avoid to include PCE_graphics.h and vulan.h
struct TlProjectionInfo;
struct TlScanDrawInfo;

enum class FitCylinderMode
{
    fast,
    robust,
    multiple
};

enum class LineConnectionType
{
    none,
    elbow,
    coaxial,
    stitching,
    reduction
};

class PolygonalPlane
{
public:
    PolygonalPlane();
    ~PolygonalPlane();

    void setCenter(const glm::dvec3& center);
    void setDirection(const glm::dvec3& direction);
    void setNormal(const glm::dvec3& normal);
    glm::dvec3 getCenter();
    glm::dvec3 getNormal();
    std::vector<glm::dvec3> getVertices();
    glm::dvec3 getDirection();

    void addVertex(const glm::dvec3& vertex);
    bool deleteVertex(const int& index);
    bool isInterior(const int& index);
    void clearInterior();

private:
    std::vector<glm::dvec3> m_vertices;
    glm::dvec3 m_normal;
    glm::dvec3 m_center;
    glm::dvec3 m_referenceDirection;
};

struct IndexedValue {
    double value;
    int index;

    // Custom comparison function for the priority queue
    bool operator<(const IndexedValue& other) const {
        return value < other.value;
    }
};

// \brief
//  * The Scanoverseer prevent that a file is opened more than one time
//  * The Scanoverseer prevent that 2 different files with the same Guid are opened at the same time
//  * The Scanoverseer allow to find the associated file to a scan

class TlScanOverseer
{
public:
    using ProgressCallback = std::function<void(size_t processed, size_t total)>;
    static TlScanOverseer& getInstance()
    {
        static TlScanOverseer instance;
        return instance;
    }

    // C++ 11 // Explicitly avoid to copy the singleton
    TlScanOverseer(TlScanOverseer const&) = delete;
    void operator=(TlScanOverseer const&) = delete;

    void init();
    void shutdown();


    static void setWorkingScansTransfo(const std::vector<tls::PointCloudInstance>& workingScans);

    // Management of the active resources
    bool getScanGuid(std::filesystem::path filePath, tls::ScanGuid& scanGuid);
    bool getScanHeader(tls::ScanGuid scanGuid, tls::ScanHeader& scanHeader);
    bool getScanPath(tls::ScanGuid scanGuid, std::filesystem::path& scanPath);

    bool isScanLeftTofree();
    void copyScanFile_async(const tls::ScanGuid& scanGuid, const std::filesystem::path& destPath, bool savePath, bool overrideDestination, bool removeSource, const ProgressCallback& progress = {});
    void freeScan_async(tls::ScanGuid scanGuid, bool deletePhysicalFile);
    void resourceManagement_sync();

private:
    struct scanCopyInfo
    {
        tls::ScanGuid guid;
        std::filesystem::path path;
        bool savePath;
        bool overrideDestination;
        bool removeSource;
        ProgressCallback progress;
    };
    void syncFileCopy();
    bool doFileCopy(scanCopyInfo& copyInfo);
    void freeWaitingResources();

public:
    // Graphics
    bool getScanView(tls::ScanGuid scanGuid, const TlProjectionInfo& projInfo, const ClippingAssembly& clippingAssembly, TlScanDrawInfo& scanDrawInfo, bool& needStreaming);

    // Streaming
    void haltStream();
    void resumeStream();
    void streamScans(uint64_t maxSize, char* stagingBuffer, std::vector<TlStagedTransferInfo>& vkTransfer);

    // Compute
    bool testClippingEffect(tls::ScanGuid scanGuid, const TransformationModule& modelMat, const ClippingAssembly& clippingAssembly);
    bool clipScan(tls::ScanGuid scanGuid, const TransformationModule& modelMat, const ClippingAssembly& clippingAssembly, IScanFileWriter* outScan, const ProgressCallback& progress = {});
    bool computeOutlierStats(tls::ScanGuid scanGuid, const TransformationModule& modelMat, const ClippingAssembly& clippingAssembly, int kNeighbors, int samplingPercent, double beta, OutlierStats& stats, const ProgressCallback& progress = {});
    bool filterOutliersAndWrite(tls::ScanGuid scanGuid, const TransformationModule& modelMat, const ClippingAssembly& clippingAssembly, int kNeighbors, const OutlierStats& stats, double nSigma, double beta, IScanFileWriter* outScan, uint64_t& removedPoints, const ProgressCallback& progress = {});
    bool balanceColorsAndWrite(tls::ScanGuid scanGuid, const TransformationModule& modelMat, const ClippingAssembly& clippingAssembly, int kMin, int kMax, double trimPercent, double sharpnessBlend, bool applyOnIntensity, bool applyOnRgb, const std::function<void(const GeometricBox&, std::vector<PointXYZIRGB>&)>& externalPointsProvider, IScanFileWriter* outScan, uint64_t& modifiedPoints, const ProgressCallback& progress = {});
    void collectPointsInGeometricBox(const GeometricBox& box, const ClippingAssembly& clippingAssembly, const tls::ScanGuid& excludedGuid, std::vector<PointXYZIRGB>& result);
    //tls::ScanGuid clipNewScan(tls::ScanGuid scanGuid, const glm::dmat4& modelMat, const ClippingAssembly& clippingAssembly, const std::filesystem::path& outPath, uint64_t& pointDeletedCount);

    // Lucas functions
    bool rayTracing(const glm::dvec3& ray, const glm::dvec3& rayOrigin, glm::dvec3& bestPoint, const double& cosAngleThreshold, const ClippingAssembly& clippingAssembly, const bool& isOrtho, std::string& scanName);
    bool findNeighborsBucketsDirected(const glm::dvec3& globalSeedPoint, const glm::dvec3& directedPoint, const double& radius, std::vector<std::vector<glm::dvec3>>& neighborList, int numberOfBuckets, const ClippingAssembly& clippingAssembly);
    bool findNeighborsBuckets(const glm::dvec3& globalSeedPoint, const double& radius, std::vector<std::vector<glm::dvec3>>& neighborList, int numberOfBuckets, const ClippingAssembly& clippingAssembly);
    bool findNeighborsBucketsTest(const glm::dvec3& globalSeedPoint, const double& radius, std::vector<std::vector<glm::dvec3>>& neighborList, int numberOfBuckets, const ClippingAssembly& clippingAssembly, std::vector<glm::dquat>& rotations, std::vector<glm::dvec3>& positions, std::vector<double>& scales);

    bool findNeighbors(const glm::dvec3& globalSeedPoint, const double& radius, std::vector<glm::dvec3>& neighborList, const ClippingAssembly& clippingAssembly);
    bool findNeighborsTowardsPoint(const glm::dvec3& globalSeedPoint, const glm::dvec3& targetPoint, const double& radius, std::vector<glm::dvec3>& neighborList, const ClippingAssembly& clippingAssembly);
    bool nearestNeighbor(const glm::dvec3& globalPoint, glm::dvec3& result);

    bool fitCylinder(const glm::dvec3& globalSeedPoint, const double& radius, const double& threshold, double& cylinderRadius, glm::dvec3& cylinderDirection, glm::dvec3& cylinderCenter, const FitCylinderMode& mode, const ClippingAssembly& clippingAssembly);
    bool fitCylinderTest(const glm::dvec3& globalSeedPoint, const double& radius, const double& threshold, double& cylinderRadius, glm::dvec3& cylinderDirection, glm::dvec3& cylinderCenter, const FitCylinderMode& mode, const ClippingAssembly& clippingAssembly, std::vector<glm::dquat>& rotations, std::vector<glm::dvec3>& positions, std::vector<double>& scales);

    bool fitCylinderMultipleSeeds(const std::vector<glm::dvec3>& globalSeedPoint, const double& radius, const double& threshold, double& cylinderRadius, glm::dvec3& cylinderDirection, glm::dvec3& cylinderCenter, const FitCylinderMode& mode, const ClippingAssembly& clippingAssembly);
    bool fitCylinder4Clicks(const std::vector<glm::dvec3>& seedPoints, const double& radius, const double& threshold, double& cylinderRadius, glm::dvec3& cylinderDirection, glm::dvec3& cylinderCenter, const ClippingAssembly& clippingAssembly);
    bool fitBigCylinder(const glm::dvec3& seedPoint1, const glm::dvec3& seedPoint2, const double& radius, const double& threshold, double& cylinderRadius, glm::dvec3& cylinderDirection, glm::dvec3& cylinderCenter, std::vector<glm::dvec3>& tags, std::vector<std::vector<double>>& planes, const ClippingAssembly& clippingAssembly);
    //bool fitBigCylinder2(const glm::dvec3& seedPoint1, const glm::dvec3& seedPoint2, const double& radius, const double& threshold, double& cylinderRadius, glm::dvec3& cylinderDirection, glm::dvec3& cylinderCenter, const ClippingAssembly& clippingAssembly);
    bool beamBending(const std::vector<glm::dvec3>& globalEndPoints, glm::dvec3& bendPoint, double& maxBend, double& ratio, bool& reliable, const ClippingAssembly& clippingAssembly);
    static bool discretize(const glm::dvec3& tip1, const glm::dvec3& tip2, const int& numberOfSteps, std::vector<glm::dvec3>& discretePoints);
    bool nearestBendNeighbor(glm::dvec3 localPoint, glm::dvec3& result, glm::dvec3 normalVector);
    static bool columnOffset(const glm::dvec3& camPosition, glm::dvec3& wallPoint1, const glm::dvec3& wallPoint2, const glm::dvec3& columnPoint1, const glm::dvec3& columnPoint2, double& offset, double& ratio);
    //bool pointToPlaneMeasure(const glm::dvec3& seedPoint, const glm::dvec3& planePoint, const ClippingAssembly& clippingAssembly);
    //bool computeBeamHeight(const glm::dvec3& seedPoint, double& beamHeight, const ClippingAssembly& clippingAssembly);
    void ballCloseToPlane(const glm::dvec3& seedPoint, const std::vector<double>& plane, const double& radius, std::vector<glm::dvec3>& result, const double& distanceThreshold, const ClippingAssembly& clippingAssembly);
    std::vector<glm::dvec3> ballInBox(const glm::dvec3 seedPoint, const glm::dvec3& beamDirection, const glm::dvec3& orthoDir, const glm::dvec3& normalDir, const std::vector<std::vector<double>>& xyRange, const double& heightMax);
    void estimateNormals();
    bool fitPlane(const glm::dvec3& seedPoint, std::vector<double>& result, const ClippingAssembly& clippingAssembly);
    bool fitPlaneRadius(const glm::dvec3& seedPoint, std::vector<double>& result, const ClippingAssembly& clippingAssembly, const double& radius);
    bool fitPlane3Points(const std::vector<glm::dvec3> points, std::vector<double>& result);
    ////////// simple measures /////////////
    bool pointToCylinderMeasure(const glm::dvec3& point, const glm::dvec3 cylinderPoint, glm::dvec3& projectedPoint, glm::dvec3& projectedCylinderPoint, double& cylinderRadius, glm::dvec3& cylinderDirection, glm::dvec3& cylinderCenter, const ClippingAssembly& clippingAssembly);
    // NOTE(robin) - Maintenant directement fait dans ContextPointToPlaneMeasure
    //bool pointToPlaneMeasure(const glm::dvec3& point, const glm::dvec3& planePoint, glm::dvec3& projectedPoint, glm::dvec3& normalVector, const ClippingAssembly& clippingAssembly);
    //bool cylinderToPlaneMeasure(const glm::dvec3& cylinderPoint, const glm::dvec3& planePoint, glm::dvec3& cylinderAxisPoint, glm::dvec3& projectedPlanePoint, glm::dvec3& cylinderDirection, double& cylinderRadius, glm::dvec3& normalVector, bool& cylinderFit);
    //bool cylinderToCylinderMeasure(const glm::dvec3& point1, const glm::dvec3& point2, glm::dvec3& cylinder1AxisPoint, glm::dvec3& cylinder2AxisPoint, glm::dvec3& cylinder1Direction, glm::dvec3 cylinder2Direction, double& cylinder1Radius, double& cylinder2Radius);

    ////////// plane fitting //////////////
    bool fitPlaneRegionGrowing(const glm::dvec3& seedPoint, std::vector<double>& result, const ClippingAssembly& clippingAssembly);
    bool fitPlaneTripleSeeds(const glm::dvec3& mainSeed, const glm::dvec3& endSeed1, const glm::dvec3& endSeed2, std::vector<double>& result, const ClippingAssembly& clippingAssembly);
    bool isPlaneAcceptable(const glm::dvec3& seedPoint, const std::vector<glm::dvec3>& points, const double& pointsRadius, const std::vector<double>& plane, const double& threshold);

    ////////// multipleCylinders /////////////
    bool beginMultipleCylinders(std::vector<glm::dvec3>& cylinderCenters, std::vector<glm::dvec3>& cylinderDirections, std::vector<double>& cylinderRadii, const ClippingAssembly& clippingAssembly, std::vector<glm::dvec3>& seedPoints);
    //bool isCylinderCloseToPreviousCylinders(const std::vector<glm::dvec3>& cylinderCenters, const std::vector<glm::dvec3>& cylinderDirections, const std::vector<double>& cylinderRadii, const glm::dvec3& cCenter, const glm::dvec3& cDirection, const double& cRadius);

    //extendCylinder//
    bool extendCylinder(const glm::dvec3& seedPoint, const double& radius, const double& threshold, double& cylinderRadius, glm::dvec3& cylinderDirection, glm::dvec3& cylinderCenter, std::vector<double>& heights, const FitCylinderMode& mode, const ClippingAssembly& clippingAssembly);
    glm::dvec3 refineDirection(const double& cylinderRadius, const glm::dvec3& cylinderCenter, const glm::dvec3& cylinderDirection, const std::vector<double>& heights, const double& threshold, const ClippingAssembly& clippingAssembly);

    bool testHeight(const double& radius, const glm::dvec3& center, const glm::dvec3& direction, const double& height, const int& numberOfTestPoints, const double& threshold);
    bool testHeight2(const double& radius, const glm::dvec3& center, const glm::dvec3& direction, const double& height, const int& numberOfTestPoints, const double& threshold, const std::vector<glm::dvec3>& realPoints, double& goodPointsRatio, const double& ratioThreshold);

    std::vector<double> computeCylinderHeight2(const glm::dvec3& seedPoint, const double& radius, const glm::dvec3& center, const glm::dvec3& direction, const int& numberOfTestPoints, const double& threshold, const double& heightStep, const ClippingAssembly& clippingAssembly);
    bool samplePointsBetweenHeights(const glm::dvec3& seedPoint, const double& radius, const glm::dvec3& direction, const glm::dvec3& center, const double& height1, const double& height2, const double& heightThreshold, std::vector<glm::dvec3>& sampledPoints, const ClippingAssembly& clippingAssembly);

    //detectBeam//
    bool beamDetection(const glm::dvec3& seedPoint, const ClippingAssembly& clippingAssembly, glm::dvec3& normalVector, double& beamHeight, std::vector<std::vector<double>>& directionRange, const glm::dvec3& camPos, glm::dvec3& beamDirection, glm::dvec3& orthoDir);
    bool beamDetectionManualExtend(const glm::dvec3& seedPoint, const glm::dvec3& endPoint1, const glm::dvec3& endPoint2, const ClippingAssembly& clippingAssembly, glm::dvec3& normalVector, double& beamHeight, std::vector<std::vector<double>>& directionRange, const glm::dvec3& camPos, glm::dvec3& beamDirection, glm::dvec3& orthoDir);
    static std::vector<double> getBeamStandardList();
    double computeClosestStandardBeam(const double& height);

    //fitSphere//
    bool fitSphere(const std::vector<glm::dvec3>& seedPoints, glm::dvec3& center, double& radius, const double& threshold, const ClippingAssembly& clippingAssembly, glm::dvec3& centerOfMass);

    //connectPipes//
    bool applyConstraints(std::vector<glm::dvec3>& centers, std::vector<glm::dvec3>& directions, std::vector<double>& lengths, std::vector<double>& angleModifs, std::vector<glm::dvec3>& elbowPoints, std::vector<LineConnectionType>& connectionType, const double& mainRadius, const std::vector<double>& radii, std::vector<std::vector<glm::dvec3>>& elbowEdges, const bool& angleConstraints, std::vector<glm::dvec3>& elbowCenters);
    double getTargetCosAngle(const glm::dvec3& direction1, const glm::dvec3& direction2);
    void getEndAndMiddlePoints(const glm::dvec3& center1, const glm::dvec3& direction1, const double& length1, const glm::dvec3& center2, const glm::dvec3& direction2, const double& length2, std::vector<glm::dvec3>& endPoints, std::vector<glm::dvec3>& middlePoints);
    glm::dvec3 computeElbowPosition(const glm::dvec3& center1, const glm::dvec3& center2, const glm::dvec3& elbowPoint, const double& radius);
    glm::dvec3 computeIntersectionAxes(const glm::dvec3& center1, const glm::dvec3& center2, const glm::dvec3& dir1, const glm::dvec3& dir2, const double& length1, const double& length2, const bool& angleConstraints);
    void getElbowParameters(glm::dvec3& center1, glm::dvec3& center2, glm::dvec3& dir1, glm::dvec3& dir2, double& length1, double& length2, const double& radius1, const double& radius2, double& mainAngle, const double& mainRadius, const glm::dvec3& elbowRealCenter, glm::dvec3& elbowPosResult, TransformationModule& mod, const double& targetCosAngle, const bool& angleConstraints);
    bool lookForStitchConnexion(glm::dvec3& center, glm::dvec3& direction, double& length, const std::vector<glm::dvec3>& cylCenters, const std::vector<glm::dvec3> cylDirections, const std::vector<double> cylRadii, const std::vector<double>& lengths, const double& radius);
    std::vector<int> lookForSecondaryStitchConnexion(const glm::dvec3& center, const glm::dvec3& direction, const double& length, const std::vector<glm::dvec3>& cylCenters, const std::vector<glm::dvec3> cylDirections, const std::vector<double>& cylRadii, const std::vector<double>& lengths, const double& radius);
       std::vector<int> getTotalCoaxialLine(glm::dvec3& endPoint1, glm::dvec3& endPoint2, const std::vector<glm::dvec3>& centers, const std::vector<glm::dvec3>& directions, const std::vector<double>& lengths, const glm::dvec3& currCenter, const glm::dvec3& currDir, const double& currLength);
    glm::dvec3 getPointFurthestAway(const glm::dvec3& center, const glm::dvec3& direction, const std::vector<glm::dvec3> points);
    double pointToLineDistance(const glm::dvec3& point, const glm::dvec3& direction, const glm::dvec3& linePoint);
    bool arrangeCylindersInLine(std::vector<glm::dvec3>& centers, std::vector<glm::dvec3>& directions, std::vector<double>& lengths, std::vector<double>& radii, std::vector<int>& order);
    double lineToLineDistance(const glm::dvec3& dir1, const glm::dvec3& dir2, const glm::dvec3& point1, const glm::dvec3& point2);

    //areaOfPolyline
    glm::dvec3 computeAreaOfPolyline(std::vector<Measure> measures);

    //SlabDetection//

    bool fitSlab(const glm::dvec3& seedPoint, glm::dvec3& boxCenter, glm::dvec3& boxDirectionX, glm::dvec3& boxDirectionY, glm::dvec3& boxDirectionZ, const ClippingAssembly& clippingAssembly);
    glm::dvec3 computeCorner(const std::vector<double>& plane1, const std::vector<double>& plane2, const std::vector<double>& plane3);
    bool fitSlabTest(const glm::dvec3& seedPoint, std::vector<std::vector<double>>& planes, std::vector<glm::dvec3>& centers, glm::dvec3& corner, const ClippingAssembly& clippingAssembly, glm::dvec3& scale, glm::dvec3& normal1, glm::dvec3& normal2, glm::dvec3& normal3, glm::dvec3& trueCenter, const bool& extend);
    bool extendSlab(const std::vector<double>& plane, const glm::dvec3& corner, const glm::dvec3& normal, const glm::dvec3& dir, const glm::dvec3& orthoDir, double& scale, const ClippingAssembly& clippingAssembly);
    bool extendSlab2(const std::vector<double>& plane, const glm::dvec3& corner, const glm::dvec3& normal, const glm::dvec3& dir, const glm::dvec3& orthoDir, double& scale, const ClippingAssembly& clippingAssembly, const double& heightStart);
    bool fitSlab3Clicks(const std::vector<glm::dvec3>& seedPoints, glm::dvec3& boxCenter, glm::dvec3& boxDirectionX, glm::dvec3& boxDirectionY, glm::dvec3& boxDirectionZ, const ClippingAssembly& clippingAssembly, std::vector<std::vector<double>>& planes, std::vector<glm::dvec3>& centers, glm::dvec3& scale, const bool& extend);
    bool fitSlab2Clicks(const std::vector<glm::dvec3>& seedPoints, glm::dvec3& boxCenter, glm::dvec3& boxDirectionX, glm::dvec3& boxDirectionY, glm::dvec3& boxDirectionZ, const ClippingAssembly& clippingAssembly, std::vector<std::vector<double>>& planes, std::vector<glm::dvec3>& centers, glm::dvec3& scale, const bool& extend);
    bool verticalizePlane(std::vector<double>& plane, const glm::dvec3& seedPoint);
    glm::dquat computeRotation(const glm::dvec3& u, const glm::dvec3& v);

    //PlaneConnection//
    
    bool computePlaneExtensionTowardsLine(PolygonalPlane& polygonalPlane, const glm::dvec3& lineDirection, const glm::dvec3 linePoint);

    //People remover

    int getNumberOfScans();

    bool createOctreeVoxelGrid(OctreeVoxelGrid& octreeVoxelGrid);
    bool classifyOctreeVoxels(OctreeVoxelGrid& octreeVoxelGrid, std::vector<std::vector<std::vector<bool>>>& dynamicVoxels);
    void createClustersOfDynamicOctreeVoxels(const OctreeVoxelGrid& octreeVoxelGrid, const std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, std::vector<std::vector<std::vector<int>>>& clusterLabels, int& numberOfClusters, std::vector<ClusterInfo>& clusterInfo, const int& threshold);
    void breadthFirstSearchOctree(const int& x, const int& y, const int& z, const std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, std::vector<std::vector<std::vector<int>>>& clusterLabels, int& currentLabel, ClusterInfo& clusterInfo, const int& threshold, const OctreeVoxelGrid& octreeVoxelGrid);
    std::vector<std::vector<int>> coverClusterWithBoxesOctree(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label, const std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, const OctreeVoxelGrid& octreeVoxelGrid, const ClusterInfo& clusterInfo);
    bool shouldSplitBoxOctree(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label, const std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, const OctreeVoxelGrid& octreeVoxelGrid, const std::vector<int>& box, int& x, int& y, int& z);
    bool displayOctreeVoxelGrid(const OctreeVoxelGrid& octreeVoxelGrid);
    void copyOctreeIntoGrid(const OctreeVoxelGrid& octreeVoxelGrid, VoxelGrid& voxelGrid);


    bool createVoxelGrid(VoxelGrid& voxelGrid, const ClippingAssembly& clippingAssembly);
    bool displayVoxelGrid(VoxelGrid& voxelGrid);
    bool classifyVoxels(VoxelGrid& voxelGrid, const ClippingAssembly& clippingAssembly, std::vector<std::vector<std::vector<bool>>>& dynamicVoxels);
    bool fillDynamicVoxelsBucketed(VoxelGrid& voxelGrid, const ClippingAssembly& clippingAssembly, std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, std::vector<std::vector<std::vector<int>>>& dynamicVoxelsBucketed);
    void mergeDynamicVoxelsVertically(const VoxelGrid& voxelGrid, const std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, std::vector<glm::dvec3>& centers, std::vector<int>& sizes);
    void createClustersOfDynamicVoxels(const VoxelGrid& voxelGrid, const std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, std::vector<std::vector<std::vector<int>>>& clusterLabels, int& numberOfClusters, std::vector<ClusterInfo>& clusterInfo, const int& threshold);
    void breadthFirstSearch(const int& x, const int& y, const int& z, const std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, std::vector<std::vector<std::vector<int>>>& clusterLabels, int& currentLabel, ClusterInfo& clusterInfo, const int& threshold, const VoxelGrid& voxelGrid);
    void mergeNearClustersTogether(std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& numberOfClusters, std::vector<int>& holesInLabels, std::vector<ClusterInfo>& clusterInfo, const int& threshold);
    void mergeClusterInfo(ClusterInfo& targetInfo, const ClusterInfo& info2);
    bool isDistanceBetweenVoxelAndClusterSmallerThanThreshold(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& x, const int& y, const int& z, const int& clusterLabel, const int& threshold, const ClusterInfo& info2);
    bool isDisanceBetweenClustersSmallerThanThreshold(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label1, const int& label2, const int& threshold, const ClusterInfo& clusterInfo2);
    int distanceBetweenPointAndBoundingBox(const ClusterInfo& info, const int& x, const int& y, const int& z);
    void changeClusterLabel(std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& oldLabel, const int& newLabel);
    void getBoundingBoxOfCluster(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label, std::vector<int>& boundingBox);
    int distanceBetweenBoundingBoxes(const ClusterInfo& info1, const ClusterInfo& info2);
    std::vector<std::vector<int>> coverClusterWithBoxes(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label, const std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, const VoxelGrid& voxelGrid, const ClusterInfo& clusterInfo);
    void splitBoxAtPoint(const std::vector<int>& box, std::vector<std::vector<int>>& resultBoxes, const int& x, const int& y, const int& z);
    void splitBoxAtPoint2(const std::vector<int>& box, std::vector<std::vector<int>>& resultBoxes, const int& x, const int& y, const int& z);
    bool shouldSplitBox(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label, const std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, const VoxelGrid& voxelGrid, const std::vector<int>& box, int& x, int& y, int& z);
    bool isBoxOfPositiveSize(const std::vector<int>& box);
    bool doesBoxContainALabeledVoxel(const std::vector<int>& box, const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label);
    void smallifyBoxToMatchCluster(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label, std::vector<int>& box);
    void smallifyBoxesToMatchCluster(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label, std::vector<std::vector<int>>& boxes);
    void mergeBoxes(std::vector<std::vector<int>>& boxes);
    int getVolumeOfCluster(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label);
    std::vector<int> mergeTwoBoxes(const std::vector<int>& box1, const std::vector<int>& box2);
    std::set<std::vector<int>> getBoxCorners(const std::vector<int>& box);
    // NOTE(robin) - Il semble que les 2 fonctions suivantes sont static, on pourrait donc leur extraire dans un fichier de code à part pour faciliter la compilation
    static std::vector<glm::dvec3> sampleHeightPoints(const glm::dvec3& center, const double& radius, const glm::dvec3& direction, const double& height, const int& numberOfTestPoints);
    static double computeHeight(const glm::dvec3& point, const glm::dvec3& direction, const glm::dvec3& center);

    //Plane detection

    bool fitLocalPlane(const ClippingAssembly& clippingAssembly, const glm::dvec3& seedPoint, RectangularPlane& result);
    bool fitPlaneAutoExtend(const ClippingAssembly& clippingAssembly, const glm::dvec3& seedPoint, RectangularPlane& rectPlane);
    bool extendRectangleFromCorner(RectangularPlane& rectPlane, const double& step, const double& planeAngleThreshold, const int& cornerIndex, const ClippingAssembly& clippingAssembly);
    bool extendPlaneFromSeed(const RectangularPlane& rectPlane, const glm::dvec3& seed, const double& planeAngleThreshold, const ClippingAssembly& clippingAssembly);
    void fitPlaneMultipleSeeds(const std::vector<glm::dvec3>& seedPoints, TransformationModule& result);
    void fitVerticalPlane(const glm::dvec3& seedPoint1, const glm::dvec3& seedPoint2, TransformationModule& result);

    //Sets of Points

    void setOfPoints(const glm::dvec3& projDirection, const glm::dvec3& startingPoint, const glm::dvec3& endPoint, const double& step, const double& threshold, std::vector<glm::dvec3>& createdPoints, std::vector<bool>& pointCreatedIsReal, const double& cosAngleThreshold, const ClippingAssembly& clipAssembly);
    void setOfPointsWith4thPoint(const glm::dvec3& projDirection, const std::vector<glm::dvec3>& userPoints, const double& step, const double& threshold, std::vector<glm::dvec3>& createdPointsStart, std::vector<glm::dvec3>& createdPointsEnd, std::vector<bool>& pointCreatedIsReal, const double& cosAngleThreshold, const ClippingAssembly& clipAssembly);

    //Torus detection
    
    bool testIndices(const int& x, const int& y, const int& z, const int& xMax, const int& yMax, const int& zMax);
    void naiveTorus(const GeometricBox& box, const ClippingAssembly& clippingAssembly);
    std::vector<glm::dvec3> pointsInBox(const GeometricBox& box, const ClippingAssembly& clippingAssembly);
    void fitCircle(const std::vector<glm::dvec2>& points, glm::dvec2& center, double& radius, double& error);
    std::vector<glm::dvec3> getPointsNearPlane(const std::vector<glm::dvec3>& points, const std::vector<double>& plane, const double& threshold);
    std::vector<glm::dvec2> convertToPlanePoints(const std::vector<glm::dvec3>& points, const glm::dvec3& planeCenter, const glm::dvec3& dirX, const glm::dvec3& dirY);
    void makeListOfTestPlanes(const glm::dvec3& searchCenter, const double& searchRadius, std::vector<std::vector<std::vector<double>>>& planes, std::vector<std::vector<glm::dvec3>>& planeCenters);
    glm::dvec3 convertTo3DPoint(const glm::dvec2& point2D, const glm::dvec3& origin, const glm::dvec3& dirX, const glm::dvec3& dirY);
    bool torusFitFromCircles(const glm::dvec3& searchOrigin, const double& searchSize, const std::vector<glm::dvec3>& dataPoints, glm::dvec3& torusCenter, double& principalRadius, double& pipeRadius, glm::dvec3& axis);
    bool torusFitFromCirclesPrep(const glm::dvec3& point, const ClippingAssembly& clipAssembly, glm::dvec3& torusCenter, double& principalRadius, double& pipeRadius, glm::dvec3& axis);
    void fitCircleTest();
    std::vector<int> getTopIndices(const std::vector<double>& values, int k);
    std::vector<int> filterOutlierValues(const std::vector<double>& values);
    double findMedian(const std::vector<double>& values);

    //helpers for fitCircle
    double computeD11(const std::vector<glm::dvec2>& points);
    double computeD20(const std::vector<glm::dvec2>& points);
    double computeD30(const std::vector<glm::dvec2>& points);
    double computeD21(const std::vector<glm::dvec2>& points);
    double computeD02(const std::vector<glm::dvec2>& points);
    double computeD03(const std::vector<glm::dvec2>& points);
    double computeD12(const std::vector<glm::dvec2>& points);
    double computeC(const std::vector<glm::dvec2>& points, const double& a, const double& b);


private:
    TlScanOverseer();
    ~TlScanOverseer();

    struct WorkingScanInfo
    {
        EmbeddedScan& scan;
        TransformationModule    transfo;
        bool                    isClippable;
    };

    std::vector<glm::dvec3> samplePoints(const int& numberOfPoints, const ClippingAssembly& clippingAssembly);
    static bool isCylinderCloseToPreviousCylinders(const std::vector<glm::dvec3>& cylinderCenters, const std::vector<glm::dvec3>& cylinderDirections, const std::vector<double>& cylinderRadii, const glm::dvec3& cCenter, const glm::dvec3& cDirection, const double& cRadius);

private:
    std::mutex m_activeMutex;
    std::atomic<bool> m_haltStream;

    // Accessible files and scans
    std::unordered_map<tls::ScanGuid, EmbeddedScan*> m_activeScans;
    static thread_local std::vector<WorkingScanInfo> s_workingScansTransfo;

    // Inaccessible scans waiting to be deleted (some frames after their last rendering)
    std::list<EmbeddedScan*> m_scansToFree;

    std::mutex m_copyMutex;
    std::list<scanCopyInfo> m_waitingCopies;
};



#endif
