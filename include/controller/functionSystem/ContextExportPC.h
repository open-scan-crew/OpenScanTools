#ifndef CONTEXT_EXPORT_PC_H_
#define CONTEXT_EXPORT_PC_H_

#include "controller/functionSystem/AContext.h"
#include "io/exports/ExportParameters.hpp"
#include "models/data/Clipping/ClippingGeometry.h"
#include "models/project/ProjectInfos.h"

#include <vector>
#include <glm/glm.hpp>
#include <map>
#include <unordered_set>

enum FileType;
enum PrecisionType;
class IScanFileWriter;
class CSVWriter;

class OpenScanToolsGraphManager;
class CameraNode;

class ContextExportPC : public AContext
{
public:
    ContextExportPC(const ContextId& id);
    ~ContextExportPC();
    ContextState start(Controller& controller) override;
    ContextState launch(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    bool canAutoRelaunch() const;

    ContextType getType() const override;
protected:
    bool processClippingExport(Controller& controller);
    bool processScanExport(Controller& controller);

private:

    bool exportClippingAndScanMerged(Controller& controller, CSVWriter* csvWriter);
    bool exportClippingSeparated(Controller& controller, CSVWriter* csvWriter);
    bool exportScanSeparated(Controller& controller, CSVWriter* csvWriter);

    bool processGridExport(Controller& controller);

    void addOriginCube(IScanFileWriter* fileWriter, tls::PointFormat pointFormat, CSVWriter& csvWriter);
    bool ensureFileWriter(Controller& controller, std::unique_ptr<IScanFileWriter>& scanFileWriter, std::wstring name, CSVWriter* csvWriter);
    bool prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath);
    std::vector<tls::PointCloudInstance> getPointCloudInstances(OpenScanToolsGraphManager& graphManager);
    void getBestOriginOrientationAndBBox(const ClippingAssembly& clippingAssembly, const tls::BoundingBox& scanBBox, glm::dvec3& bestOrigin, glm::dquat& bestOrientation);
    // Should be static functions for tls::BoundingBox
    static tls::BoundingBox getGlobalBoundingBox(const std::vector<tls::PointCloudInstance>& pcInstances);
    static tls::BoundingBox extractBBox(const IClippingGeometry& clippingGeom);
    static tls::BoundingBox transformBoundingBox(const tls::BoundingBox& bbox, glm::dmat4 transfo);
    static void unionBoundingBox(tls::BoundingBox& dstBBox, const tls::BoundingBox& srcBBox);
    static void intersectBoundingBox(tls::BoundingBox& dstBBox, const tls::BoundingBox& srcBBox);
    // !! static //
    tls::Transformation getCommonTransformation(const std::vector<tls::PointCloudInstance>& pcInfos);
    tls::PointFormat getCommonFormat(const std::vector<tls::PointCloudInstance>& pcInfos);

protected:
    int m_neededMessageCount;
    bool m_forSubProject;
    SafePtr<CameraNode> m_cameraNode;
    ClippingExportParameters m_parameters;

private:
    std::unordered_set<SafePtr<APointCloudNode>> m_selectedPcs;
    ContextId m_saveContext;
    xg::Guid m_viewportId;
    bool m_exportScans;
    bool m_exportPCOs;
    bool m_useClips;
    bool m_useGrids;
    glm::dvec3 m_scanTranslationToAdd = glm::dvec3(0.);

    uint64_t m_currentStep;
};

class ContextExportSubProject : public ContextExportPC
{
public:
    ContextExportSubProject(const ContextId& id);
    ~ContextExportSubProject();
    ContextState start(Controller& controller) override;
    ContextState launch(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    ContextState abort(Controller& controller) override;
    bool canAutoRelaunch() const;

    ContextType getType() const override;
private:
    //void recGetObjectsClusters(std::unordered_map<xg::Guid, const Data*>& dataMap, const Data* data, Project* project);

private:
    ProjectInfos m_subProjectInfo;
    ProjectInternalInfo m_subProjectInternal;
    ObjectStatusFilter m_objectFilterType;
};

#endif // !CONTEXT_GRID_CLIPPING_H_