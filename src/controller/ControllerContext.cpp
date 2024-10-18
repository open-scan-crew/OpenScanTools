#include "controller/ControllerContext.h"

// Data Model
#include "models/graph/ScanNode.h"

#include "utils/ProjectColor.hpp"
#include "utils/ProjectStringSets.hpp"
#include "models/application/Author.h"

ControllerContext::ControllerContext()
	: m_activeIcon(scs::MarkerIcon::Tag_Base)
	, m_renderPointSize(1.f)
	, m_backgroundColors({ Color32(0,0,0,255), Color32(96,96,96,255), Color32(255,255,255,255) })
	, m_currentBackgroundColor(0)
	, m_showClusterColor(false)
	, m_activeAuthor()
{
	ProjectColor::registerColor("GREEN", Color32(32, 123, 64, 255));
	ProjectColor::registerColor("RED", Color32(235, 90, 70, 255));
	ProjectColor::registerColor("ORANGE", Color32(255, 159, 26, 255));
	ProjectColor::registerColor("YELLOW", Color32(242, 214, 0, 255));
	ProjectColor::registerColor("BLUE", Color32(0, 121, 191, 255));
	ProjectColor::registerColor("PURPLE", Color32(195, 119, 224, 255));
	ProjectColor::registerColor("BROWN", Color32(139, 95, 60));
	ProjectColor::registerColor("LIGHT GREY", Color32(179, 186, 197, 255));

	ProjectColor::registerColor("WHITE", Color32(255, 255, 255, 255));
	ProjectColor::registerColor("OLIVE", Color32(128, 128, 0));
	ProjectColor::registerColor("DARK GREEN", Color32(42, 110, 85, 255));
	ProjectColor::registerColor("TURQUOISE", Color32(64, 224, 208));
	ProjectColor::registerColor("VIOLET", Color32(153, 50, 204));
	ProjectColor::registerColor("ULIGHT GREY", Color32(192, 192, 192));
	ProjectColor::registerColor("BLACK", Color32(0, 0, 0, 255));

	ProjectStringSets::registerStringSet(L"ACTION", std::vector<std::wstring>({ L"", L"Calibrate", L"Check", L"Clean", L"Create", L"Model", L"Model all", L"Remove", L"Replace" }));
	ProjectStringSets::registerStringSet(L"DISCIPLINE", std::vector<std::wstring>({ L"NA", L"All", L"Misc.", L"Architecture", L"Asbestos", L"Alarm", L"Certification", L"Civil engineering", L"Cladding", L"Demolition", L"Diagnostic", L"Dismantling", L"Doors and openings", L"Electricity", L"Fire protection", L"Flooring", L"Framework", L"Furnitures", L"Heat production", L"HVAC", L"Inspection", L"Investigation", L"Insulation", L"Joineries", L"Lifting", L"Maintenance", L"Measurement", L"Metalwork", L"Modeling", L"Partition", L"Piping", L"Plumbing", L"Power generation", L"Recycling", L"Refrigeration", L"Roofing", L"Scaffolds", L"Security", L"Special equipment", L"Special fluids", L"Storage", L"Surface treatment", L"Water treatment" }));
	ProjectStringSets::registerStringSet(L"PREFIX", std::vector<std::wstring>({ L"", L"CIV", L"CPR", L"CTR", L"DUC", L"ELE", L"INS", L"MWO", L"PIP", L"PMP", L"SAF", L"STR", L"VLV", L"WAL" }));
	ProjectStringSets::registerStringSet(L"PHASE", std::vector<std::wstring>({ L"NA", L"All", L"New", L"Project", L"Audit", L"Diagnostic", L"Call for tenders", L"Technical studies", L"Existing", L"Construction", L"Reception", L"Warranty", L"Production", L"Maintenance", L"Demolition", L"Dismantling", L"P1", L"P2", L"P3", L"P4", L"P5", L"P6", L"P7", L"P8", L"P9", L"P10" }));

	ProjectColor::registerColor("RED_DISABLED", Color32(231, 76, 60, 170));
	ProjectColor::registerColor("BLUE_DISABLED", Color32(0, 111, 189, 170));
	ProjectColor::registerColor("GREEN_DISABLED", Color32(83, 219, 170, 170));
	ProjectColor::registerColor("YELLOW_DISABLED", Color32(241, 196, 15, 170));
	ProjectColor::registerColor("PURPLE_DISABLED", Color32(155, 89, 182, 170));
	ProjectColor::registerColor("ORANGE_DISABLED", Color32(230, 126, 34, 170));
	ProjectColor::registerColor("LIGHT GREY_DISABLED", Color32(180, 180, 180, 170));
	ProjectColor::registerColor("BLACK_DISABLED", Color32(0, 0, 0, 170));

	ProjectColor::registerColor("RED_HOVERED", Color32(251, 96, 80, 255));
	ProjectColor::registerColor("BLUE_HOVERED", Color32(20, 131, 209, 255));
	ProjectColor::registerColor("GREEN_HOVERED", Color32(103, 239, 190, 255));
	ProjectColor::registerColor("DARK GREEN_HOVERED", Color32(52, 120, 95, 255));
	ProjectColor::registerColor("YELLOW_HOVERED", Color32(255, 216, 35, 255));
	ProjectColor::registerColor("PURPLE_HOVERED", Color32(175, 109, 202, 255));
	ProjectColor::registerColor("ORANGE_HOVERED", Color32(250, 146, 54, 255));
	ProjectColor::registerColor("LIGHT GREY_HOVERED", Color32(200, 200, 200, 255));
	ProjectColor::registerColor("BLACK_HOVERED", Color32(20, 20, 20, 255));

	ProjectColor::registerColor("RED_ACTIVE", Color32(211, 56, 40, 255));
	ProjectColor::registerColor("BLUE_ACTIVE", Color32(0, 91, 169, 255));
	ProjectColor::registerColor("GREEN_ACTIVE", Color32(63, 199, 150, 255));
	ProjectColor::registerColor("DARK GREEN_ACTIVE", Color32(32, 100, 75, 255));
	ProjectColor::registerColor("YELLOW_ACTIVE", Color32(221, 176, 0, 255));
	ProjectColor::registerColor("PURPLE_ACTIVE", Color32(135, 69, 162, 255));
	ProjectColor::registerColor("ORANGE_ACTIVE", Color32(210, 106, 14, 255));
	ProjectColor::registerColor("LIGHT GREY_ACTIVE", Color32(160, 160, 160, 255));
	ProjectColor::registerColor("BLACK_ACTIVE", Color32(0, 0, 0, 255));

	m_creationAttributes.color = ProjectColor::getColor("BLUE");

	m_activeIcon = scs::MarkerIcon::Tag_Base;
	m_clippingSettings.size = glm::vec3(1.0f, 1.0f, 1.0f);
	m_clippingSettings.angleZ = 0.0;
	m_clippingSettings.offset = ClippingBoxOffset::CenterOnPoint;
	m_duplicationSettings.type = DuplicationMode::Click;
	m_duplicationSettings.isLocal = false;
	m_duplicationSettings.offset = glm::dvec3(1.0);
	m_duplicationSettings.step = glm::ivec3(0);
}

