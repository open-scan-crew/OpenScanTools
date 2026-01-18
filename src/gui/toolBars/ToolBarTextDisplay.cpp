#include "gui/toolBars/ToolBarTextDisplay.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "models/graph/CameraNode.h"

#include <QSignalBlocker>

ToolBarTextDisplay::ToolBarTextDisplay(IDataDispatcher& dataDispatcher, QWidget *parent, const float& guiScale) :
	QWidget(parent),
	m_dataDispatcher(dataDispatcher),
	m_textFilter(13)
{
	m_ui.setupUi(this);
	setEnabled(false);


	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::renderActiveCamera);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::focusViewport);

	m_methods.insert({ guiDType::projectLoaded, &ToolBarTextDisplay::onProjectLoad });
	m_methods.insert({ guiDType::renderActiveCamera, &ToolBarTextDisplay::onActiveCamera });
	m_methods.insert({ guiDType::focusViewport, &ToolBarTextDisplay::onFocusViewport });

	connect(m_ui.indexBox, &QCheckBox::clicked, this, [this](bool checked) {this->toggleRenderParameter(checked, TEXT_SHOW_INDEX_BIT); });
	connect(m_ui.authorBox, &QCheckBox::clicked, this, [this](bool checked) {this->toggleRenderParameter(checked, TEXT_SHOW_AUTHOR_BIT); });
	connect(m_ui.identifierBox, &QCheckBox::clicked, this, [this](bool checked) {this->toggleRenderParameter(checked, TEXT_SHOW_IDENTIFIER_BIT); });
	connect(m_ui.nameBox, &QCheckBox::clicked, this, [this](bool checked) {this->toggleRenderParameter(checked, TEXT_SHOW_NAME_BIT); });
	connect(m_ui.disciplineBox, &QCheckBox::clicked, this, [this](bool checked) {this->toggleRenderParameter(checked, TEXT_SHOW_DISCIPLINE_BIT); });
	connect(m_ui.phaseBox, &QCheckBox::clicked, this, [this](bool checked) {this->toggleRenderParameter(checked, TEXT_SHOW_PHASE_BIT); });
	connect(m_ui.diameterBox, &QCheckBox::clicked, this, [this](bool checked) {this->toggleRenderParameter(checked, TEXT_SHOW_DIAMETER_BIT); });
	connect(m_ui.lengthBox, &QCheckBox::clicked, this, [this](bool checked) {this->toggleRenderParameter(checked, TEXT_SHOW_LENGTH_BIT); });
	connect(m_ui.coordinatesBox, &QCheckBox::clicked, this, [this](bool checked) {this->toggleRenderParameter(checked, TEXT_SHOW_COORD_BIT); });

	connect(m_ui.themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarTextDisplay::changeTheme);

	connect(m_ui.fontSizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolBarTextDisplay::changeTextFontSize);

	m_ui.indexBox->setChecked(true);
	m_ui.identifierBox->setChecked(true);
	m_ui.nameBox->setChecked(true);
}

ToolBarTextDisplay::~ToolBarTextDisplay()
{
	m_dataDispatcher.unregisterObserver(this);
}

