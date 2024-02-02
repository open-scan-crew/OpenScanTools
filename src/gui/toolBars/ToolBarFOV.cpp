#include "gui/toolBars/ToolBarFOV.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include <QDoubleSpinBox>
#include "utils/math/trigo.h"

#include "models/3d/Graph/CameraNode.h"

ToolBarFOV::ToolBarFOV(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale) 
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);

	connect(m_ui.FOVYSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolBarFOV::slotFov);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::renderIsPanoramic);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::renderActiveCamera);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::focusViewport);

	m_methods.insert({ guiDType::projectLoaded, &ToolBarFOV::onProjectLoad });
	m_methods.insert({ guiDType::renderIsPanoramic, &ToolBarFOV::onNaviPanoramic });
	m_methods.insert({ guiDType::renderActiveCamera, &ToolBarFOV::onActiveCamera });
	m_methods.insert({ guiDType::focusViewport, &ToolBarFOV::onFocusViewport });

}

ToolBarFOV::~ToolBarFOV()
{
	m_dataDispatcher.unregisterObserver(this);
}

void ToolBarFOV::informData(IGuiData *data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		fovToolBarMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarFOV::onNaviPanoramic(IGuiData * data)
{
	auto* panoramic = static_cast<GuiDataRenderIsPanoramic*>(data);
	m_ui.FOVYSpinBox->blockSignals(true);
	m_ui.FOVYSpinBox->setEnabled(!panoramic->m_pano);
	m_ui.FOVYSpinBox->blockSignals(false);
}

void ToolBarFOV::onProjectLoad(IGuiData* data)
{
	auto* function = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(function->m_isProjectLoad);
}

void ToolBarFOV::onFocusViewport(IGuiData* data)
{
	GuiDataFocusViewport* castData = static_cast<GuiDataFocusViewport*>(data);
	if (castData->m_forceFocus && castData->m_camera)
		m_focusCamera = castData->m_camera;
}

void ToolBarFOV::onActiveCamera(IGuiData* data)
{
	auto infos = static_cast<GuiDataCameraInfo*>(data);
	if (infos->m_camera && m_focusCamera != infos->m_camera)
		return;

	ReadPtr<CameraNode> rCam = m_focusCamera.cget();
	if (!rCam)
		return;

	double fov(rCam->getFovy() * (180.0 / M_PI));
	m_ui.FOVYSpinBox->blockSignals(true);
	m_ui.FOVYSpinBox->setValue(fov);
	m_ui.FOVYSpinBox->blockSignals(false);
}

void ToolBarFOV::slotFov(double fov) 
{
	m_dataDispatcher.updateInformation(new GuiDataRenderFov(fov, m_focusCamera));
}