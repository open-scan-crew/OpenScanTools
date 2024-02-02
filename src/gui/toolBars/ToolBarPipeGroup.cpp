#include "gui/toolBars/ToolBarPipeGroup.h"
#include "gui/Dialog/StandardListDialog.h"
#include "controller/controls/ControlMeasure.h"
#include "controller/controls/ControlFunction.h"
#include "controller/controls/ControlStandards.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMeasure.h"
#include "gui/GuiData/GuiDataList.h"
#include <QtWidgets/qpushbutton.h>

ToolBarPipeGroup::ToolBarPipeGroup(IDataDispatcher& dataDispatcher, QWidget* parent, float scale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);
	m_ui.fastRadioButton->setChecked(true);
	m_ui.noExtendCheckBox->setChecked(true);
	m_ui.pipeToolButton->setIconSize(QSize(20, 20) * scale);
	m_ui.robustToolButton->setIconSize(QSize(20, 20) * scale);
	m_ui.insulatedLineEdit->setEnabled(m_ui.insulatedCheckBox->isChecked());
	m_ui.insulatedLineEdit->setType(NumericType::DIAMETER);

	connect(m_ui.pipeToolButton, &QToolButton::released, this, &ToolBarPipeGroup::initPipeDetection);
	connect(m_ui.robustToolButton, &QToolButton::released, this, &ToolBarPipeGroup::initLargePipeDetection);
	connect(m_ui.managePushButton, &QToolButton::released, this, &ToolBarPipeGroup::initPipeStandardManagement);
	connect(m_ui.manualExtendCheckBox, &QCheckBox::stateChanged, [this](const int& state) {this->extensionStateChanged(state, m_ui.manualExtendCheckBox); });
	connect(m_ui.extendCheckBox, &QCheckBox::stateChanged, [this](const int& state) {this->extensionStateChanged(state, m_ui.extendCheckBox); });
	connect(m_ui.noExtendCheckBox, &QCheckBox::stateChanged, [this](const int& state) {this->extensionStateChanged(state, m_ui.noExtendCheckBox); });

	connect(m_ui.fastRadioButton, &QRadioButton::released, this, &ToolBarPipeGroup::updateDetectionOptions);
	connect(m_ui.optimizedRadioButton, &QRadioButton::released, this, &ToolBarPipeGroup::updateDetectionOptions);
	connect(m_ui.noisyCheckBox, &QCheckBox::stateChanged, this, &ToolBarPipeGroup::updateDetectionOptions);
	connect(m_ui.insulatedLineEdit, &QLineEdit::editingFinished, this, &ToolBarPipeGroup::updateDetectionOptions);

	connect(m_ui.insulatedCheckBox, &QCheckBox::stateChanged, this, [this]() 
		{ 
			m_ui.insulatedLineEdit->setEnabled(m_ui.insulatedCheckBox->isChecked()); 
			updateDetectionOptions();
		});

	connect(m_ui.standardsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarPipeGroup::slotStandardChanged);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendListsStandards);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);

	m_methods.insert({ guiDType::activatedFunctions, &ToolBarPipeGroup::onActivateFunction });
	m_methods.insert({ guiDType::sendListsStandards, &ToolBarPipeGroup::onStandardRecieved });
	m_methods.insert({ guiDType::projectLoaded, &ToolBarPipeGroup::onProjectLoad });

}

ToolBarPipeGroup::~ToolBarPipeGroup()
{
	m_dataDispatcher.unregisterObserver(this);
}