ControllerContext::~ControllerContext()
{
}

bool ControllerContext::getIsCurrentProjectSaved() const
{
	return (m_isCurrentProjectSaved);
}

std::unordered_set<SafePtr<StandardList>>& ControllerContext::getStandards(const StandardType& type)
{
	return m_standardsLists[type];
}

SafePtr<StandardList> ControllerContext::getStandard(const StandardType& type, const listId& id) const
{
	SafePtr<StandardList> list;
	if (m_standardsLists.find(type) == m_standardsLists.end())
		return list;

	for (const SafePtr<StandardList>& standard : m_standardsLists.at(type))
	{
		ReadPtr<StandardList> rStandard = standard.cget();
		if (rStandard && rStandard->getId() == id)
		{
			list = standard;
			break;
		}
	}
	return list;
}

std::unordered_set<SafePtr<UserList>>& ControllerContext::getUserLists()
{
	return (m_userLists);
}

SafePtr<UserList> ControllerContext::getUserList(listId id) const
{
	for (const SafePtr<UserList>& list : m_userLists)
	{
		ReadPtr<UserList> rList = list.cget();
		if (rList && rList->getId() == id)
			return list;
	}
	return SafePtr<UserList>();
}

std::unordered_set<SafePtr<sma::TagTemplate>>& ControllerContext::getTemplates()
{
	return (m_templates);
}

