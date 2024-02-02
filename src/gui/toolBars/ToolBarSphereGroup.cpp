#include "gui/toolBars/ToolBarSphereGroup.h"
#include "gui/Dialog/StandardListDialog.h"
#include "controller/controls/ControlFunction.h"
#include "controller/controls/ControlMeasure.h"
#include "controller/controls/ControlStandards.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataList.h"
#include <QtWidgets/qpushbutton.h>

ToolBarSphereGroup::ToolBarSphereGroup(IDataDispatcher& dataDispatcher, QWidget* parent, float scale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);
	m_ui.sphereToolButton->setIconSize(QSize(20, 20) * scale);
	m_ui.robustToolButton->setIconSize(QSize(20, 20) * scale);

	m_ui.standardsComboBox->setVisible(false);
	m_ui.managePushButton->setVisible(false);


	connect(m_ui.sphereToolButton, &QToolButton::released, this, &ToolBarSphereGroup::initSphereDetection);
	connect(m_ui.robustToolButton, &QToolButton::released, this, &ToolBarSphereGroup::initLargeSphereDetection);
	connect(m_ui.managePushButton, &QToolButton::released, this, &ToolBarSphereGroup::initSphereStandardManagement);
	
	connect(m_ui.standardsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarSphereGroup::slotStandardChanged);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendListsStandards);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_methods.insert({ guiDType::activatedFunctions, &ToolBarSphereGroup::onActivateFunction });
	m_methods.insert({ guiDType::sendListsStandards, &ToolBarSphereGroup::onStandardRecieved });
	m_methods.insert({ guiDType::projectLoaded, &ToolBarSphereGroup::onProjectLoad });
}

ToolBarSphereGroup::~ToolBarSphereGroup()
{
	m_dataDispatcher.unregisterObserver(this);
}

void ToolBarSphereGroup::informData(IGuiData* data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		SphereGroupMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarSphereGroup::onProjectLoad(IGuiData* data)
{
	auto* function = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(function->m_isProjectLoad);
}

void ToolBarSphereGroup::onStandardRecieved(IGuiData* data)
{
	m_ui.standardsComboBox->blockSignals(true);
	auto* standards = static_cast<GuiDataSendListsStandards*>(data);
	if (standards->m_type != StandardType::Sphere)
		return;
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
		if (rStand && rStand->getOrigin())
			continue;
		m_standardsElems.push_back(stand);
		m_ui.standardsComboBox->addItem(QString::fromStdWString(rStand->getName()));
	}

	m_ui.standardsComboBox->setCurrentIndex(selectedItem);
	m_ui.standardsComboBox->blockSignals(false);

	slotStandardChanged(selectedItem);
}

void ToolBarSphereGroup::slotStandardChanged(const int& index)
{
	if(m_standardsElems.size() > index)
		m_dataDispatcher.sendControl(new control::standards::SelectStandard(m_standardsElems[index], StandardType::Sphere));
}

void ToolBarSphereGroup::onActivateFunction(IGuiData* data)
{
	auto* function = static_cast<GuiDataActivatedFunctions*>(data);

	m_ui.sphereToolButton->blockSignals(true);
	m_ui.robustToolButton->blockSignals(true);

	if (function->type == ContextType::Sphere)
	{
		m_ui.sphereToolButton->setChecked(true);
		m_ui.robustToolButton->setChecked(false);
	}
    else if (function->type == ContextType::ClicsSphere4)
	{
		m_ui.sphereToolButton->setChecked(false);
		m_ui.robustToolButton->setChecked(true);
	}
	else
	{
		m_ui.sphereToolButton->setChecked(false);
		m_ui.robustToolButton->setChecked(false);
	}

	m_ui.sphereToolButton->blockSignals(false);
	m_ui.robustToolButton->blockSignals(false);
}

void ToolBarSphereGroup::initSphereDetection()
{
	m_dataDispatcher.sendControl(new control::measure::ActivateSphere());	
}

void ToolBarSphereGroup::initLargeSphereDetection()
{
	m_dataDispatcher.sendControl(new control::measure::Activate4ClicsSphere());
}

void ToolBarSphereGroup::initSphereStandardManagement()
{
	StandardListDialog* dialog(new StandardListDialog(m_dataDispatcher, StandardType::Sphere, this->parentWidget(), true)); 
	m_dataDispatcher.sendControl(new control::standards::SendStandards(StandardType::Sphere));
	dialog->show();
}