void ToolBarPipeGroup::informData(IGuiData* data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		PipeGroupMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarPipeGroup::onProjectLoad(IGuiData* data)
{
	auto* function = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(function->m_isProjectLoad);
}

void ToolBarPipeGroup::onStandardRecieved(IGuiData* data)
{
	auto* standards = static_cast<GuiDataSendListsStandards*>(data);
	if (standards->m_type != StandardType::Pipe)
		return;
	m_ui.standardsComboBox->blockSignals(true);
	int selectedItem(m_ui.standardsComboBox->currentIndex());
	selectedItem = selectedItem == -1 ? 0 : selectedItem;
	m_ui.standardsComboBox->clear();
	m_standardsElems.clear();
	
	//Chercher le standard par défaut (NA) à ajouter en premier
	for (const SafePtr<StandardList>& stand : standards->m_lists) {
		ReadPtr<StandardList> rStand = stand.cget();
		if (rStand && rStand->getOrigin()) {
			m_standardsElems.push_back(stand);
			m_ui.standardsComboBox->addItem(QString::fromStdWString(rStand->getName()));
			break;
		}
	}

	//Ajouter le reste des standards 
	for (const SafePtr<StandardList>& stand : standards->m_lists)
	{
		ReadPtr<StandardList> rStand = stand.cget();
		if (!rStand || rStand->getOrigin())
			continue;
		m_standardsElems.push_back(stand);
		m_ui.standardsComboBox->addItem(QString::fromStdWString(rStand->getName()));
	}

	m_ui.standardsComboBox->setCurrentIndex(selectedItem);
	m_ui.standardsComboBox->blockSignals(false);

	slotStandardChanged(selectedItem);
}

void ToolBarPipeGroup::slotStandardChanged(const int& index)
{
	if(m_standardsElems.size() > index)
		m_dataDispatcher.sendControl(new control::standards::SelectStandard(m_standardsElems[index], StandardType::Pipe));
}

void ToolBarPipeGroup::onActivateFunction(IGuiData* data)
{
	auto* function = static_cast<GuiDataActivatedFunctions*>(data);

	m_ui.pipeToolButton->blockSignals(true);
	m_ui.robustToolButton->blockSignals(true);
	switch (function->type) {
		case ContextType::bigCylinderFit:
		{
			m_ui.pipeToolButton->setChecked(false);
			m_ui.fastRadioButton->setEnabled(false);
			m_ui.optimizedRadioButton->setEnabled(false);
			m_ui.noisyCheckBox->setEnabled(false);
			m_ui.extendCheckBox->setEnabled(false);
			m_ui.noExtendCheckBox->setEnabled(false);
			m_ui.manualExtendCheckBox->setEnabled(false);
			m_ui.manualExtendCheckBox->setChecked(true);
		}
		break;
		case ContextType::fitCylinder:
		//case ContextType::pipeDetectionConnexion:
		{
			m_ui.robustToolButton->setChecked(false);
			m_ui.extendCheckBox->setEnabled(false);
			m_ui.noExtendCheckBox->setEnabled(false);
			m_ui.manualExtendCheckBox->setEnabled(false);
			m_ui.fastRadioButton->setEnabled(false);
			m_ui.optimizedRadioButton->setEnabled(false);
			m_ui.noisyCheckBox->setEnabled(false);
		}
		break;
		default:
		{
			m_ui.pipeToolButton->setChecked(false);
			m_ui.robustToolButton->setChecked(false);
			m_ui.noExtendCheckBox->setEnabled(true);
			m_ui.fastRadioButton->setEnabled(true);
			m_ui.extendCheckBox->setEnabled(true);
			m_ui.manualExtendCheckBox->setEnabled(true);
			m_ui.optimizedRadioButton->setEnabled(true);
			m_ui.noisyCheckBox->setEnabled(true);
		}
		break;
	}
	

	m_ui.pipeToolButton->blockSignals(false);
	m_ui.robustToolButton->blockSignals(false);
}

void ToolBarPipeGroup::extensionStateChanged(int state, QCheckBox* active)
{
	if (state != 2)
	{
		active->blockSignals(true);
		active->setChecked(true);
		active->blockSignals(false);
	}
	else
	{
		m_ui.manualExtendCheckBox->blockSignals(true);
		m_ui.extendCheckBox->blockSignals(true);
		m_ui.noExtendCheckBox->blockSignals(true);
		if (m_ui.manualExtendCheckBox == active)
		{ 
			m_ui.extendCheckBox->setChecked(false);
			m_ui.noExtendCheckBox->setChecked(false);
		}
		else if (m_ui.extendCheckBox == active)
		{
			m_ui.manualExtendCheckBox->setChecked(false);
			m_ui.noExtendCheckBox->setChecked(false);
		}
		else if (m_ui.noExtendCheckBox == active)
		{
			m_ui.manualExtendCheckBox->setChecked(false);
			m_ui.extendCheckBox->setChecked(false);
		}
		m_ui.manualExtendCheckBox->blockSignals(false);
		m_ui.extendCheckBox->blockSignals(false);
		m_ui.noExtendCheckBox->blockSignals(false);
	}
	updateDetectionOptions();
}

void ToolBarPipeGroup::updateDetectionOptions()
{

	PipeDetectionOptions options;
	options.noisy = m_ui.noisyCheckBox->isChecked();
	options.optimized = m_ui.optimizedRadioButton->isChecked();
	options.extendMode = m_ui.extendCheckBox->isChecked() ? PipeDetectionExtendMode::Auto : (m_ui.manualExtendCheckBox->isChecked() ? PipeDetectionExtendMode::Manual : PipeDetectionExtendMode::Default);
	options.insulatedThickness = m_ui.insulatedCheckBox->isChecked() ? m_ui.insulatedLineEdit->getValue() : 0.;

	m_dataDispatcher.sendControl(new control::measure::SendPipeDetectionOptions(options));
}

void ToolBarPipeGroup::initPipeDetection()
{
	if (m_ui.pipeToolButton->isChecked() && (m_ui.fastRadioButton->isChecked() == true || m_ui.optimizedRadioButton->isChecked() == true))
		m_dataDispatcher.sendControl(new control::measure::ActivateFitCylinder());
	else
	{
		m_ui.pipeToolButton->setChecked(false);
		m_ui.robustToolButton->setChecked(false);
		m_dataDispatcher.sendControl(new control::function::Abort());
	}
}

void ToolBarPipeGroup::initLargePipeDetection()
{
	if (m_ui.robustToolButton->isChecked())
		m_dataDispatcher.sendControl(new control::measure::ActivateBigCylinderFit());
	else
	{
		m_ui.pipeToolButton->setChecked(false);
		m_ui.robustToolButton->setChecked(false);
		m_dataDispatcher.sendControl(new control::function::Abort());
	}
}

void ToolBarPipeGroup::initPipeStandardManagement()
{
	StandardListDialog* dialog(new StandardListDialog(m_dataDispatcher, StandardType::Pipe, this->parentWidget(), true)); 
	m_dataDispatcher.sendControl(new control::standards::SendStandards(StandardType::Pipe));
	dialog->show();
}