const std::unordered_set<SafePtr<sma::TagTemplate>>& ControllerContext::cgetTemplates() const
{
	return (m_templates);
}

bool ControllerContext::verifNameForList(std::wstring nameToTest)
{
	for (SafePtr<UserList> ul : m_userLists)
	{
		ReadPtr<UserList> rUl = ul.cget();
		if(rUl && rUl->getName() == nameToTest)
			return (false);
	}
	return (true);
}

bool ControllerContext::verifNameForTemplate(std::wstring nameToTest)
{
	for (SafePtr<sma::TagTemplate> temp : m_templates)
	{
		ReadPtr<sma::TagTemplate> rTemp = temp.cget();
		if (rTemp && rTemp->getName() == nameToTest)
			return (false);
	}
	return (true);
}

bool ControllerContext::verifNameForStandards(StandardType type, std::wstring nameToTest)
{
	for (SafePtr<StandardList> stand : m_standardsLists[type])
	{
		ReadPtr<StandardList> rStand = stand.cget();
		if (rStand && rStand->getName() == nameToTest)
			return (false);
	}
	return (true);
}

ClippingBoxSettings ControllerContext::getClippingSettings()
{
	return (m_clippingSettings);
}

void ControllerContext::setCurrentTemplate(SafePtr<sma::TagTemplate> temp)
{
	m_currentTemplate = temp;
}

void ControllerContext::setActiveColor(Color32 color)
{
	m_creationAttributes.color = color;
}

void ControllerContext::setActiveName(const std::wstring& name)
{
	m_creationAttributes.name = name;
}

void ControllerContext::setActiveIdentifer(const std::wstring& identifier)
{
	m_creationAttributes.identifier = identifier;
}

void ControllerContext::setActiveDiscipline(const std::wstring& discipline)
{
	m_creationAttributes.discipline = discipline;
}

void ControllerContext::setActivePhase(const std::wstring& phase)
{
	m_creationAttributes.phase = phase;
}

void ControllerContext::setActiveIcon(scs::MarkerIcon icon)
{
	m_activeIcon = icon;
}

SafePtr<sma::TagTemplate> ControllerContext::getCurrentTemplate() const
{
	return (m_currentTemplate);
}

const Color32& ControllerContext::getActiveColor() const
{
	return (m_creationAttributes.color);
}

const std::wstring& ControllerContext::getActiveName() const
{
	return (m_creationAttributes.name);
}

const std::wstring& ControllerContext::getActiveIdentifer() const
{
	return (m_creationAttributes.identifier);
}

const std::wstring& ControllerContext::getActiveDiscipline() const
{
	return (m_creationAttributes.discipline);
}

const std::wstring& ControllerContext::getActivePhase() const
{
	return (m_creationAttributes.phase);
}

scs::MarkerIcon ControllerContext::getActiveIcon() const
{
    return (m_activeIcon);
}

// QUESTION(robin) - Should this function exist ??
// ANSWER(Alexis) - it was useless before, now it has more logic
// Maybe I could search a bit more to find if it can be replaced
// UPDATE(Alexis) - I don't know, maybe
// UPDATE(Alexis) - Life is a kiwi :thumbsup:
// UPDATE(Alexis) - Maybe a day this "thread" will be readen
// UPDATE(Aurélien) - We need to keep the state so I guess we can keep it 
//						(and reduce the access scope to the controller only, 
//								(like a lot of stuff in the context :))
void ControllerContext::setIsCurrentProjectSaved(bool value)
{
	m_isCurrentProjectSaved = value;
}