void ToolBarTextDisplay::informData(IGuiData * data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		TextDisplayMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarTextDisplay::onProjectLoad(IGuiData * data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
	m_ui.indexBox->setEnabled(true);
	m_ui.authorBox->setEnabled(true);
	m_ui.identifierBox->setEnabled(true);
	m_ui.nameBox->setEnabled(true);
	m_ui.disciplineBox->setEnabled(true);
	m_ui.phaseBox->setEnabled(true);
	m_ui.diameterBox->setEnabled(true);
	m_ui.lengthBox->setEnabled(true);
	m_ui.coordinatesBox->setEnabled(true);

	m_ui.themeComboBox->setEnabled(true);
}

void ToolBarTextDisplay::onActiveCamera(IGuiData* data)
{
	auto infos = static_cast<GuiDataCameraInfo*>(data);
	if (infos->m_camera && m_focusCamera != infos->m_camera)
		return;

	updateUiFromCamera();
}

void ToolBarTextDisplay::onFocusViewport(IGuiData* data)
{
	GuiDataFocusViewport* castData = static_cast<GuiDataFocusViewport*>(data);
	if (castData->m_forceFocus && castData->m_camera)
		m_focusCamera = castData->m_camera;
}

void ToolBarTextDisplay::updateUiFromCamera()
{
	ReadPtr<CameraNode> rCam = m_focusCamera.cget();
	if (!rCam)
		return;

	const DisplayParameters& displayParameters = rCam->getDisplayParameters();
	blockAllSignals(true);

	m_textFilter = displayParameters.m_textOptions.m_filter;
	m_ui.indexBox->setChecked(m_textFilter & TEXT_SHOW_INDEX_BIT);
	m_ui.authorBox->setChecked(m_textFilter & TEXT_SHOW_AUTHOR_BIT);
	m_ui.identifierBox->setChecked(m_textFilter & TEXT_SHOW_IDENTIFIER_BIT);
	m_ui.nameBox->setChecked(m_textFilter & TEXT_SHOW_NAME_BIT);
	m_ui.disciplineBox->setChecked(m_textFilter & TEXT_SHOW_DISCIPLINE_BIT);
	m_ui.phaseBox->setChecked(m_textFilter & TEXT_SHOW_PHASE_BIT);
	m_ui.diameterBox->setChecked(m_textFilter & TEXT_SHOW_DIAMETER_BIT);
	m_ui.lengthBox->setChecked(m_textFilter & TEXT_SHOW_LENGTH_BIT);
	m_ui.coordinatesBox->setChecked(m_textFilter & TEXT_SHOW_COORD_BIT);

	m_ui.themeComboBox->setCurrentIndex(displayParameters.m_textOptions.m_textTheme);
	m_ui.fontSizeSpinBox->setValue(displayParameters.m_textOptions.m_textFontSize);

	blockAllSignals(false);
}

void ToolBarTextDisplay::blockAllSignals(bool block)
{
	m_ui.indexBox->blockSignals(block);
	m_ui.authorBox->blockSignals(block);
	m_ui.identifierBox->blockSignals(block);
	m_ui.nameBox->blockSignals(block);
	m_ui.disciplineBox->blockSignals(block);
	m_ui.phaseBox->blockSignals(block);
	m_ui.diameterBox->blockSignals(block);
	m_ui.lengthBox->blockSignals(block);
	m_ui.coordinatesBox->blockSignals(block);
	m_ui.themeComboBox->blockSignals(block);
	m_ui.fontSizeSpinBox->blockSignals(block);
}


inline void ToolBarTextDisplay::addRemoveFilter(TextFilter& textFilter, bool checked, int filter)
{
    if (checked)
        textFilter |= filter;
    else
        textFilter &= ~filter;
	//textFilter += (checked) ? filter : -filter;
	assert(((textFilter & filter) /*== filter (inutile)*/ && checked) || (!(textFilter & filter) && !checked)); //On regarde si le bouton est bien coché ou non selon le filtre appliqué
}

void ToolBarTextDisplay::toggleRenderParameter(bool checked, int parameter) {
	addRemoveFilter(m_textFilter, checked, parameter);
	m_dataDispatcher.updateInformation(new GuiDataRenderTextFilter(m_textFilter, m_focusCamera));
}

void ToolBarTextDisplay::changeTheme(int theme)
{
	//0 -> Dark | 1 -> Light
	m_dataDispatcher.updateInformation(new GuiDataRenderTextTheme(theme, m_focusCamera));
}

void ToolBarTextDisplay::changeTextFontSize(double font)
{
	m_dataDispatcher.updateInformation(new GuiDataRenderTextFontSize((float)font, m_focusCamera));
}
