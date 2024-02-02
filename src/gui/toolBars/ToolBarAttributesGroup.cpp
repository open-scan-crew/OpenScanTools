#include "gui/toolBars/ToolBarAttributesGroup.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataList.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/controls/ControlCreationAttributesEdition.h"
#include "io/FileUtils.h"
#include "gui/widgets/CustomWidgets/GlobalColorPicker.h"
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton.h>
#include <QtGui/QScreen>

ToolBarAttributesGroup::ToolBarAttributesGroup(Controller& controller, QWidget* parent,const float& guiScale)
	: QWidget(parent)
	, m_dataDispatcher(controller.getDataDispatcher())
{
	m_ui.setupUi(this);
	setEnabled(false);
	m_ui.colorPicker->setDataDispatcher(&controller.getDataDispatcher());

	connect(m_ui.colorPicker, &ColorPicker::pickedColor, this, &ToolBarAttributesGroup::onColorChange);
	connect(m_ui.disciplineComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarAttributesGroup::onDisciplineChange);
	connect(m_ui.phaseComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarAttributesGroup::onPhaseChange);

	connect(m_ui.identifierLineEdit, &QLineEdit::editingFinished, this, &ToolBarAttributesGroup::onIdentifierChange);
	connect(m_ui.nameLineEdit, &QLineEdit::editingFinished, this, &ToolBarAttributesGroup::onNameChange);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendListsList);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendDisciplineSelected);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendPhaseSelected);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::nameSelection);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::identifierSelection);

	m_methods.insert({ guiDType::projectLoaded, &ToolBarAttributesGroup::onProjectLoad });
	m_methods.insert({ guiDType::sendListsList, &ToolBarAttributesGroup::onList });
	m_methods.insert({ guiDType::sendDisciplineSelected, &ToolBarAttributesGroup::onDisciplineSelected });
	m_methods.insert({ guiDType::sendPhaseSelected, &ToolBarAttributesGroup::onPhaseSelected });
	m_methods.insert({ guiDType::nameSelection, &ToolBarAttributesGroup::onNameSelected });
	m_methods.insert({ guiDType::identifierSelection, &ToolBarAttributesGroup::onIdentifierSelected });


}

void ToolBarAttributesGroup::hideColorPicker(const bool& visible)
{
	m_ui.colorPicker->setVisible(visible);
	m_ui.colorLabel->setVisible(visible);
}

void ToolBarAttributesGroup::informData(IGuiData* data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		AttributesGroupMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarAttributesGroup::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
}

void ToolBarAttributesGroup::onList(IGuiData* data)
{
	const std::unordered_set<SafePtr<UserList>>& lists = static_cast<GuiDataSendListsList*>(data)->_lists;
	for (SafePtr<UserList> list : lists)
	{
		ReadPtr<UserList> rList = list.cget();
		if (!rList)
			continue;
		xg::Guid id = rList->getId();

		if (id == listId(DISCIPLINE_ID))
			m_disciplineList = list;

		if (id == listId(PHASE_ID))
			m_phaseList = list;
	}

	updateLists();
}

void ToolBarAttributesGroup::onDisciplineSelected(IGuiData* data)
{
	m_ui.disciplineComboBox->blockSignals(true);
	m_ui.disciplineComboBox->setCurrentIndex(static_cast<GuiDataSendDisciplineSelected*>(data)->m_id);
	m_ui.disciplineComboBox->blockSignals(false);
}

void ToolBarAttributesGroup::onPhaseSelected(IGuiData* data)
{
	m_ui.phaseComboBox->blockSignals(true);
	m_ui.phaseComboBox->setCurrentIndex(static_cast<GuiDataSendPhaseSelected*>(data)->m_id);
	m_ui.phaseComboBox->blockSignals(false);
}

void ToolBarAttributesGroup::onNameSelected(IGuiData* data)
{
	m_ui.nameLineEdit->blockSignals(true);
	m_ui.nameLineEdit->setText(QString::fromStdWString(static_cast<GuiDataNameSelection*>(data)->m_name));
	m_ui.nameLineEdit->blockSignals(false);
}

