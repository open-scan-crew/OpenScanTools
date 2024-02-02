#include "gui/toolBars/ToolBarStructureAnalysis.h"
#include "controller/controls/ControlMeasure.h"
#include "controller/controls/ControlFunction.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qlineedit.h>

ToolBarStructureAnalysis::ToolBarStructureAnalysis(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);
	m_ui.beamBendingButton->setIconSize(QSize(20, 20) * guiScale);
	m_ui.columnTiltButton->setIconSize(QSize(20, 20) * guiScale);

	QObject::connect(m_ui.columnTiltButton, &QPushButton::released, this, &ToolBarStructureAnalysis::initColumnTiltMeasure);
	QObject::connect(m_ui.tiltLineEdit, &QLineEdit::editingFinished, this, &ToolBarStructureAnalysis::setTiltTolerance);
	QObject::connect(m_ui.beamBendingButton, &QPushButton::released, this, &ToolBarStructureAnalysis::initBeamBendingMeasure);
	QObject::connect(m_ui.bendingLineEdit, &QLineEdit::editingFinished, this, &ToolBarStructureAnalysis::setBendingTolerance);

	m_ui.tiltLineEdit->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);
	m_ui.bendingLineEdit->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::tolerancesProperties);

	m_methods.insert({ guiDType::activatedFunctions, &ToolBarStructureAnalysis::onFunctionActived });
	m_methods.insert({ guiDType::projectLoaded, &ToolBarStructureAnalysis::onProjectLoad });
	m_methods.insert({ guiDType::tolerancesProperties, &ToolBarStructureAnalysis::onTolerances });


}

void ToolBarStructureAnalysis::informData(IGuiData* data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		StructureAnalysisMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarStructureAnalysis::onProjectLoad(IGuiData* data)
{
	auto* proj = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(proj->m_isProjectLoad);
}

void ToolBarStructureAnalysis::onFunctionActived(IGuiData* data)
{
	auto* function = static_cast<GuiDataActivatedFunctions*>(data);

	m_ui.beamBendingButton->blockSignals(true);
	m_ui.columnTiltButton->blockSignals(true);
	switch (function->type)
	{
	case ContextType::beamBending:
		m_ui.beamBendingButton->setChecked(true);
		m_ui.columnTiltButton->setChecked(false);
		break;
	case ContextType::columnTilt:
		m_ui.beamBendingButton->setChecked(false);
		m_ui.columnTiltButton->setChecked(true);
		break;
	default:
		m_ui.beamBendingButton->setChecked(false);
		m_ui.columnTiltButton->setChecked(false);
	}
	m_ui.beamBendingButton->blockSignals(false);
	m_ui.columnTiltButton->blockSignals(false);
}

void ToolBarStructureAnalysis::onTolerances(IGuiData* data)
{
	auto* tol = static_cast<GuiDataTolerances*>(data);

	m_ui.tiltLineEdit->blockSignals(true);
	m_ui.bendingLineEdit->blockSignals(true);

	m_ui.tiltLineEdit->setText(QString::number(tol->m_tilt));
	m_ui.bendingLineEdit->setText(QString::number(tol->m_bending));

	m_ui.tiltLineEdit->blockSignals(false);
	m_ui.bendingLineEdit->blockSignals(false);
}

ToolBarStructureAnalysis::~ToolBarStructureAnalysis()
{
	m_dataDispatcher.unregisterObserver(this);
}

void ToolBarStructureAnalysis::initColumnTiltMeasure()
{
	if (m_ui.columnTiltButton->isChecked())
		m_dataDispatcher.sendControl(new control::measure::ActivateColumnTilt());
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarStructureAnalysis::initBeamBendingMeasure()
{
	if (m_ui.beamBendingButton->isChecked())
	{
		BeamBendingOptions options = m_ui.manualBeamBendingCheckBox->isChecked() ? BeamBendingOptions::manual : BeamBendingOptions::normal;
		m_dataDispatcher.sendControl(new control::measure::ActivateBeamBending(options));
	}
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarStructureAnalysis::setTiltTolerance()
{
	m_dataDispatcher.sendControl(new control::measure::SetTiltTolerance(m_ui.tiltLineEdit->getValue()));
}

void ToolBarStructureAnalysis::setBendingTolerance()
{
	m_dataDispatcher.sendControl(new control::measure::SetBendingTolerance(m_ui.bendingLineEdit->getValue()));
}