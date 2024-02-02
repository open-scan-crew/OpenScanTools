#include "gui/toolBars/ToolBarMarkerDisplayOptions.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"

#include "models/3d/Graph/CameraNode.h"

ToolBarMarkerDisplayOptions::ToolBarMarkerDisplayOptions(IDataDispatcher& dataDispatcher, QWidget *parent, const float& guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
{
    m_ui.setupUi(this);
    setEnabled(false);
    m_ui.lineEdit_far->setFixedWidth(60 * guiScale);
    m_ui.lineEdit_near->setFixedWidth(60 * guiScale);
    m_ui.lineEdit_maxDistance->setFixedWidth(60 * guiScale);
    m_ui.lineEdit_sizeFar->setFixedWidth(60 * guiScale);
    m_ui.lineEdit_sizeNear->setFixedWidth(60 * guiScale);

    m_ui.lineEdit_maxDistance->setType(NumericType::DISTANCE);
    m_ui.lineEdit_near->setType(NumericType::DISTANCE);
    m_ui.lineEdit_far->setType(NumericType::DISTANCE);


    m_ui.lineEdit_sizeNear->setUnit(UnitType::PX);
    m_ui.lineEdit_sizeFar->setUnit(UnitType::PX);


    registerGuiDataFunction(guiDType::renderMarkerDisplayOptions, &ToolBarMarkerDisplayOptions::onInitOptions);
    registerGuiDataFunction(guiDType::projectLoaded, &ToolBarMarkerDisplayOptions::onProjectLoad);
    registerGuiDataFunction(guiDType::renderActiveCamera, &ToolBarMarkerDisplayOptions::onActiveCamera);
    registerGuiDataFunction(guiDType::focusViewport, &ToolBarMarkerDisplayOptions::onFocusViewport);

    connect(m_ui.cb_visibility, &QCheckBox::clicked, this, &ToolBarMarkerDisplayOptions::sendOptions);
    connect(m_ui.lineEdit_maxDistance, &QLineEdit::editingFinished, this, &ToolBarMarkerDisplayOptions::sendOptions);
    connect(m_ui.lineEdit_near, &QLineEdit::editingFinished, this, &ToolBarMarkerDisplayOptions::sendOptions);
    connect(m_ui.lineEdit_far, &QLineEdit::editingFinished, this, &ToolBarMarkerDisplayOptions::sendOptions);
    connect(m_ui.lineEdit_sizeNear, &QLineEdit::editingFinished, this, &ToolBarMarkerDisplayOptions::sendOptions);
    connect(m_ui.lineEdit_sizeFar, &QLineEdit::editingFinished, this, &ToolBarMarkerDisplayOptions::sendOptions);

    m_ui.lineEdit_maxDistance->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);
}

ToolBarMarkerDisplayOptions::~ToolBarMarkerDisplayOptions()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarMarkerDisplayOptions::informData(IGuiData * data)
{
    if (m_functions.find(data->getType()) != m_functions.end())
    {
        GuiDataFunction fct = m_functions.at(data->getType());
        (this->*fct)(data);
    }
}

void ToolBarMarkerDisplayOptions::updateUI()
{
	m_ui.cb_visibility->setChecked(m_storedParameters.improveVisibility);

	m_ui.lineEdit_maxDistance->setValue(m_storedParameters.maximumDisplayDistance);
	m_ui.lineEdit_near->setValue(m_storedParameters.nearLimit);
	m_ui.lineEdit_far->setValue(m_storedParameters.farLimit);

	m_ui.lineEdit_sizeNear->setValue(m_storedParameters.nearSize);
	m_ui.lineEdit_sizeFar->setValue(m_storedParameters.farSize);
}

void ToolBarMarkerDisplayOptions::onProjectLoad(IGuiData * data)
{
    GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
    setEnabled(plData->m_isProjectLoad);
}

void ToolBarMarkerDisplayOptions::onInitOptions(IGuiData *data)
{
    MarkerDisplayOptions params = static_cast<GuiDataMarkerDisplayOptions*>(data)->m_parameters;
	m_storedParameters = params;
	updateUI();
}

void ToolBarMarkerDisplayOptions::onFocusViewport(IGuiData* data)
{
    GuiDataFocusViewport* castData = static_cast<GuiDataFocusViewport*>(data);
    if (castData->m_forceFocus && castData->m_camera)
        m_focusCamera = castData->m_camera;
}

void ToolBarMarkerDisplayOptions::onActiveCamera(IGuiData* data)
{
    auto gui = static_cast<GuiDataCameraInfo*>(data);
    if (gui->m_camera && m_focusCamera != gui->m_camera)
        return;

    ReadPtr<CameraNode> rCam = m_focusCamera.cget();
    if (!rCam)
        return;

    m_storedParameters = rCam->getDisplayParameters().m_markerOptions;
    updateUI();
}

void ToolBarMarkerDisplayOptions::sendOptions()
{
    MarkerDisplayOptions options = {
        m_ui.cb_visibility->isChecked(),
		m_ui.lineEdit_maxDistance->getValue(),
        (float)m_ui.lineEdit_near->getValue(),
        (float)m_ui.lineEdit_far->getValue(),
        (float)m_ui.lineEdit_sizeNear->getValue(),
        (float)m_ui.lineEdit_sizeFar->getValue()
    };

	m_storedParameters = options;
	updateUI();

    m_dataDispatcher.updateInformation(new GuiDataMarkerDisplayOptions(m_storedParameters, m_focusCamera), this);
}