void ToolBarAttributesGroup::onIdentifierSelected(IGuiData* data)
{
	m_ui.identifierLineEdit->blockSignals(true);
	m_ui.identifierLineEdit->setText(QString::fromStdWString(static_cast<GuiDataIdentifierSelection*>(data)->m_identifier));
	m_ui.identifierLineEdit->blockSignals(false);
}

void ToolBarAttributesGroup::onNameChange()
{
	m_dataDispatcher.sendControl(new control::attributesEdition::SetDefaultName(m_ui.nameLineEdit->text().toStdWString()));
	m_dataDispatcher.updateInformation(new GuiDataNameSelection(m_ui.nameLineEdit->text().toStdWString()), this);
}

void ToolBarAttributesGroup::onIdentifierChange()
{
	m_dataDispatcher.sendControl(new control::attributesEdition::SetDefaultIdentifier(m_ui.identifierLineEdit->text().toStdWString()));
	m_dataDispatcher.updateInformation(new GuiDataIdentifierSelection(m_ui.identifierLineEdit->text().toStdWString()), this);
}

void ToolBarAttributesGroup::onPhaseChange(const int& index)
{
	m_dataDispatcher.sendControl(new control::attributesEdition::SetDefaultPhase(m_ui.phaseComboBox->currentText().toStdWString()));
	m_dataDispatcher.updateInformation(new GuiDataSendPhaseSelected(index), this);
}

void ToolBarAttributesGroup::onDisciplineChange(const int& index)
{
	m_dataDispatcher.sendControl(new control::attributesEdition::SetDefaultDiscipline(m_ui.disciplineComboBox->currentText().toStdWString()));
	m_dataDispatcher.updateInformation(new GuiDataSendDisciplineSelected(index), this);
}

void ToolBarAttributesGroup::onColorChange(const Color32& color)
{
	m_dataDispatcher.sendControl(new control::attributesEdition::SetDefaultColor(color));
}

ToolBarAttributesGroup::~ToolBarAttributesGroup()
{
	m_dataDispatcher.unregisterObserver(this);
}

void ToolBarAttributesGroup::updateLists()
{
	m_ui.disciplineComboBox->blockSignals(true);
	m_ui.phaseComboBox->blockSignals(true);
	std::wstring selected;

	{
		ReadPtr<UserList> rDisc = m_disciplineList.cget();
		selected = m_ui.disciplineComboBox->currentText().toStdWString();
		m_ui.disciplineComboBox->clear();
		if (rDisc)
		{

			int storedId = rDisc->clist().empty() ? -1 : 0;
			int i = 0;
			for (std::wstring data : rDisc->clist())
			{
				m_ui.disciplineComboBox->addItem(QString::fromStdWString(data));
				if (selected == L"" && (data == L"N.A." || data == L"Sans objet"))
					storedId = i;
				if (selected == data)
					storedId = i;
				++i;
			}
			if (storedId >= 0)
				m_ui.disciplineComboBox->setCurrentIndex(storedId);
			if (m_ui.disciplineComboBox->currentText().toStdString() != "")
				m_dataDispatcher.sendControl(new control::attributesEdition::SetDefaultDiscipline(m_ui.disciplineComboBox->currentText().toStdWString()));

		}
	}
	
	{
		ReadPtr<UserList> rPhase = m_phaseList.cget();
		selected = m_ui.phaseComboBox->currentText().toStdWString();
		m_ui.phaseComboBox->clear();
		if (rPhase)
		{
			int i = 0;

			int storedId = rPhase->clist().empty() ? -1 : 0;

			for (std::wstring data : rPhase->clist())
			{
				m_ui.phaseComboBox->addItem(QString::fromStdWString(data));
				if (selected == L"" && (data == L"N.A." || data == L"Sans objet"))
					storedId = i;
				if (selected == data)
					storedId = i;
				++i;
			}
			if (storedId >= 0)
				m_ui.phaseComboBox->setCurrentIndex(storedId);
			if (m_ui.phaseComboBox->currentText().toStdString() != "")
				m_dataDispatcher.sendControl(new control::attributesEdition::SetDefaultPhase(m_ui.phaseComboBox->currentText().toStdWString()));

		}
	}

	m_ui.disciplineComboBox->blockSignals(false);
	m_ui.phaseComboBox->blockSignals(false);
}
