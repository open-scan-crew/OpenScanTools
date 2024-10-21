#ifndef CONTROLLER_CONTEXT_H
#define CONTROLLER_CONTEXT_H

//#include "models/3d/ManipulationTypes.h"
#include "models/3d/DuplicationTypes.h"
#include "models/application/List.h"
#include "models/application/TagTemplate.h"
#include "models/project/Marker.h"
#include "pointCloudEngine/RenderingTypes.h"
#include "models/project/ProjectTypes.h"
#include "models/data/PolylineMeasure/PolyLineTypes.h"
#include "controller/functionSystem/PipeDetectionOptions.h"
#include "gui/UnitUsage.h"

#include "models/data/Box/ClippingBoxSettings.h"
//#include "models/application/Ids.hpp"
#include "models/project/ProjectInfos.h"

#include "models/application/UserOrientation.h"

class Author;
class ScanNode;
class CameraNode;

struct CreationAttributes
{
	Color32		color;
	std::wstring name;
	std::wstring identifier;
	std::wstring discipline;
	std::wstring phase;
};

// Permet l'accès à une partie de tous ce qui ne se sauvegarde pas dans le projet (affichage des points, couleurs de clusters, ...) et à l'accès au Project
class ControllerContext
{
public:
	ControllerContext();
	~ControllerContext();

	SafePtr<StandardList> getCurrentStandard(const StandardType& type) const;
	const Color32& getActiveColor() const;
	const std::wstring& getActiveName() const;
	const std::wstring& getActiveIdentifer() const;
	const std::wstring& getActiveDiscipline() const;
	const std::wstring& getActivePhase() const;
	float getRenderPointSize() const;
	Color32 getActiveBackgroundColor();
	Color32 getNextBackgroundColor();
	void setUserBackgroundColor(const Color32& color, const uint32_t& position);
	DecimationOptions getDecimationOptions();
	void setDecimationOptions(const DecimationOptions& options);
	scs::MarkerIcon getActiveIcon() const;
	bool getShowClusterColors() const;
	void setShowClusterColors(bool show);
	bool getIsCurrentProjectSaved() const;
	std::unordered_set<SafePtr<StandardList>>& getStandards(const StandardType& type);
	SafePtr<StandardList> getStandard(const StandardType& type, const listId& id) const;

	SafePtr<sma::TagTemplate> getCurrentTemplate() const;
	std::unordered_set<SafePtr<sma::TagTemplate>>& getTemplates();
	const std::unordered_set<SafePtr<sma::TagTemplate>>& cgetTemplates() const;
	bool setTemplates(std::vector<sma::TagTemplate> newTemps, bool reset = false);

	std::unordered_set<SafePtr<UserList>>& getUserLists();
	bool setUserLists(std::vector<UserList> list, bool reset = false);
	void removeUserLists(SafePtr<UserList> list);
	SafePtr<UserList> getUserList(listId id) const;

	bool verifNameForList(std::wstring nameToTest);
	bool verifNameForTemplate(std::wstring nameToTest);
	bool verifNameForStandards(StandardType type, std::wstring nameToTest);

	ClippingBoxSettings getClippingSettings();
	const ClippingBoxSettings& CgetClippingSettings() const;
	const PolyLineOptions& getPolyLineOptions();

	//SafePtr<Author> getAuthor(const xg::Guid& authorId) const;
	SafePtr<Author> getActiveAuthor() const;
	std::unordered_set<SafePtr<Author>> getLocalAuthors() const;
	std::unordered_set<SafePtr<Author>> getProjectAuthors() const;

	DuplicationSettings getDuplicationSettings();
	const DuplicationSettings& CgetDuplicationSettings() const;
	const std::filesystem::path& getTemporaryPath() const;
	const std::filesystem::path& getProjectsPath() const;

	const std::vector<std::pair<std::filesystem::path, time_t>>& getRecentProjects() const;
	const IndexationMethod& getIndexationMethod() const;

	const PipeDetectionOptions& getPipeDetectionOptions() const;

	void setDuplicationSettings(DuplicationSettings settings);// to DuplicationSettings
	void setClippingSettings(ClippingBoxSettings settings);// to ClippingBoxSettings
	void setClippingSize(const glm::vec3& size);// to ClippingBoxSettings
	void setClippingOffset(const ClippingBoxOffset& offset);// to ClippingBoxSettings
	void setClippingAngleZValue(const double& angle);// to ClippingBoxSettings
	void setCurrentTemplate(SafePtr<sma::TagTemplate> temp);
	void setCurrentStandard(const SafePtr<StandardList>& standard, const StandardType& type);

