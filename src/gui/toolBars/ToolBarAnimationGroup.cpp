#include "gui/toolBars/ToolBarAnimationGroup.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "controller/controls/ControlAnimation.h"

ToolBarAnimationGroup::ToolBarAnimationGroup(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
	, m_dialog(new DialogExportVideo(dataDispatcher, this, guiScale))
	, m_isStarted(false)
	, m_isProjectLoaded(false)
	, m_canStartAnimation(false)
{
	m_ui.setupUi(this);
	m_ui.comboBox_animationList->setEnabled(false);
	m_ui.toolButton_editViewpointAnimConfig->setEnabled(false);
	m_ui.toolButton_newViewpointAnimConfig->setEnabled(false);
	m_ui.degreesSpinBox->setEnabled(false);
	m_ui.label_degrees->setEnabled(false);
	setEnabled(false);

	connect(m_ui.startAnimationButton, &QPushButton::pressed, this, &ToolBarAnimationGroup::slotStartAnimation); 
	connect(m_ui.stopAnimationButton, &QPushButton::pressed, this, &ToolBarAnimationGroup::slotStopAnimation); 
	connect(m_ui.generateVideoPushButton, &QPushButton::clicked, this, &ToolBarAnimationGroup::slotGenerateVideo);
	connect(m_ui.betweenViewpointsRadioButton, &QRadioButton::toggled, this, &ToolBarAnimationGroup::slotAnimationModeChanged);
	connect(m_ui.orbital360RadioButton, &QRadioButton::toggled, this, &ToolBarAnimationGroup::slotAnimationModeChanged);
	slotAnimationModeChanged();

	updateUI();

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::actualizeNodes);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::renderAnimationToolbarState);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::renderStopAnimation);
	m_methods.insert({ guiDType::projectLoaded, &ToolBarAnimationGroup::onProjectLoad });
	m_methods.insert({ guiDType::actualizeNodes, &ToolBarAnimationGroup::onProjectTreeActualize });
	m_methods.insert({ guiDType::renderAnimationToolbarState, &ToolBarAnimationGroup::onAnimationToolbarState });
	m_methods.insert({ guiDType::renderStopAnimation, &ToolBarAnimationGroup::onRenderStopAnimation });
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
	m_isProjectLoaded = plData->m_isProjectLoad;
	setEnabled(m_isProjectLoaded);
	if (m_isProjectLoaded)
		refreshAnimationAvailability();
	else
	{
		m_canStartAnimation = false;
		m_isStarted = false;
		updateUI();
	}
}

void ToolBarAnimationGroup::onProjectTreeActualize(IGuiData* data)
{
	(void)data;
	if (!m_isProjectLoaded)
		return;
	refreshAnimationAvailability();
}

void ToolBarAnimationGroup::onAnimationToolbarState(IGuiData* data)
{
	auto state = static_cast<GuiDataRenderAnimationToolbarState*>(data);
	m_canStartAnimation = state->m_canStart;
	if (!m_canStartAnimation)
		m_isStarted = false;
	updateUI();
}

void ToolBarAnimationGroup::onRenderStopAnimation(IGuiData* data)
{
	(void)data;
	m_isStarted = false;
	refreshAnimationAvailability();
	updateUI();
}

void ToolBarAnimationGroup::updateUI()
{
	const bool canStart = m_isProjectLoaded && m_canStartAnimation && !m_isStarted;
	m_ui.startAnimationButton->setEnabled(canStart);
	m_ui.stopAnimationButton->setEnabled(m_isProjectLoaded && m_isStarted);
}

void ToolBarAnimationGroup::slotStartAnimation()
{
	if (!m_isProjectLoaded || !m_canStartAnimation)
		return;

	m_dataDispatcher.sendControl(new control::animation::PrepareViewpointsAnimation());
	if (!m_canStartAnimation)
		return;

	m_dataDispatcher.updateInformation(new GuiDataRenderStartAnimation());
	m_isStarted = true;
	updateUI();
}

void ToolBarAnimationGroup::slotStopAnimation()
{
	if (!m_isStarted)
		return;

	m_dataDispatcher.updateInformation(new GuiDataRenderStopAnimation());
	m_isStarted = false;
	refreshAnimationAvailability();
	updateUI();
}

void ToolBarAnimationGroup::slotGenerateVideo()
{
	if (!m_isProjectLoaded)
		return;

	VideoAnimationMode animationMode = m_ui.betweenViewpointsRadioButton->isChecked()
		? VideoAnimationMode::BETWEENVIEWPOINTS
		: VideoAnimationMode::ORBITAL;
	m_dialog->setToolbarAnimationOptions(animationMode, m_ui.lengthSpinBox->value(), m_ui.interpolateCheckBox->isChecked());
	m_dialog->show();
}

void ToolBarAnimationGroup::slotAnimationModeChanged()
{
	bool useViewpoints = m_ui.betweenViewpointsRadioButton->isChecked();
	m_ui.interpolateCheckBox->setEnabled(useViewpoints);
}

void ToolBarAnimationGroup::refreshAnimationAvailability()
{
	m_dataDispatcher.sendControl(new control::animation::RefreshViewpointsAnimationState());
}
