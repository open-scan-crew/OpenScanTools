#ifndef CONTEXT_STATISTICAL_OUTLIER_FILTER_H
#define CONTEXT_STATISTICAL_OUTLIER_FILTER_H

#include "controller/functionSystem/AContext.h"
#include "models/graph/TransformationModule.h"
#include "io/FileUtils.h"

#include "crossguid/guid.hpp"

#include <filesystem>

class ContextStatisticalOutlierFilter : public AContext
{
public:
    ContextStatisticalOutlierFilter(const ContextId& id);
    ~ContextStatisticalOutlierFilter();

    ContextState start(Controller& controller) override;
    ContextState launch(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    bool canAutoRelaunch() const override;
    ContextType getType() const override;

private:
    bool prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath);

    bool m_warningModal = false;
    bool m_globalFiltering = false;
    int m_kNeighbors = 20;
    double m_nSigma = 1.0;
    int m_samplingPercent = 2;
    double m_beta = 4.0;
    xg::Guid m_panoramic;
    FileType m_outputFileType = FileType::TLS;
    std::filesystem::path m_outputFolder;
    bool m_openFolderAfterExport = true;
};

#endif
