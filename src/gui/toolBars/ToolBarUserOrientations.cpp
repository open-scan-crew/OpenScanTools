#include "gui/toolBars/ToolBarUserOrientations.h"

#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataUserOrientation.h"

#include "controller/controls/ControlUserOrientation.h"


ToolBarUserOrientation::ToolBarUserOrientation(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
	, m_useUserOrientation(false)
{
	m_ui.setupUi(this);
	setEnabled(false);

    // Projection Mode
    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::callbackUO);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendUserOrientationList);

	connect(m_ui.editButton, &QAbstractButton::released, this, [this]() {this->slotEditOrientation(this->m_ui.orientationsListBox->currentIndex()); });
    connect(m_ui.newButton, &QAbstractButton::released, this, &ToolBarUserOrientation::slotNewOrientation);

	connect(m_ui.projectButton, &QRadioButton::released, this, [this]() {this->changeOrientation(false); });
	connect(m_ui.userButton, &QRadioButton::released, this, [this]() {this->changeOrientation(true); });
	connect(m_ui.orientationsListBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarUserOrientation::slotSetOrientation);

	m_methods.insert(std::pair<guiDType, UserOrientationMethod>(guiDType::projectLoaded, &ToolBarUserOrientation::onProjectLoad));
	m_methods.insert(std::pair<guiDType, UserOrientationMethod>(guiDType::callbackUO, &ToolBarUserOrientation::onEditUO));
	m_methods.insert(std::pair<guiDType, UserOrientationMethod>(guiDType::sendUserOrientationList, &ToolBarUserOrientation::onLoadUO));


	
}

ToolBarUserOrientation::~ToolBarUserOrientation()
{
	m_dataDispatcher.unregisterObserver(this);
}

void ToolBarUserOrientation::informData(IGuiData *data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		UserOrientationMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarUserOrientation::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
}

void ToolBarUserOrientation::onEditUO(IGuiData * data)
{
	m_ui.orientationsListBox->blockSignals(true);
	GuiDataCallbackAddRemoveUserOrientation* addRemoveUo = static_cast<GuiDataCallbackAddRemoveUserOrientation*>(data);
	
	int ind = 0;
	bool edit = false;
	for (auto i = m_idList.begin(); i != m_idList.end(); i++) {
		if (*i == addRemoveUo->m_id) {
			if (addRemoveUo->m_remove)
				m_idList.erase(i);
			edit = true;
			break;
		}
		ind++;
	}

	if (ind <= m_ui.orientationsListBox->count()) {
		if (addRemoveUo->m_remove)
			m_ui.orientationsListBox->removeItem(ind);
		else if (edit)
			m_ui.orientationsListBox->setItemText(ind, addRemoveUo->m_name);
	}
	if (!edit) {
		m_idList.push_back(addRemoveUo->m_id);
		m_ui.orientationsListBox->addItem(addRemoveUo->m_name);
	}
	
	m_ui.orientationsListBox->blockSignals(false);
	slotSetOrientation(m_ui.orientationsListBox->currentIndex());
}

void ToolBarUserOrientation::onLoadUO(IGuiData * data)
{
	m_ui.orientationsListBox->blockSignals(true);
	GuiDataSendUserOrientationList* uos = static_cast<GuiDataSendUserOrientationList*>(data);
	std::unordered_map<uint32_t, QString> comboBox;
	m_ui.orientationsListBox->clear();
	m_idList.clear();
	for (auto it = uos->m_orientations.begin(); it != uos->m_orientations.end(); it++) {
		m_idList.push_back(it->second.first);
		m_ui.orientationsListBox->addItem(it->second.second);
	}
	m_ui.orientationsListBox->blockSignals(false);
	slotSetOrientation(m_ui.orientationsListBox->currentIndex());
}


void ToolBarUserOrientation::changeOrientation(bool isUserOrientation)
{
	if (isUserOrientation == m_useUserOrientation)
		return;
	m_useUserOrientation = isUserOrientation;
	slotSetOrientation(m_ui.orientationsListBox->currentIndex());
}



void ToolBarUserOrientation::slotSetOrientation(int idComboBox)
{
	if (m_useUserOrientation && idComboBox >= 0 )
		m_dataDispatcher.sendControl(new control::userOrientation::SetUserOrientation(m_idList[idComboBox]));
	else
		m_dataDispatcher.sendControl(new control::userOrientation::UnsetUserOrientation());
}

void ToolBarUserOrientation::slotEditOrientation(int idComboBox)
{
	if (idComboBox >= 0)
		m_dataDispatcher.sendControl(new control::userOrientation::UserOrientationsProperties(m_idList[idComboBox]));
}

void ToolBarUserOrientation::slotNewOrientation()
{
	m_dataDispatcher.sendControl(new control::userOrientation::UserOrientationsProperties());
}