	void setActiveColor(Color32 color);
	void setActiveName(const std::wstring& name);
	void setActiveIdentifer(const std::wstring& identifier);
	void setActiveDiscipline(const std::wstring& discipline);
	void setActivePhase(const std::wstring& phase);

	void setActiveIcon(scs::MarkerIcon icon);
	void setIsCurrentProjectSaved(bool value);
	bool setStandards(std::vector<StandardList> list, const StandardType& type, bool reset = false);
	void setPolyLineOptions(const PolyLineOptions& options);

	SafePtr<Author> createAuthor(const Author&) const;
	void setActiveAuthor(const SafePtr<Author>& activeAuthor);
	void addProjectAuthors(const std::unordered_set<SafePtr<Author>>& authors);
	void addLocalAuthors(const std::unordered_set<SafePtr<Author>>& authors);
	void remLocalAuthors(const std::unordered_set<SafePtr<Author>>& authors);

	void setTemporaryPath(const std::filesystem::path& path);
	void setProjectsPath(const std::filesystem::path& path);
	void setRenderPointSize(float pointSize);

	void setRecentProjects(std::vector<std::pair<std::filesystem::path, time_t>> projects);
	void setIndexationMethod(IndexationMethod method);

	void setProjectTransformation(const glm::dvec3& projectTransformation);

	void setDefaultScanId(SafePtr<ScanNode> defaultScan);
	SafePtr<ScanNode> getDefaultScan();

	void setPipeDetectionOptions(const PipeDetectionOptions& pipeDetectionOptions);

	void initProjectInfo(std::filesystem::path projectFolder, const ProjectInfos& info);
	void cleanProjectInfo();
	bool isProjectLoaded();

	void setProjectInfo(const ProjectInfos& info);

	ProjectInfos& getProjectInfo();
	const ProjectInfos& cgetProjectInfo() const;
	const ProjectInternalInfo& cgetProjectInternalInfo() const;
	const ProjectInternalInfo& cgetProjectCentralInternalInfo() const;

	std::unordered_map<xg::Guid, UserOrientation>& getUserOrientations();
	const std::unordered_map<xg::Guid, UserOrientation>& cgetUserOrientations() const;

	const UserOrientation& getActiveUserOrientation() const;
	void setActiveUserOrientation(const UserOrientation& uo);


	UnitUsage m_unitUsage;

private:
	bool m_isCurrentProjectSaved = true;
	uint32_t m_currentBackgroundColor;
	DecimationOptions m_decimationOptions;
	float m_renderPointSize;
	std::vector<Color32> m_backgroundColors;

	std::unordered_set<SafePtr<Author>> m_contextKnownAuthors;
	std::unordered_set<SafePtr<Author>> m_localAuthors;
	std::unordered_set<SafePtr<Author>> m_projAuthors;
	SafePtr<Author> m_activeAuthor;

	std::unordered_map<StandardType, std::unordered_set<SafePtr<StandardList>>> m_standardsLists;
	std::unordered_map<StandardType, SafePtr<StandardList>> m_currentStandards;

	std::unordered_set<SafePtr<UserList>> m_userLists;

	std::unordered_set<SafePtr<sma::TagTemplate>> m_templates;
	SafePtr<sma::TagTemplate> m_currentTemplate;

	SafePtr<ScanNode> m_defaultScan;
	
	std::vector<std::pair<std::filesystem::path, time_t>> m_recentProjects;

	IndexationMethod m_indexationMethod;

    scs::MarkerIcon m_activeIcon;
	ClippingBoxSettings m_clippingSettings;
	DuplicationSettings m_duplicationSettings;
	CreationAttributes	m_creationAttributes;
	bool m_showClusterColor;

	std::filesystem::path m_temporaryPath;
	std::filesystem::path m_projectsPath;

	PolyLineOptions m_polyLineOptions;

	PipeDetectionOptions m_pipeDetectionOptions;


	//** From Project

	ProjectInfos m_info;
	ProjectInternalInfo m_internInfo;
	ProjectInternalInfo m_centralInternInfo;

	bool m_projectLoaded = false;

	std::unordered_map<xg::Guid, UserOrientation> m_userOrientations;
	UserOrientation m_activeUserOrientation;
};

#endif // !CONTROLLER_CONTEXT_H_