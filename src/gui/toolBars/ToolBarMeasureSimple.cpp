#include "gui/toolBars/ToolBarMeasureSimple.h"
#include "controller/controls/ControlMeasure.h"
#include "controller/controls/ControlFunction.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataUserOrientation.h"
#include <QtWidgets/qpushbutton.h>

#include "models/data/PolylineMeasure/PolyLineTypes.h"

ToolBarMeasureSimple::ToolBarMeasureSimple(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);

	setEnabled(false);
	m_ui.simpleButton->setIconSize(QSize(20, 20) * guiScale);
	m_ui.polylineButton->setIconSize(QSize(20, 20) * guiScale);

	//m_ui.threePointsPlaneCheckBox->setChecked(false);
	connect(m_ui.simpleButton,&QPushButton::released, this, &ToolBarMeasureSimple::initSimpleMeasure);
	connect(m_ui.polylineButton, &QPushButton::released, this, &ToolBarMeasureSimple::initPolylineMeasure);

	connect(m_ui.applyPolylineConstraintCheckBox, &QCheckBox::released, this, &ToolBarMeasureSimple::activeOptions);
	connect(m_ui.lockZRadioButton, &QRadioButton::released, this, &ToolBarMeasureSimple::activeOptions);
	connect(m_ui.XZplaneRadioButton, &QRadioButton::released, this, &ToolBarMeasureSimple::activeOptions);
	connect(m_ui.YZplaneRadioButton, &QRadioButton::released, this, &ToolBarMeasureSimple::activeOptions);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::userOrientation);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectOrientation);
	m_methods.insert({ guiDType::projectLoaded, &ToolBarMeasureSimple::onProjectLoad });
	m_methods.insert({ guiDType::activatedFunctions, &ToolBarMeasureSimple::onActivateFunction });
	m_methods.insert({ guiDType::userOrientation, &ToolBarMeasureSimple::onUserOrientation});
	m_methods.insert({ guiDType::projectOrientation, &ToolBarMeasureSimple::onProjectOrientation });

	adjustSize();
	activeOptions();
}

void ToolBarMeasureSimple::informData(IGuiData* data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		MeasureSimpleMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarMeasureSimple::setPolyligneOptions(bool visible)
{
	m_ui.polyOptionsFrame->setVisible(visible);
	adjustSize();
}

void ToolBarMeasureSimple::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
}

void ToolBarMeasureSimple::onActivateFunction(IGuiData *data)
{
	auto* function = static_cast<GuiDataActivatedFunctions*>(data);

	m_ui.simpleButton->blockSignals(true);
	m_ui.polylineButton->blockSignals(true);


	m_ui.simpleButton->setChecked(false);
	m_ui.polylineButton->setChecked(false);

	switch (function->type)
	{
	case ContextType::simpleMeasure:
		m_ui.simpleButton->setChecked(true);
		break;

	case ContextType::pointsMeasure:
		m_ui.polylineButton->setChecked(true);
		break;
	default:
		break;
	}

	m_ui.simpleButton->blockSignals(false);
	m_ui.polylineButton->blockSignals(false);
}

void ToolBarMeasureSimple::onUserOrientation(IGuiData* data)
{
	auto gui = static_cast<GuiDataSetUserOrientation*>(data);
	m_userOrientationAngle = gui->m_userOrientation.getAngle();
	activeOptions();

}

void ToolBarMeasureSimple::onProjectOrientation(IGuiData* data)
{
	m_userOrientationAngle = 0.0;
	activeOptions();
}

ToolBarMeasureSimple::~ToolBarMeasureSimple()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarMeasureSimple::initSimpleMeasure()
{
    if (m_ui.simpleButton->isChecked())
        m_dataDispatcher.sendControl(new control::measure::ActivateSimpleMeasure());
    else
        m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarMeasureSimple::initPolylineMeasure()
{
    if (m_ui.polylineButton->isChecked())
        m_dataDispatcher.sendControl(new control::measure::ActivatePolylineMeasure());
    else
        m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarMeasureSimple::activeOptions()
{
	bool applyOptions = m_ui.applyPolylineConstraintCheckBox->isChecked();
	m_ui.lockZRadioButton->setEnabled(applyOptions);
	m_ui.XZplaneRadioButton->setEnabled(applyOptions);
	m_ui.YZplaneRadioButton->setEnabled(applyOptions);

	PolyLineOptions options;
	options.activeOption = applyOptions;
	options.decalOrientation = m_userOrientationAngle;

	if (m_ui.lockZRadioButton->isChecked())
		options.currentLock = PolyLineLock::LockZ;
	else if (m_ui.XZplaneRadioButton->isChecked())
		options.currentLock = PolyLineLock::LockY;
	else if (m_ui.YZplaneRadioButton->isChecked())
		options.currentLock = PolyLineLock::LockX;

	m_dataDispatcher.sendControl(new control::measure::SendMeasuresOptions(options));
	
}
