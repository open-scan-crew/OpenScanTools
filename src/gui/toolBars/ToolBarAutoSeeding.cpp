#include "gui/toolBars/ToolBarAutoSeeding.h"
#include "controller/controls/ControlMeasure.h"
#include "gui/GuiData/GuiDataGeneralProject.h"

ToolBarAutoSeeding::ToolBarAutoSeeding(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	setEnabled(false);
	m_ui.setupUi(this);

	QObject::connect(m_ui.pushButton1top, &QPushButton::clicked, this, &ToolBarAutoSeeding::init1FromTop);
	QObject::connect(m_ui.pushButton1bottom, &QPushButton::clicked, this, &ToolBarAutoSeeding::init1FromBot);
	QObject::connect(m_ui.pushButton2top, &QPushButton::clicked, this, &ToolBarAutoSeeding::init2FromTop);
	QObject::connect(m_ui.pushButton2bottom, &QPushButton::clicked, this, &ToolBarAutoSeeding::init2FromBot);
	QObject::connect(m_ui.pushButton3plane, &QPushButton::clicked, this, &ToolBarAutoSeeding::init3Plane);
	QObject::connect(m_ui.pushButton4horizontal, &QPushButton::clicked, this, &ToolBarAutoSeeding::init4Horiz);
	QObject::connect(m_ui.pushButton5slope, &QPushButton::clicked, this, &ToolBarAutoSeeding::init5Slope);
	QObject::connect(m_ui.pushButton6vertical, &QPushButton::clicked, this, &ToolBarAutoSeeding::init6Vert);
	QObject::connect(m_ui.pushButton7slope, &QPushButton::clicked, this, &ToolBarAutoSeeding::init7Slope);
	QObject::connect(m_ui.pushButton8horizontal, &QPushButton::clicked, this, &ToolBarAutoSeeding::init8Horiz);
	QObject::connect(m_ui.pushButton8plane, &QPushButton::clicked, this, &ToolBarAutoSeeding::init8Plane);

	m_ui.spacingLineEdit->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);
	m_ui.thresholdLineEdit->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);

	m_ui.spacingLineEdit->setType(NumericType::DISTANCE);
	m_ui.thresholdLineEdit->setType(NumericType::DISTANCE);

	m_ui.spacingLineEdit->setValue(0.05);
	m_ui.thresholdLineEdit->setValue(0.1);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
}

void ToolBarAutoSeeding::informData(IGuiData *data)
{
	if (data->getType() == guiDType::projectLoaded)
	{
		GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
		setEnabled(plData->m_isProjectLoad);
	}
}

ToolBarAutoSeeding::~ToolBarAutoSeeding()
{
	m_dataDispatcher.unregisterObserver(this);
}


void ToolBarAutoSeeding::init1FromTop()
{
	launchAutoSeeding(SetOfPointsOptions::case1, false, true, true, true);
}

void ToolBarAutoSeeding::init1FromBot()
{
	launchAutoSeeding(SetOfPointsOptions::case1, false, true, false, true);
}

void ToolBarAutoSeeding::init2FromTop()
{
	launchAutoSeeding(SetOfPointsOptions::case2, false, true, true, true);
}

void ToolBarAutoSeeding::init2FromBot()
{
	launchAutoSeeding(SetOfPointsOptions::case2, false, true, false, true);
}

void ToolBarAutoSeeding::init3Plane()
{
	launchAutoSeeding(SetOfPointsOptions::case3, false, true, true, true);
}

void ToolBarAutoSeeding::init4Horiz()
{
	launchAutoSeeding(SetOfPointsOptions::case4, false, true, true, true);
}

void ToolBarAutoSeeding::init5Slope()
{
	launchAutoSeeding(SetOfPointsOptions::case5, false, true, true, true);
}

void ToolBarAutoSeeding::init6Vert()
{
	launchAutoSeeding(SetOfPointsOptions::case6, false, true, true, true);
}

void ToolBarAutoSeeding::init7Slope()
{
	launchAutoSeeding(SetOfPointsOptions::case7, false, true, true, true);
}

void ToolBarAutoSeeding::init8Horiz()
{
	launchAutoSeeding(SetOfPointsOptions::case8, false, true, true, true);
}

void ToolBarAutoSeeding::init8Plane()
{
	launchAutoSeeding(SetOfPointsOptions::case8, false, true, true, false);
}

void ToolBarAutoSeeding::launchAutoSeeding(SetOfPointsOptions options, bool userAxes, bool createMeasures, bool fromTop, bool horizontal)
{
	m_dataDispatcher.sendControl(new control::measure::ActivateSetOfPoints(m_ui.spacingLineEdit->getValue(), m_ui.thresholdLineEdit->getValue(), options, userAxes, createMeasures, fromTop, horizontal));
}
