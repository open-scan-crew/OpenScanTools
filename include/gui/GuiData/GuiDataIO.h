#ifndef GUI_DATA_IO_H
#define GUI_DATA_IO_H

#include "gui/GuiData/IGuiData.h"
#include "io/exports/ExportParameters.hpp"

#include <unordered_set>

class AGraphNode;
class AClippingNode;

class GuiDataOpenProject : public IGuiData
{
public:
    GuiDataOpenProject(const std::filesystem::path& folder);
    ~GuiDataOpenProject();
    guiDType getType() override;
public:
    const std::filesystem::path m_folder;
};

class GuiDataImportScans : public IGuiData
{
public:
    GuiDataImportScans();
    ~GuiDataImportScans();
    guiDType getType() override;
};

class GuiDataConversionOptionsDisplay : public IGuiData
{
public:
    GuiDataConversionOptionsDisplay(const uint64_t& fileType, const glm::dvec3& translation);
    guiDType getType() override;
public:
    uint64_t m_type;
    glm::dvec3 m_translation;
};

class GuiDataConversionFilePaths : public IGuiData
{
public:
    GuiDataConversionFilePaths(const std::vector<std::filesystem::path>& paths);
    guiDType getType() override;
public:
    std::vector<std::filesystem::path> m_paths;
};

class GuiDataExportParametersDisplay : public IGuiData
{
public:
    GuiDataExportParametersDisplay(const std::unordered_set<SafePtr<AClippingNode>>& clippings, ObjectStatusFilter status_filter, bool useClippings, bool useGrids, bool showMergeOption);
    ~GuiDataExportParametersDisplay() {};
    guiDType getType() override;

public:
    std::unordered_set<SafePtr<AClippingNode>> m_clippings;
    ObjectStatusFilter pc_status_filter_;
    bool m_useClippings;
    bool m_useGrids;
    bool m_showMergeOption;
};

class GuiDataTemporaryPath : public IGuiData
{
public:
    GuiDataTemporaryPath(const std::filesystem::path& path);
    ~GuiDataTemporaryPath();
    guiDType getType() override;

    const std::filesystem::path m_path;
};

class GuiDataProjectPath : public IGuiData
{
public:
    GuiDataProjectPath(const std::filesystem::path& path);
    ~GuiDataProjectPath();
    guiDType getType() override;

    const std::filesystem::path m_path;
};

class GuiDataSendRecentProjects : public IGuiData
{
public:
    GuiDataSendRecentProjects(std::vector<std::pair<std::filesystem::path, time_t>> recentProjects);
    ~GuiDataSendRecentProjects();
    guiDType getType() override;

public:
    std::vector<std::pair<std::filesystem::path, time_t>> m_recentProjects;
};

class GuiDataOpenInExplorer : public IGuiData
{
public:
    GuiDataOpenInExplorer(const std::filesystem::path& path);
    ~GuiDataOpenInExplorer();
    guiDType getType() override;

    const std::filesystem::path m_path;
};

class GuiDataExportFileObjectDialog : public IGuiData
{
public:
    GuiDataExportFileObjectDialog(const std::unordered_map<std::wstring, std::wstring>& infoExport);
    ~GuiDataExportFileObjectDialog();
    guiDType getType() override;
public:
    std::unordered_map<std::wstring, std::wstring> m_infoExport;

};

class GuiDataImportFileObjectDialog : public IGuiData
{
public:
    GuiDataImportFileObjectDialog(const std::unordered_set<SafePtr<AGraphNode>>& notFoundFileObjects, bool isOnlyLink);
    ~GuiDataImportFileObjectDialog();
    guiDType getType() override;
public:
    std::unordered_set<SafePtr<AGraphNode>>							   m_notFoundFileObjects;
    bool													           m_isOnlyLink;

};
/*
class GuiDataAsciiPCFilesInfo : public IGuiData
{
public:
    GuiDataImportFileObjectDialog(const std::unordered_map<xg::Guid, std::pair<std::string, std::string>>& infoImport, bool isOnlyLink);
    ~GuiDataImportFileObjectDialog();
    guiDType getType() override;
public:
    std::unordered_map<xg::Guid, std::pair<std::string, std::string>>  m_infoImport;
    bool													           m_isOnlyLink;

};
*/

#endif
