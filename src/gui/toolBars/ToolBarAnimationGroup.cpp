#include "gui/toolBars/ToolBarAnimationGroup.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "controller/controls/ControlIO.h"
#include "controller/controls/ControlAnimation.h"
#include "io/ImageTypes.h"

ToolBarAnimationGroup::ToolBarAnimationGroup(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
	, m_isStarted(false)
{
	m_ui.setupUi(this);
	setEnabled(false);

	connect(m_ui.cleanListButton, &QPushButton::pressed, this, &ToolBarAnimationGroup::slotCleanAnimationList);
	connect(m_ui.startAnimationButton, &QPushButton::pressed, this, &ToolBarAnimationGroup::slotStartAnimation); 
	connect(m_ui.stopAnimationButton, &QPushButton::pressed, this, &ToolBarAnimationGroup::slotStopAnimation); 
	connect(m_ui.loopCheckBox, &QCheckBox::clicked, this, &ToolBarAnimationGroup::slotLoopAnimation);
	connect(m_ui.animationSpeedSlider, &QSlider::valueChanged, this, &ToolBarAnimationGroup::slotSpeedChange);
	connect(m_ui.recordPerfCheckBox, &QCheckBox::clicked, this, &ToolBarAnimationGroup::slotRecordPerformance);
	connect(m_ui.scansAnimPushButton, &QPushButton::clicked, this, &ToolBarAnimationGroup::slotScansAnimation);

	m_ui.formatComboBox->clear();
	for (const auto& iterator : ImageFormatDictio)
		m_ui.formatComboBox->addItem(QString::fromStdString(iterator.second));
	m_ui.formatComboBox->setCurrentIndex(1);

	updateUI();

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_methods.insert({ guiDType::projectLoaded, &ToolBarAnimationGroup::onProjectLoad });
}

ToolBarAnimationGroup::~ToolBarAnimationGroup()
{
	m_dataDispatcher.unregisterObserver(this);
}

void ToolBarAnimationGroup::informData(IGuiData* data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		AnimGroupMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarAnimationGroup::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
}

void ToolBarAnimationGroup::updateUI()
{
	m_ui.cleanListButton->setDisabled(m_isStarted);
	m_ui.startAnimationButton->setDisabled(m_isStarted);
	m_ui.cleanListButton->setDisabled(m_isStarted);
	m_ui.formatComboBox->setDisabled(m_isStarted);
	m_ui.stopAnimationButton->setEnabled(m_isStarted);
}

void ToolBarAnimationGroup::slotStartAnimation()
{
	m_dataDispatcher.updateInformation(new GuiDataRenderStartAnimation());
	m_isStarted = true;
	updateUI();
	if (m_ui.recordPerfCheckBox->isChecked())
		m_dataDispatcher.sendControl(new control::io::RecordPerformance());
	else
		m_dataDispatcher.updateInformation(new GuiDataRenderRecordPerformances(""));
}

void ToolBarAnimationGroup::slotStopAnimation()
{
	m_dataDispatcher.updateInformation(new GuiDataRenderStopAnimation());
	m_isStarted = false;
	updateUI();
}

void ToolBarAnimationGroup::slotCleanAnimationList()
{
	m_dataDispatcher.updateInformation(new GuiDataRenderCleanAnimationList());
}

void ToolBarAnimationGroup::slotLoopAnimation()
{
	m_dataDispatcher.updateInformation(new GuiDataRenderAnimationLoop(m_ui.loopCheckBox->isChecked()));
}

void ToolBarAnimationGroup::slotSpeedChange()
{
	m_dataDispatcher.updateInformation(new GuiDataRenderAnimationSpeed(m_ui.animationSpeedSlider->value()));
}

void ToolBarAnimationGroup::slotRecordPerformance()
{
	if(m_ui.recordPerfCheckBox->isChecked())
		m_dataDispatcher.updateInformation(new GuiDataRenderRecordPerformances(""));
}

void ToolBarAnimationGroup::slotScansAnimation()
{
	m_dataDispatcher.sendControl(new control::animation::AddScansViewPoint());
}