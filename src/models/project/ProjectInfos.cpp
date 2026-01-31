#include "models/project/ProjectInfos.h"
#include "models/application/Author.h"
#include "utils/FilesAndFoldersDefinitions.h"


ProjectInfos::ProjectInfos()
	: m_beamBendingTolerance(200.0)
	, m_columnTiltTolerance(150.0)
	, m_defaultMinClipDistance(0.0)
	, m_defaultMaxClipDistance(0.5)
	, m_defaultMinRampDistance(0.0)
	, m_defaultMaxRampDistance(0.5)
{
	m_projectName = L"";
	m_author = SafePtr<Author>();
	m_company = L"";
	m_location = L"";
	m_description = L"";
	m_id = xg::Guid();
}

ProjectInfos::ProjectInfos(const ProjectInfos& p)
{
	this->m_projectName = p.m_projectName;
	this->m_author = p.m_author;
	this->m_company = p.m_company;
	this->m_location = p.m_location;
	this->m_description = p.m_description;
	this->m_beamBendingTolerance = p.m_beamBendingTolerance;
	this->m_columnTiltTolerance = p.m_columnTiltTolerance;
	this->m_defaultMinClipDistance = p.m_defaultMinClipDistance;
	this->m_defaultMaxClipDistance = p.m_defaultMaxClipDistance;
	this->m_defaultMinRampDistance = p.m_defaultMinRampDistance;
	this->m_defaultMaxRampDistance = p.m_defaultMaxRampDistance;
	this->m_defaultScan = p.m_defaultScan;
	this->m_importScanTranslation = p.m_importScanTranslation;
	this->m_id = p.m_id;
	this->m_customScanFolderPath = p.m_customScanFolderPath;
	this->m_temperatureScaleFilePath = p.m_temperatureScaleFilePath;
}

ProjectInfos::ProjectInfos(const xg::Guid id, const std::wstring & projectName, const  SafePtr<Author>& author, const std::wstring & company, const std::wstring & location, const std::wstring & description)
	: ProjectInfos()
{
	m_projectName = projectName;
	m_author = author;
	m_company = company;
	m_location = location;
	m_description = description;
	m_id = id;
}

const std::filesystem::path& ProjectInternalInfo::getProjectFilePath() const
{
	return (m_projectFilePath);
}

const std::filesystem::path& ProjectInternalInfo::getProjectFolderPath() const
{
	return (m_projectFolderPath);
}

std::filesystem::path ProjectInternalInfo::getPointCloudFolderPath(bool is_object) const
{
	if (is_object)
		return getObjectsFilesFolderPath();

	if(m_customScanFolderPath.empty())
		return (m_projectFolderPath / Folder_Scans);
	else
		return (m_customScanFolderPath);
}

std::filesystem::path ProjectInternalInfo::getObjectsProjectPath() const
{
	return (m_projectFolderPath / Folder_Objects);
}

std::filesystem::path ProjectInternalInfo::getObjectsFilesFolderPath() const
{
	return (m_projectFolderPath / Folder_ObjectsFiles);
}

std::filesystem::path ProjectInternalInfo::getTagsFolderPath() const
{
	return (m_projectFolderPath / Folder_Tags);
}

std::filesystem::path ProjectInternalInfo::getReportsFolderPath() const
{
	return (m_projectFolderPath / Folder_Reports);
}

std::filesystem::path ProjectInternalInfo::getTemplatesFolderPath() const
{
	return (m_projectFolderPath / Folder_Template);
}

std::filesystem::path ProjectInternalInfo::getQuickScreenshotsFolderPath() const
{
	return (m_projectFolderPath / Folder_Screenshots);
}

std::filesystem::path ProjectInternalInfo::getOrthoHDFolderPath() const
{
	return (m_projectFolderPath / Folder_OrthoHD);
}

std::filesystem::path ProjectInternalInfo::getPerspHDFolderPath() const
{
	return (m_projectFolderPath / Folder_PerspHD);
}

std::filesystem::path ProjectInternalInfo::getQuickVideosFolderPath() const
{
	return (m_projectFolderPath / Folder_Videos);
}

void ProjectInternalInfo::setProjectFolderPath(const std::filesystem::path& projectFolderPath, const std::wstring& projectName)
{
    m_projectFilePath = projectFolderPath / projectName;
    m_projectFilePath += ".tlp";
    m_projectFolderPath = projectFolderPath;
}

void ProjectInternalInfo::setCustomScanFolderPath(const std::filesystem::path& customScanFolderPath)
{
	if(std::filesystem::exists(customScanFolderPath))
		m_customScanFolderPath = customScanFolderPath;
}
