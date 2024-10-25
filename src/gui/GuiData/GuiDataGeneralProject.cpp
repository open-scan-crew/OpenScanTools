#include "gui/GuiData/GuiDataGeneralProject.h"

#include "models/graph/AGraphNode.h"
#include "controller/ControllerContext.h"
#include "models/graph/GraphManager.hxx"

#include "utils/Logger.h"
#include "io/FileUtils.h"

#include <unordered_set>


// **** GuiDataNewProject ****

GuiDataNewProject::GuiDataNewProject(const std::filesystem::path& folder, const std::vector<std::filesystem::path>& templates)
	: m_folder(folder)
	, m_templates(templates)
{ }

GuiDataNewProject::~GuiDataNewProject()
{ }

guiDType GuiDataNewProject::getType()
{
	return (guiDType::newProject);
}

// **** GuiDataQuit ****

GuiDataQuit::GuiDataQuit()
{ }

GuiDataQuit::~GuiDataQuit()
{ }

guiDType GuiDataQuit::getType()
{
	return (guiDType::quitEvent);
}

// **** GuiDataAbort ****

GuiDataAbort::GuiDataAbort()
{}

GuiDataAbort::~GuiDataAbort()
{}

guiDType GuiDataAbort::getType()
{
	return (guiDType::abortEvent);
}


// **** GuiDataProjectLoaded ****

//projectMode is true for white project
GuiDataProjectLoaded::GuiDataProjectLoaded(bool projectLoad, const std::wstring& name)
    : m_isProjectLoad(projectLoad)
    , m_projectName(name)
{ }

GuiDataProjectLoaded::~GuiDataProjectLoaded()
{ }

guiDType GuiDataProjectLoaded::getType()
{
	return (guiDType::projectLoaded);
}

// **** GuiDataProjectTemplateList***

GuiDataProjectTemplateList::GuiDataProjectTemplateList(const std::vector<std::filesystem::path>& templates)
	: m_templates(templates)
{}

GuiDataProjectTemplateList::~GuiDataProjectTemplateList()
{}

guiDType GuiDataProjectTemplateList::getType()
{
	return (guiDType::projectTemplateList);
}

// **** GuiDataHidePropertyPanels ****

GuiDataHidePropertyPanels::GuiDataHidePropertyPanels()
    : hideProjectProperties(true)
    , hideScanProperties(true)
    , hideTagProperties(true)
    , hideClusterProperties(true)
{ }

GuiDataHidePropertyPanels::GuiDataHidePropertyPanels(bool project, bool scan, bool tag, bool cluster)
    : hideProjectProperties(project)
    , hideScanProperties(scan)
    , hideTagProperties(tag)
    , hideClusterProperties(cluster)
{ }

GuiDataHidePropertyPanels::~GuiDataHidePropertyPanels()
{ }

guiDType GuiDataHidePropertyPanels::getType()
{
    return (guiDType::hidePropertyPanels);
}

// **** GuiDataCameraPos ****

GuiDataCameraPos::GuiDataCameraPos()
{
	pos = Pos3D();
}

GuiDataCameraPos::GuiDataCameraPos(const Pos3D & npos)
{
	pos = npos;
}

GuiDataCameraPos::~GuiDataCameraPos()
{}

guiDType GuiDataCameraPos::getType()
{
	return (guiDType::cameraData);
}

// **** GuiDataScanCurrent ****

GuiDataScanCurrent::GuiDataScanCurrent(const Pos3D & npos)
{
	pos = npos;
	exists = true;
}

GuiDataScanCurrent::GuiDataScanCurrent()
{
	exists = false;
	pos = Pos3D();
}

guiDType GuiDataScanCurrent::getType()
{
	return (guiDType::currentScanData);
}

// **** GuiDataUndoRedoAble ****

GuiDataUndoRedoAble::GuiDataUndoRedoAble(bool undo, bool redo)
{
	_undoAble = undo;
	_redoAble = redo;
}

guiDType GuiDataUndoRedoAble::getType()
{
	return (guiDType::undoRedoData);
}

// **** GuiDataObjectSelected ****

GuiDataObjectSelected::GuiDataObjectSelected(SafePtr<AGraphNode> object)
	: m_object(object)
{
	ReadPtr<AGraphNode> rObj = m_object.cget();
	if (rObj)
		m_type = rObj->getType();
}

