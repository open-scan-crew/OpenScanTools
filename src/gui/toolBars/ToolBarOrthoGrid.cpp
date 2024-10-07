#include "gui/toolBars/ToolBarOrthoGrid.h"

#include "controller/controls/ControlApplication.h"

#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"

#include "models/graph/CameraNode.h"

#include <qcolordialog.h>

#include "gui/Texts.hpp"

ToolBarOrthoGrid::ToolBarOrthoGrid(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
	, m_selectedColor(128, 128, 128)
{
	m_ui.setupUi(this);
	setEnabled(false);

	m_ui.colorPicker->setPalette(QPalette(m_selectedColor));
	m_ui.gridStepLineEdit->setType(NumericType::DISTANCE);
	m_ui.gridStepLineEdit->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);

	connect(m_ui.colorPicker, &QPushButton::clicked, this, &ToolBarOrthoGrid::slotColorPicking);
	connect(m_ui.displayGridCheckBox, &QPushButton::clicked, this, &ToolBarOrthoGrid::updateGrid);
	connect(m_ui.gridStepLineEdit, &QLineEdit::editingFinished, this, &ToolBarOrthoGrid::updateGrid);
	connect(m_ui.lineWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ToolBarOrthoGrid::updateGrid);

	registerGuiDataFunction(guiDType::renderActiveCamera, &ToolBarOrthoGrid::onActiveCamera);
	registerGuiDataFunction(guiDType::focusViewport, &ToolBarOrthoGrid::onFocusViewport);
	registerGuiDataFunction(guiDType::projectLoaded, &ToolBarOrthoGrid::onProjectLoad);
}

ToolBarOrthoGrid::~ToolBarOrthoGrid()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarOrthoGrid::informData(IGuiData *data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		GuiDataFunction method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarOrthoGrid::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
}

void ToolBarOrthoGrid::blockAllSignals(bool block)
{
	m_ui.displayGridCheckBox->blockSignals(block);
	m_ui.colorPicker->blockSignals(block);
	m_ui.gridStepLineEdit->blockSignals(block);
	m_ui.lineWidthSpinBox->blockSignals(block);
}

void ToolBarOrthoGrid::onActiveCamera(IGuiData* data)
{
	auto infos = static_cast<GuiDataCameraInfo*>(data);

	ReadPtr<CameraNode> rCam = m_focusCamera.cget();
	if (!rCam)
		return;
	const DisplayParameters& displayParameters = rCam->getDisplayParameters();

	blockAllSignals(true);

	m_ui.displayGridCheckBox->setChecked(displayParameters.m_orthoGridActive);
	m_selectedColor.setRgb(displayParameters.m_orthoGridColor.r, displayParameters.m_orthoGridColor.g, displayParameters.m_orthoGridColor.b);
	m_ui.colorPicker->setPalette(QPalette(m_selectedColor));
	m_ui.gridStepLineEdit->setValue(displayParameters.m_orthoGridStep);
	m_ui.lineWidthSpinBox->setValue(displayParameters.m_orthoGridLineWidth);

	blockAllSignals(false);
}

void ToolBarOrthoGrid::onFocusViewport(IGuiData* data)
{
	GuiDataFocusViewport* castData = static_cast<GuiDataFocusViewport*>(data);
	if (castData->m_forceFocus && castData->m_camera)
		m_focusCamera = castData->m_camera;
}

void ToolBarOrthoGrid::slotColorPicking()
{
	QColor newColor = QColorDialog::getColor(m_selectedColor, this);
	if (newColor.isValid())
	{
		m_selectedColor = newColor;
		m_ui.colorPicker->setPalette(QPalette(m_selectedColor));
		updateGrid();
	}
}

void ToolBarOrthoGrid::updateGrid()
{
	Color32 color(m_selectedColor.red(), m_selectedColor.green(), m_selectedColor.blue());
	m_dataDispatcher.sendControl(new control::application::SetOrthoGridParameters(m_ui.displayGridCheckBox->isChecked(), color, 
																					m_ui.gridStepLineEdit->getValue(), m_ui.lineWidthSpinBox->value()));
}

