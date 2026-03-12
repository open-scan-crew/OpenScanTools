#include "gui/toolBars/ToolBarAnimationGroup.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "controller/controls/ControlAnimation.h"
#include "gui/Dialog/DialogAnimationConfig.h"

ToolBarAnimationGroup::ToolBarAnimationGroup(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
	, m_dialog(new DialogExportVideo(dataDispatcher, this, guiScale))
	, m_animationConfigDialog(new DialogAnimationConfig(dataDispatcher, this))
	, m_isStarted(false)
	, m_isProjectLoaded(false)
	, m_canStartAnimation(false)
{
	m_ui.setupUi(this);
	setEnabled(false);
	m_animationModeButtons.setExclusive(true);
	m_animationModeButtons.addButton(m_ui.orbital360RadioButton);
	m_animationModeButtons.addButton(m_ui.betweenViewpointsRadioButton);

	connect(m_ui.startAnimationButton, &QPushButton::pressed, this, &ToolBarAnimationGroup::slotStartAnimation); 
	connect(m_ui.stopAnimationButton, &QPushButton::pressed, this, &ToolBarAnimationGroup::slotStopAnimation); 
	connect(m_ui.generateVideoPushButton, &QPushButton::clicked, this, &ToolBarAnimationGroup::slotGenerateVideo);
	connect(m_ui.betweenViewpointsRadioButton, &QRadioButton::clicked, this, &ToolBarAnimationGroup::slotAnimationModeChanged);
	connect(m_ui.orbital360RadioButton, &QRadioButton::clicked, this, &ToolBarAnimationGroup::slotAnimationModeChanged);
	connect(m_ui.toolButton_newViewpointAnimConfig, &QToolButton::clicked, this, &ToolBarAnimationGroup::slotNewViewPointAnimationConfig);
	connect(m_ui.toolButton_editViewpointAnimConfig, &QToolButton::clicked, this, &ToolBarAnimationGroup::slotEditViewPointAnimationConfig);
	connect(m_ui.comboBox_animationList, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarAnimationGroup::slotAnimationConfigChanged);
	m_ui.degreesLabel->setEnabled(false);
	slotAnimationModeChanged();

	updateUI();

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::actualizeNodes);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::renderAnimationToolbarState);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::renderStopAnimation);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendViewPointAnimationData);
	m_methods.insert({ guiDType::projectLoaded, &ToolBarAnimationGroup::onProjectLoad });
	m_methods.insert({ guiDType::actualizeNodes, &ToolBarAnimationGroup::onProjectTreeActualize });
	m_methods.insert({ guiDType::renderAnimationToolbarState, &ToolBarAnimationGroup::onAnimationToolbarState });
	m_methods.insert({ guiDType::renderStopAnimation, &ToolBarAnimationGroup::onRenderStopAnimation });
	m_methods.insert({ guiDType::sendViewPointAnimationData, &ToolBarAnimationGroup::onAnimationData });
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
		m_animationConfigs.clear();
		m_availableViewpoints.clear();
		m_ui.comboBox_animationList->clear();
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

	const bool hasSelection = m_ui.comboBox_animationList->currentIndex() >= 0;
	const bool viewpointsMode = m_ui.betweenViewpointsRadioButton->isChecked();
	m_ui.comboBox_animationList->setEnabled(m_isProjectLoaded && viewpointsMode);
	m_ui.toolButton_newViewpointAnimConfig->setEnabled(m_isProjectLoaded && viewpointsMode);
	m_ui.toolButton_editViewpointAnimConfig->setEnabled(m_isProjectLoaded && viewpointsMode && hasSelection);
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

	m_dialog->setAnimationMode(m_ui.betweenViewpointsRadioButton->isChecked() ? VideoAnimationMode::BETWEENVIEWPOINTS : VideoAnimationMode::ORBITAL);
	m_dialog->setLength(m_ui.lengthSpinBox->value());
	m_dialog->setInterpolateRenderings(m_ui.interpolateCheckBox->isChecked());
	m_dialog->show();
}

void ToolBarAnimationGroup::slotAnimationModeChanged()
{
	m_ui.interpolateCheckBox->setEnabled(m_ui.betweenViewpointsRadioButton->isChecked());
	updateUI();
}

void ToolBarAnimationGroup::refreshAnimationAvailability()
{
	m_dataDispatcher.sendControl(new control::animation::RefreshViewpointsAnimationState());
	m_dataDispatcher.sendControl(new control::animation::SendViewPointAnimationData());
}

void ToolBarAnimationGroup::slotNewViewPointAnimationConfig()
{
	m_animationConfigDialog->setKnownAnimations(m_animationConfigs);
	m_animationConfigDialog->setAvailableViewpoints(m_availableViewpoints);
	m_animationConfigDialog->setupForNew();
	m_animationConfigDialog->exec();
}

void ToolBarAnimationGroup::slotEditViewPointAnimationConfig()
{
	const int index = m_ui.comboBox_animationList->currentIndex();
	if (index < 0)
		return;

	const xg::Guid selectedId(m_ui.comboBox_animationList->itemData(index).toString().toStdString());
	for (const ViewPointAnimationConfig& cfg : m_animationConfigs)
	{
		if (cfg.getId() == selectedId)
		{
			m_animationConfigDialog->setKnownAnimations(m_animationConfigs);
			m_animationConfigDialog->setAvailableViewpoints(m_availableViewpoints);
			m_animationConfigDialog->setupForEdit(cfg);
			m_animationConfigDialog->exec();
			break;
		}
	}
}

void ToolBarAnimationGroup::slotAnimationConfigChanged(int index)
{
	(void)index;
	updateUI();
}

void ToolBarAnimationGroup::onAnimationData(IGuiData* keyValue)
{
	auto animationData = static_cast<GuiDataSendViewPointAnimationData*>(keyValue);
	m_animationConfigs = animationData->m_animations;
	m_availableViewpoints = animationData->m_viewpoints;

	QString selectedId;
	if (m_ui.comboBox_animationList->currentIndex() >= 0)
		selectedId = m_ui.comboBox_animationList->currentData().toString();

	m_ui.comboBox_animationList->blockSignals(true);
	m_ui.comboBox_animationList->clear();
	int selectionIndex = -1;
	for (const ViewPointAnimationConfig& config : m_animationConfigs)
	{
		const QString id = QString::fromStdString(config.getId().str());
		m_ui.comboBox_animationList->addItem(config.getName(), id);
		if (!selectedId.isEmpty() && selectedId == id)
			selectionIndex = m_ui.comboBox_animationList->count() - 1;
	}
	if (selectionIndex >= 0)
		m_ui.comboBox_animationList->setCurrentIndex(selectionIndex);
	m_ui.comboBox_animationList->blockSignals(false);

	updateUI();
}
