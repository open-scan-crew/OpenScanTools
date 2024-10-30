#ifndef CONTEXT_EXPORT_PC_H_
#define CONTEXT_EXPORT_PC_H_

#include "controller/functionSystem/AContext.h"
#include "io/exports/ExportParameters.hpp"
#include "models/data/Clipping/ClippingGeometry.h"
#include "models/project/ProjectInfos.h"
#include "models/pointCloud/PointCloudInstance.h"

#include <vector>
#include <glm/glm.hpp>
#include <unordered_set>

enum FileType;
enum PrecisionType;
class IScanFileWriter;
class CSVWriter;

class GraphManager;
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
    // specific export functions
    bool processClippingExport(Controller& controller);
    bool processScanExport(Controller& controller);

private:
    // all combinations with clipping active
    bool exportClippingAndScanMerged(Controller& controller, CSVWriter* csvWriter);
    bool exportClippingSeparated(Controller& controller, CSVWriter* csvWriter);
    bool exportScanSeparated(Controller& controller, CSVWriter* csvWriter);

    bool processGridExport(Controller& controller);

    // helper functions
    void addOriginCube(IScanFileWriter* fileWriter, tls::PointFormat pointFormat, CSVWriter& csvWriter);
    bool ensureFileWriter(Controller& controller, std::unique_ptr<IScanFileWriter>& scanFileWriter, std::wstring name, CSVWriter* csvWriter);
    bool prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath);
    std::vector<tls::PointCloudInstance> getPointCloudInstances(GraphManager& graphManager);
    void getBestOriginOrientationAndBBox(const ClippingAssembly& clippingAssembly, const BoundingBoxD& scanBBox, glm::dvec3& bestOrigin, glm::dquat& bestOrientation);
    // Should be static functions for BoundingBox
    static BoundingBoxD getGlobalBoundingBox(const std::vector<tls::PointCloudInstance>& pcInstances);
    static BoundingBoxD extractBBox(const IClippingGeometry& clippingGeom);
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
    ProjectInfos m_subProjectInfo;
    ProjectInternalInfo m_subProjectInternal;
    ObjectStatusFilter m_objectFilterType;
};

#endif // !CONTEXT_GRID_CLIPPING_H_