bool ControllerContext::setUserLists(std::vector<UserList> list, bool reset)
{
	if (list.empty())
		return false;
	
	if (reset == true)
		m_userLists.clear();

	std::unordered_set<SafePtr<UserList>> newTemplates;

	for (UserList ulData : list)
	{
		bool editedList = false;
		for (SafePtr<UserList> ul : m_userLists)
		{
			WritePtr<UserList> wUl = ul.get();
			if (!wUl || ulData.getId() != wUl->getId())
				continue;

			wUl->mergeList(ulData);
			editedList = true;
			break;
		}

		if (!editedList)
			newTemplates.insert(make_safe<UserList>(ulData));
	}

	m_userLists.merge(newTemplates);

	return true;
}

void ControllerContext::removeUserLists(SafePtr<UserList> list)
{
	m_userLists.erase(list);
	list.destroy();
}

bool ControllerContext::setStandards(std::vector<StandardList> list, const StandardType& type, bool reset)
{
	if (list.empty())
		return false;
	
	if (m_standardsLists.find(type) == m_standardsLists.end())
		m_standardsLists[type] = std::unordered_set<SafePtr<StandardList>>();

	if (reset == true)
		m_standardsLists[type].clear();

	bool dontContainNA = true;

	std::unordered_set<SafePtr<StandardList>> newStandard;

	for (const StandardList& standardData : list)
	{
		bool editedTemp = false;
		if (standardData.clist().empty())
			dontContainNA = false;

		for (SafePtr<StandardList> standard : m_standardsLists[type])
		{
			WritePtr<StandardList> wStd = standard.get();
			if (!wStd)
				continue;

			if (wStd->clist().empty())
				dontContainNA = false;

			if (standardData.getId() != wStd->getId())
				continue;

			wStd->mergeList(standardData);
			editedTemp = true;
		}

		if (!editedTemp)
			newStandard.insert(make_safe<StandardList>(standardData));
	}

	if (dontContainNA)
	{
		StandardList std(L"N.A.");
		std.setOrigin(true);
		m_standardsLists.at(type).insert(make_safe<StandardList>(std));
	}

	m_standardsLists[type].merge(newStandard);

	return true;
}

bool ControllerContext::setTemplates(std::vector<sma::TagTemplate> newTemps, bool reset)
{
	if (newTemps.empty())
		return false;
	
	if (reset == true)
		m_templates.clear();

	std::unordered_set<SafePtr<sma::TagTemplate>> newTemplates;

	for (const sma::TagTemplate& newTempData : newTemps)
	{
		bool editedTemp = false;
		for (SafePtr<sma::TagTemplate> temp : m_templates)
		{
			WritePtr<sma::TagTemplate> wTemp = temp.get();
			if (!wTemp || newTempData.getId() != wTemp->getId())
				continue;

			wTemp->mergeTemplate(newTempData);
			editedTemp = true;
			break;
		}

		if (!editedTemp)
			newTemplates.insert(make_safe<sma::TagTemplate>(newTempData));
	}

	m_templates.merge(newTemplates);

	return true;
}

void ControllerContext::setClippingSettings(ClippingBoxSettings settings)
{
	m_clippingSettings = settings;
}

bool ControllerContext::getShowClusterColors() const
{
	return m_showClusterColor;
}

void ControllerContext::setShowClusterColors(bool show)
{
	m_showClusterColor = show;
}

const ClippingBoxSettings& ControllerContext::CgetClippingSettings() const
{
	return m_clippingSettings;
}

const PolyLineOptions& ControllerContext::getPolyLineOptions()
{
	return m_polyLineOptions;
}

SafePtr<Author> ControllerContext::getActiveAuthor() const
{
	return m_activeAuthor;
}

std::unordered_set<SafePtr<Author>> ControllerContext::getLocalAuthors() const
{
	return m_localAuthors;
}

std::unordered_set<SafePtr<Author>> ControllerContext::getProjectAuthors() const
{
	return m_projAuthors;
}

void ControllerContext::setClippingSize(const glm::vec3& size)
{
	m_clippingSettings.size = size;
}

void ControllerContext::setClippingOffset(const ClippingBoxOffset& offset)
{
	m_clippingSettings.offset = offset;
}

void ControllerContext::setClippingAngleZValue(const double& angle)
{
	m_clippingSettings.angleZ = angle;
}

