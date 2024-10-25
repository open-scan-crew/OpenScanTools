#ifndef PROJECT_INFO_H
#define PROJECT_INFO_H

#include <string>
#include <filesystem>
#include "crossguid/guid.hpp"
#include "models/data/Clipping/ClippingTypes.h"

#include "utils/safe_ptr.h"

#include <glm/glm.hpp>

class Author;

class ProjectInfos
{
public:
	ProjectInfos();

	//new
	ProjectInfos(const xg::Guid id, const std::wstring& projectName, const SafePtr<Author>& author, const std::wstring& company,
		const std::wstring& location, const std::wstring& description, const bool& isCentral);

	ProjectInfos(const ProjectInfos& p);

public:
	//new
	xg::Guid m_id;
	bool m_isCentral;
	std::filesystem::path m_centralProjectPath;
	xg::Guid m_centralId;

	std::filesystem::path m_customScanFolderPath;
	
	std::filesystem::path m_projectName;
	SafePtr<Author>	m_author;
	std::wstring m_company;
	std::wstring m_location;
	std::wstring m_description;
	double		m_beamBendingTolerance;
	double		m_columnTiltTolerance;

	float m_defaultMinClipDistance;
	float m_defaultMaxClipDistance;
	ClippingMode m_defaultClipMode;
	float m_defaultMinRampDistance;
	float m_defaultMaxRampDistance;
	int m_defaultRampSteps = 8;

	xg::Guid	m_defaultScan;
	glm::dvec3 m_importScanTranslation;
	
};

class ProjectInternalInfo
{
public:

	//getter
	const std::filesystem::path& getProjectFilePath() const;
	const std::filesystem::path& getProjectFolderPath() const;

	std::filesystem::path getScansFolderPath() const;
	std::filesystem::path getObjectsProjectPath() const;
	std::filesystem::path getTemplatesFolderPath() const;
	std::filesystem::path getObjectsFilesFolderPath() const;
	std::filesystem::path getTagsFolderPath() const;
	std::filesystem::path getReportsFolderPath() const;

	std::filesystem::path getQuickScreenshotsFolderPath() const;
	std::filesystem::path getOrthoHDFolderPath() const;
	std::filesystem::path getPerspHDFolderPath() const;

	std::filesystem::path getQuickVideosFolderPath() const;

	//setter
	void setProjectFolderPath(const std::filesystem::path& projectFolderPath, const std::wstring& projectName);
	void setCustomScanFolderPath(const std::filesystem::path& customScanFolderPath);

private:
	std::filesystem::path m_projectFilePath;
	std::filesystem::path m_projectFolderPath;

	std::filesystem::path m_customScanFolderPath;
};

#endif // !_PROJECT_INFO_H_