GuiDataObjectSelected::GuiDataObjectSelected()
{}

GuiDataObjectSelected::~GuiDataObjectSelected()
{}

guiDType GuiDataObjectSelected::getType()
{
	return guiDType::objectSelected;
}


// **** GuiDataObjectSelected ****

GuiDataMultiObjectProperties::GuiDataMultiObjectProperties(std::unordered_set<SafePtr<AGraphNode>> objects)
	: m_objects(objects)
{
}

GuiDataMultiObjectProperties::~GuiDataMultiObjectProperties()
{}

guiDType GuiDataMultiObjectProperties::getType()
{
	return guiDType::multiObjectProperties;
}

// **** GuiDataTolerances ****

GuiDataTolerances::GuiDataTolerances(const double& bending, const double& tilt)
	: m_bending(bending)
	, m_tilt(tilt)
{}

GuiDataTolerances::~GuiDataTolerances()
{}

guiDType GuiDataTolerances::getType()
{
	return (guiDType::tolerancesProperties);
}

// **** GuiDataDefaultClipParams ****
GuiDataDefaultClipParams::GuiDataDefaultClipParams(float min, float max, ClippingMode mode)
	: m_minClipDistance(min)
	, m_maxClipDistance(max)
	, m_mode(mode)
{}

GuiDataDefaultClipParams::~GuiDataDefaultClipParams()
{}

guiDType GuiDataDefaultClipParams::getType()
{
	return (guiDType::defaultClipParams);
}

// **** GuiDataDefaultRampParams ****
GuiDataDefaultRampParams::GuiDataDefaultRampParams(float min, float max, int steps)
	: m_minRampDistance(min)
	, m_maxRampDistance(max)
	, m_steps(steps)
{}

GuiDataDefaultRampParams::~GuiDataDefaultRampParams()
{}

guiDType GuiDataDefaultRampParams::getType()
{
	return (guiDType::defaultRampParams);
}

// **** Project Properties ****

GuiDataProjectProperties::GuiDataProjectProperties(const ControllerContext& context, const GraphManager& graphManager, bool openProperties)
	: m_scans(graphManager.getNodesByTypes({ ElementType::Scan }))
{
	m_projectInfo = context.cgetProjectInfo();
	m_projectPointsCount = graphManager.getProjectPointsCount();
	m_projectActiveScansCount = graphManager.getVisiblePointCloudInstances(xg::Guid(), true, false).size();
	m_projectScansCount = graphManager.getPointCloudInstances(xg::Guid(), true, false, ObjectStatusFilter::ALL).size();

	m_openProperties = openProperties;
}

guiDType GuiDataProjectProperties::getType()
{
	if (m_openProperties)
		return (guiDType::projectDataProperties);
	else
		return (guiDType::projectDataPropertiesNoOpen);
}

// **** Activated Functions ****

GuiDataActivatedFunctions::GuiDataActivatedFunctions(const ContextType& type)
	: type(type)
{}

guiDType GuiDataActivatedFunctions::getType()
{
	return (guiDType::activatedFunctions);
}

// **** Clipping Export Parameters Display ****

GuiDataDeletePointsDialogDisplay::GuiDataDeletePointsDialogDisplay(const std::vector<SafePtr<AClippingNode>>& clippings)
    : m_clippings(clippings)
{}

GuiDataDeletePointsDialogDisplay::~GuiDataDeletePointsDialogDisplay()
{
}

guiDType GuiDataDeletePointsDialogDisplay::getType()
{
    return (guiDType::deletePointsDialogDisplay);
}

//++++++ Global ColorPicker value +++++++

GuiDataGlobalColorPickerValue::GuiDataGlobalColorPickerValue(const Color32& color, const bool& picked)
	: m_color(color)
	, m_isPicked(picked)
{}

guiDType GuiDataGlobalColorPickerValue::getType()
{
	return guiDType::globalColorPickerValue;
}

// **** SendAuthors List ****

GuiDataSendAuthorsList::GuiDataSendAuthorsList(const std::unordered_set<SafePtr<Author>>& authors, const bool& projectScope)
	: m_isProjectScope(projectScope)
	, m_authors(authors)
	, m_selectedAuthor(-1)
{}

