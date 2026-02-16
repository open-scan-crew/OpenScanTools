#ifndef CONTEXT_FILTERED_SCANS_EXPORT_H
#define CONTEXT_FILTERED_SCANS_EXPORT_H

#include "controller/functionSystem/AContext.h"
#include "io/exports/ExportParameters.hpp"
#include "models/3d/DisplayParameters.h"
#include "models/pointCloud/PointCloudInstance.h"

#include <filesystem>

class CameraNode;
class Controller;
class IMessage;

class ContextFilteredScansExport : public AContext
{
public:
    ContextFilteredScansExport(const ContextId& id);
    ~ContextFilteredScansExport();

    ContextState start(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    ContextState launch(Controller& controller) override;
    bool canAutoRelaunch() const override;
    ContextType getType() const override;

private:
    bool prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath);
    bool hasActiveFilter() const;
    std::wstring buildDefaultFileName(const std::wstring& projectName) const;

private:
    int m_neededMessageCount = 2;
    ClippingExportParameters m_parameters;
    ColorimetricFilterSettings m_colorimetricFilterSettings;
    PolygonalSelectorSettings m_polygonalSelectorSettings;
    UiRenderMode m_renderMode = UiRenderMode::RGB;
    bool m_filterReady = false;
    glm::dvec3 m_scanTranslationToAdd = glm::dvec3(0.0);
};

#endif
