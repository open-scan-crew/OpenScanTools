#include "gui/Dialog/DialogGizmo.h"
#include "controller/controls/ControlApplication.h"
#include "controller/controls/ControlProjectTemplate.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/DataDispatcher.h"
#include "gui/UnitUsage.h"
#include "models/project/ProjectTypes.h"
#include "utils/QtLogStream.hpp"

#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/qcolordialog.h>
#include <QtCore/QStandardPaths>

DialogGizmo::DialogGizmo(IDataDispatcher& dataDispatcher, QWidget* parent)
	: ADialog(dataDispatcher, parent)
	, m_parameters(-0.85, 0.9, 0.25)
	, m_oldParameters(-0.85, 0.9, 0.25)
{
	m_ui.setupUi(this);
	connect(m_ui.verticalSlider, &QSlider::valueChanged, this, &DialogGizmo::onVerticalSlider);
	connect(m_ui.horizontalSlider, &QSlider::valueChanged, this, &DialogGizmo::onHorizontalSlider);
	connect(m_ui.sizeDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DialogGizmo::onSizeDoubleSpin);

    m_dataDispatcher.registerObserverOnKey(this, guiDType::renderGizmoParameters);

    initConfigValues();
}

DialogGizmo::~DialogGizmo()
{
	m_dataDispatcher.unregisterObserver(this);
}

void DialogGizmo::informData(IGuiData* idata)
{
    switch (idata->getType())
    {
    case guiDType::renderGizmoParameters:
		m_oldParameters = static_cast<GuiDataGizmoParameters*>(idata)->m_paramters;
		initConfigValues();
        break;
    default:
        break;
    }
}

void DialogGizmo::initConfigValues()
{
	m_ui.verticalSlider->blockSignals(true);
	m_ui.horizontalSlider->blockSignals(true);
	m_ui.sizeDoubleSpinBox->blockSignals(true);
	
	m_ui.sizeDoubleSpinBox->setValue(m_oldParameters.z);
	m_ui.horizontalSlider->setValue(m_oldParameters.x*100);
	m_ui.verticalSlider->setValue(-m_oldParameters.y*100);

	m_ui.verticalSlider->blockSignals(false);
	m_ui.horizontalSlider->blockSignals(false);
	m_ui.sizeDoubleSpinBox->blockSignals(false);
}

void DialogGizmo::onSizeDoubleSpin(const double& value)
{
	if (m_parameters.z != value)
	{
		m_parameters.z = value;
		sendUpdate();
	}
}

void DialogGizmo::onHorizontalSlider(const int& value)
{
	double dValue(value / 100.0);
	if (m_parameters.z != dValue)
	{
		m_parameters.x = dValue;
		sendUpdate();
	}
}

void DialogGizmo::onVerticalSlider(const int& value)
{
	double dValue(-value / 100.0);
	if (m_parameters.z != dValue)
	{
		m_parameters.y = dValue;
		sendUpdate();
	}
}

void DialogGizmo::sendUpdate()
{
	m_dataDispatcher.updateInformation(new GuiDataGizmoParameters(m_parameters), this);
}

void DialogGizmo::accept()
{
	m_oldParameters = m_parameters;
	m_dataDispatcher.sendControl(new control::application::SetGizmoParameters(true, m_oldParameters, true));
	QDialog::accept();
}

void DialogGizmo::reject()
{
	m_dataDispatcher.updateInformation(new GuiDataGizmoParameters(m_oldParameters), this);	
	QDialog::reject();
}