void ControllerContext::setPolyLineOptions(const PolyLineOptions& options)
{
	m_polyLineOptions = options;
}

SafePtr<Author> ControllerContext::createAuthor(const Author& newAuthor) const
{
	xg::Guid authorId = newAuthor.getId();
	SafePtr<Author> newSafeAuthor;
	for (SafePtr<Author> author : m_contextKnownAuthors)
	{
		ReadPtr<Author> rAuthor = author.cget();
		if (rAuthor && rAuthor->getId() == authorId)
		{
			newSafeAuthor = author;
			break;
		}
	}

	if (!newSafeAuthor)
		newSafeAuthor = make_safe<Author>(newAuthor);

	return newSafeAuthor;
}

void ControllerContext::setActiveAuthor(const SafePtr<Author>& activeAuthor)
{
	m_activeAuthor = activeAuthor;
}

void addAuthorToSet(const SafePtr<Author>& auth, std::unordered_set<SafePtr<Author>>& set)
{
	xg::Guid idAuth;
	{
		ReadPtr<Author> rAuth = auth.cget();
		if (rAuth)
			idAuth = rAuth->getId();
	}

	if (!idAuth.isValid())
		return;

	for (const SafePtr<Author>& a : set)
	{
		xg::Guid idA;
		ReadPtr<Author> rA = a.cget();
		if (rA)
			idA = rA->getId();
		if (idA == idAuth)
			return;
	}

	set.insert(auth);
}

void ControllerContext::addProjectAuthors(const std::unordered_set<SafePtr<Author>>& authors)
{
	for (const SafePtr<Author>& auth : authors)
	{
		addAuthorToSet(auth, m_contextKnownAuthors);
		addAuthorToSet(auth, m_projAuthors);
	}
}

void ControllerContext::addLocalAuthors(const std::unordered_set<SafePtr<Author>>& authors)
{
	for (const SafePtr<Author>& auth : authors)
	{
		addAuthorToSet(auth, m_contextKnownAuthors);
		addAuthorToSet(auth, m_localAuthors);
	}
}

void ControllerContext::remLocalAuthors(const std::unordered_set<SafePtr<Author>>& authors)
{
	for (const SafePtr<Author>& auth : authors)
		m_localAuthors.erase(auth);
}

DuplicationSettings ControllerContext::getDuplicationSettings()
{
	return m_duplicationSettings;
}

const DuplicationSettings& ControllerContext::CgetDuplicationSettings() const
{
	return m_duplicationSettings;
}

void ControllerContext::setDuplicationSettings(DuplicationSettings settings)
{
	m_duplicationSettings = settings;
}

const std::filesystem::path& ControllerContext::getProjectsPath() const
{
	return m_projectsPath;
}

const std::filesystem::path& ControllerContext::getTemporaryPath() const
{
	return m_temporaryPath;
}

void ControllerContext::setProjectsPath(const std::filesystem::path& path)
{
	m_projectsPath = path;
}

void ControllerContext::setRenderPointSize(float pointSize)
{
    m_renderPointSize = pointSize;
}

void ControllerContext::setTemporaryPath(const std::filesystem::path& path)
{
	m_temporaryPath = path;
}

void ControllerContext::setUserBackgroundColor(const Color32& color, const uint32_t& position)
{
	if (position >= m_backgroundColors.size())
		m_backgroundColors.push_back(color);
	else
		m_backgroundColors[position] = color;
	//m_currentBackgroundColor = position > m_backgroundColors.size() ? position : m_backgroundColors.size() - 1;
}

float ControllerContext::getRenderPointSize() const
{
    return m_renderPointSize;
}

Color32 ControllerContext::getActiveBackgroundColor()
{
	if (!m_backgroundColors.empty())
	{
		if (m_currentBackgroundColor > m_backgroundColors.size())
			m_currentBackgroundColor = 0;
		return m_backgroundColors[m_currentBackgroundColor];
	}
	return Color32(0, 0, 0, 1);
}

DecimationOptions ControllerContext::getDecimationOptions()
{
    return m_decimationOptions;
}

void ControllerContext::setDecimationOptions(const DecimationOptions& options)
{
    m_decimationOptions = options;
}

