#include "gui/GuiData/GuiDataIO.h"

#include "utils/Logger.h"
#include "io/FileUtils.h"

// **** GuiDataOpenProjectCentral ****

GuiDataOpenProjectCentral::GuiDataOpenProjectCentral(const std::filesystem::path& centralPath)
	: m_centralPath(centralPath)
{ }

GuiDataOpenProjectCentral::~GuiDataOpenProjectCentral()
{ }

guiDType GuiDataOpenProjectCentral::getType()
{
	return (guiDType::openProjectCentral);
}

// **** GuiDataOpenProject ****

GuiDataOpenProject::GuiDataOpenProject(const std::filesystem::path& folder)
	: m_folder(folder)
{ }

GuiDataOpenProject::~GuiDataOpenProject()
{ }

guiDType GuiDataOpenProject::getType()
{
	return (guiDType::openProject);
}

// **** GuiDataImportScans ****

GuiDataImportScans::GuiDataImportScans()
{ }

GuiDataImportScans::~GuiDataImportScans()
{ }

guiDType GuiDataImportScans::getType()
{
	return (guiDType::importScans);
}

// **** Conversion Options Display ****

GuiDataConversionOptionsDisplay::GuiDataConversionOptionsDisplay(const uint64_t& fileType, const glm::dvec3& translation)
	: m_type(fileType)
	, m_translation(translation)
{}

guiDType GuiDataConversionOptionsDisplay::getType()
{
	return (guiDType::conversionOptionsDisplay);
}

// **** Conversion File Paths ****

GuiDataConversionFilePaths::GuiDataConversionFilePaths(const std::vector<std::filesystem::path>& paths)
	: m_paths(paths)
{}

guiDType GuiDataConversionFilePaths::getType()
{
	return (guiDType::conversionFilePaths);
}

// **** Clipping Export Parameters Display ****

GuiDataExportParametersDisplay::GuiDataExportParametersDisplay(const std::unordered_set<SafePtr<AClippingNode>>& clippings, bool useClippings, bool useGrids, bool showMergeOption)
    : m_clippings(clippings)
    , m_useClippings(useClippings)
    , m_useGrids(useGrids)
	, m_showMergeOption(showMergeOption)
{}

GuiDataExportParametersDisplay::~GuiDataExportParametersDisplay()
{
}

guiDType GuiDataExportParametersDisplay::getType()
{
    return (guiDType::exportParametersDisplay);
}

/*
* GuiDataTemporaryPath
*/

GuiDataTemporaryPath::GuiDataTemporaryPath(const std::filesystem::path& path)
	: m_path(path)
{}

GuiDataTemporaryPath::~GuiDataTemporaryPath()
{}

guiDType GuiDataTemporaryPath::getType() 
{
	return (guiDType::temporaryPath);
}

/*
* GuiDataTemporaryPath
*/

GuiDataProjectPath::GuiDataProjectPath(const std::filesystem::path& path)
	: m_path(path)
{}

GuiDataProjectPath::~GuiDataProjectPath()
{}

guiDType GuiDataProjectPath::getType()
{
	return (guiDType::projectPath);
}

// **** GuiDataSendRecentProjects ****

GuiDataSendRecentProjects::GuiDataSendRecentProjects(std::vector<std::pair<std::filesystem::path, time_t>>  recentProjects) : m_recentProjects(recentProjects)
{}

GuiDataSendRecentProjects::~GuiDataSendRecentProjects()
{}

guiDType GuiDataSendRecentProjects::getType()
{
	return guiDType::sendRecentProjects;
}

GuiDataOpenInExplorer::GuiDataOpenInExplorer(const std::filesystem::path& path)
	: m_path(path)
{
}

GuiDataOpenInExplorer::~GuiDataOpenInExplorer()
{
}

guiDType GuiDataOpenInExplorer::getType()
{
	return guiDType::openInExplorer;
}

// **** GuiDataExportFileObjectDialog ****

GuiDataExportFileObjectDialog::GuiDataExportFileObjectDialog(const std::unordered_map<std::wstring, std::wstring>& infoExport)
	: m_infoExport(infoExport)
{}

GuiDataExportFileObjectDialog::~GuiDataExportFileObjectDialog()
{}

guiDType GuiDataExportFileObjectDialog::getType()
{
	return guiDType::exportFileObjectDialog;
}

// **** GuiDataImportFileObjectDialog ****

GuiDataImportFileObjectDialog::GuiDataImportFileObjectDialog(const std::unordered_set<SafePtr<AGraphNode>>& notFoundFileObjects, bool isOnlyLink)
	: m_notFoundFileObjects(notFoundFileObjects)
	, m_isOnlyLink(isOnlyLink)
{}

GuiDataImportFileObjectDialog::~GuiDataImportFileObjectDialog()
{}

guiDType GuiDataImportFileObjectDialog::getType()
{
	return guiDType::importFileObjectDialog;
}