GuiDataSendAuthorsList::GuiDataSendAuthorsList(const std::unordered_set<SafePtr<Author>>& authors, const int& selectedAuthor)
	: m_isProjectScope(false)
	, m_authors(authors)
	, m_selectedAuthor(selectedAuthor)
{}

GuiDataSendAuthorsList::~GuiDataSendAuthorsList()
{ }

guiDType GuiDataSendAuthorsList::getType()
{
	return guiDType::sendAuthorsList;
}

// **** CloseAuthors List ****

GuiDataCloseAuthorsList::GuiDataCloseAuthorsList()
{
}

GuiDataCloseAuthorsList::~GuiDataCloseAuthorsList()
{ }

guiDType GuiDataCloseAuthorsList::getType()
{
	return guiDType::closeAuthorList;
}

// **** GuiDataAuthorSelection ****

GuiDataAuthorNameSelection::GuiDataAuthorNameSelection(const std::wstring& author)
	: m_author(author)
{}

guiDType GuiDataAuthorNameSelection::getType()
{
	return (guiDType::authorSelection);
}

// **** GuiDataAuthorSelection ****

GuiDataNameSelection::GuiDataNameSelection(const std::wstring& name)
	: m_name(name)
{}

guiDType GuiDataNameSelection::getType()
{
	return (guiDType::nameSelection);
}

// **** GuiDataAuthorSelection ****

GuiDataIdentifierSelection::GuiDataIdentifierSelection(const std::wstring& identifier)
	: m_identifier(identifier)
{}

guiDType GuiDataIdentifierSelection::getType()
{
	return (guiDType::identifierSelection);
}

// **** GuiDataFocusViewport ****

GuiDataFocusViewport::GuiDataFocusViewport(SafePtr<CameraNode> camera, int width, int height, bool forceFocus)
	: m_camera(camera)
	, m_width(width)
	, m_height(height)
	, m_forceFocus(forceFocus)
{}

guiDType GuiDataFocusViewport::getType() 
{
	return (guiDType::focusViewport);
}


// **** GuiDataExamineCentering ****
GuiDataExamineOptions::GuiDataExamineOptions(bool targetCentering, bool keepOnPan)
	: m_targetCentering(targetCentering)
	, m_keepOnPan(keepOnPan)
{}

GuiDataExamineOptions::~GuiDataExamineOptions()
{}

guiDType GuiDataExamineOptions::getType()
{
	return guiDType::examineOptions;
}

// **** GuiDataDeleteFileDependantObjectDialog ****

GuiDataDeleteFileDependantObjectDialog::GuiDataDeleteFileDependantObjectDialog(const std::unordered_map<SafePtr<AGraphNode>, std::pair<QString, QString>>& importantData, const std::unordered_set<SafePtr<AGraphNode>> & otherData)
	: m_importantData(importantData)
	, m_otherData(otherData)
{}

GuiDataDeleteFileDependantObjectDialog::~GuiDataDeleteFileDependantObjectDialog()
{}

guiDType GuiDataDeleteFileDependantObjectDialog::getType()
{
	return guiDType::deleteScanDialog;
}

// **** GuiDataProjectTemplateManagerDisplay ****

GuiDataProjectTemplateManagerDisplay::GuiDataProjectTemplateManagerDisplay()
{}

GuiDataProjectTemplateManagerDisplay::~GuiDataProjectTemplateManagerDisplay()
{}

guiDType GuiDataProjectTemplateManagerDisplay::getType()
{
	return guiDType::projectTemplateDialog;
}

// **** GuiDataDisableFullScreen ****

GuiDataDisableFullScreen::GuiDataDisableFullScreen()
{}

GuiDataDisableFullScreen::~GuiDataDisableFullScreen()
{}

guiDType GuiDataDisableFullScreen::getType()
{
	return guiDType::disableFullScreen;
}

// **** GuiDataActualizeData ****

GuiDataSelectItems::GuiDataSelectItems(const std::unordered_set<SafePtr<AGraphNode>>& objects)
	: m_objects(objects)
{
}

GuiDataSelectItems::~GuiDataSelectItems()
{
}

guiDType GuiDataSelectItems::getType()
{
	return guiDType::selectElems;
}