Color32 ControllerContext::getNextBackgroundColor()
{
	if (!m_backgroundColors.empty())
	{
		m_currentBackgroundColor = ++m_currentBackgroundColor % m_backgroundColors.size();
		return m_backgroundColors[m_currentBackgroundColor];
	}
	return Color32(0, 0, 0, 1);
}

void ControllerContext::setCurrentStandard(const SafePtr<StandardList>& standard, const StandardType& type)
{
	m_currentStandards[type] = standard;
}

SafePtr<StandardList> ControllerContext::getCurrentStandard(const StandardType& type) const
{
	//assert(m_currentStandards.find(type) != m_currentStandards.end());
	if(m_currentStandards.find(type) != m_currentStandards.end())
		return (m_currentStandards.at(type));
	return SafePtr<StandardList>();
}

void ControllerContext::setProjectTransformation(const glm::dvec3& projectTransformation)
{
	m_projectTransformation = projectTransformation;
}

void ControllerContext::setDefaultScanId(SafePtr<ScanNode> defaultScan)
{
	m_defaultScan = defaultScan;
}

SafePtr<ScanNode> ControllerContext::getDefaultScan()
{
	return m_defaultScan;
}

void ControllerContext::setPipeDetectionOptions(const PipeDetectionOptions& pipeDetectionOptions)
{
	m_pipeDetectionOptions = pipeDetectionOptions;
}

void ControllerContext::setRecentProjects(std::vector<std::pair<std::filesystem::path, time_t>> projects)
{
	m_recentProjects = projects;
}

void ControllerContext::setIndexationMethod(IndexationMethod method)
{
	m_indexationMethod = method;
}

const std::vector<std::pair<std::filesystem::path, time_t>> &ControllerContext::getRecentProjects() const
{
	return m_recentProjects;
}

const IndexationMethod& ControllerContext::getIndexationMethod() const
{
	return m_indexationMethod;
}

const PipeDetectionOptions& ControllerContext::getPipeDetectionOptions() const
{
	return m_pipeDetectionOptions;
}

void ControllerContext::initProjectInfo(std::filesystem::path projectFolder, const ProjectInfos& info)
{
	m_info = info;
	m_internInfo.setProjectFolderPath(projectFolder, m_info.m_projectName.wstring());
	m_internInfo.setCustomScanFolderPath(info.m_customScanFolderPath);
	if (std::filesystem::exists(info.m_centralProjectPath))
		m_centralInternInfo.setProjectFolderPath(info.m_centralProjectPath.parent_path(), info.m_centralProjectPath.stem().wstring());
	m_projectLoaded = true;
}

void ControllerContext::cleanProjectInfo()
{
	m_info = ProjectInfos();
	m_internInfo = ProjectInternalInfo();
	m_centralInternInfo = ProjectInternalInfo();
	m_userOrientations.clear();
	m_projectLoaded = false;
}

bool ControllerContext::isProjectLoaded()
{
	return m_projectLoaded;
}

void ControllerContext::setProjectInfo(const ProjectInfos& info)
{
	m_info = info;
	m_internInfo.setCustomScanFolderPath(info.m_customScanFolderPath);
}

ProjectInfos& ControllerContext::getProjectInfo()
{
	return m_info;
}

const ProjectInfos& ControllerContext::cgetProjectInfo() const
{
	return m_info;
}

const ProjectInternalInfo& ControllerContext::cgetProjectInternalInfo() const
{
	return m_internInfo;
}

const ProjectInternalInfo& ControllerContext::cgetProjectCentralInternalInfo() const
{
	return m_centralInternInfo;
}

std::unordered_map<userOrientationId, UserOrientation>& ControllerContext::getUserOrientations()
{
	return m_userOrientations;
}

const std::unordered_map<userOrientationId, UserOrientation>& ControllerContext::cgetUserOrientations() const
{
	return m_userOrientations;
}

const UserOrientation& ControllerContext::getActiveUserOrientation() const
{
	return m_activeUserOrientation;
}

void ControllerContext::setActiveUserOrientation(const UserOrientation& uo)
{
	m_activeUserOrientation = uo;
}
