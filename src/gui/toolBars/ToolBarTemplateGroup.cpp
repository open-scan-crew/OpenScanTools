#include <QtWidgets/qwidget.h>
#include <QtGui/QScreen>

#include "gui/Dialog/ListListDialog.h"
#include "gui/toolBars/ToolBarTemplateGroup.h"
#include "gui/widgets/CustomWidgets/ColorPicker.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataTag.h"
#include "gui/GuiData/GuiDataTemplate.h"
#include "controller/controls/ControlFunctionTag.h"
#include "controller/controls/ControlFunction.h"
#include "controller/controls/ControlCreationAttributesEdition.h"
#include "controller/controls/ControlUserList.h"
#include "services/MarkerDefinitions.hpp"
#include "gui/widgets/CustomWidgets/GlobalColorPicker.h"

#include "utils/Logger.h"
#include "utils/ProjectStringSets.hpp"

#include "models/application/Ids.hpp"

ToolBarTemplateGroup::ToolBarTemplateGroup(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);
    m_ui.iconTagButton->setIconSize(QSize(30, 30) * guiScale);

    m_iconSelectionDialog = new MarkerIconSelectionDialog(this, guiScale);

	QObject::connect(m_ui.TemplateComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeTemplateCombo(int)));
    QObject::connect(m_iconSelectionDialog, &MarkerIconSelectionDialog::markerSelected, this, &ToolBarTemplateGroup::changeMarkerIcon);
    QObject::connect(m_ui.iconTagButton, &QToolButton::released, this, &ToolBarTemplateGroup::clickIconTag);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendTemplateList);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::tagDefaultIcon);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::tagDefaultColor);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendTagTemplateSelected);
	
	methods.insert({ guiDType::projectLoaded, &ToolBarTemplateGroup::onProjectLoad });
	methods.insert({ guiDType::sendTemplateList, &ToolBarTemplateGroup::onTemplateListReceive });
	methods.insert({ guiDType::tagDefaultIcon, &ToolBarTemplateGroup::onIconTag });
	methods.insert({ guiDType::sendTagTemplateSelected, &ToolBarTemplateGroup::onTemplateSelected });


}

ToolBarTemplateGroup::~ToolBarTemplateGroup()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarTemplateGroup::informData(IGuiData *data)
{
	if (methods.find(data->getType()) != methods.end())
	{
		TemplateGroupMethod method = methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarTemplateGroup::onProjectLoad(IGuiData *data)
{
    GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
    setEnabled(plData->m_isProjectLoad);
}

void ToolBarTemplateGroup::onIconTag(IGuiData *data)
{
	GuiDataTagDefaultIcon *iconData = static_cast<GuiDataTagDefaultIcon*>(data);
	m_ui.iconTagButton->setIcon(QIcon(scs::markerStyleDefs.at(iconData->icon).qresource));
}

void ToolBarTemplateGroup::onTemplateListReceive(IGuiData *data)
{
	m_ui.TemplateComboBox->blockSignals(true);

	GuiDataSendTemplateList *tData = static_cast<GuiDataSendTemplateList*>(data);
	SafePtr<sma::TagTemplate> selected;
	if (templateElems.size() > m_ui.TemplateComboBox->currentIndex() && m_ui.TemplateComboBox->currentIndex() >= 0)
		selected = templateElems.at(m_ui.TemplateComboBox->currentIndex());

	m_ui.TemplateComboBox->clear();
	templateElems.clear();

	bool isSelectedIdInNewTemplates = false;
	SafePtr<sma::TagTemplate> annottationTemp;
	for (SafePtr<sma::TagTemplate> temp : tData->m_templates)
	{
		ReadPtr<sma::TagTemplate> rTemp = temp.cget();
		if (!rTemp)
			continue;

		templateElems.push_back(temp);
		m_ui.TemplateComboBox->addItem(QString::fromStdWString(rTemp->getName()));

		if (temp == selected)
			isSelectedIdInNewTemplates = true;
		if (rTemp->getId() == xg::Guid(ANNOTATION_TEMP_ID))
			annottationTemp = temp;
	}

	if (!isSelectedIdInNewTemplates)
	{
		if (annottationTemp)
			selected = annottationTemp;
		else
		{
			if (!templateElems.empty())
				selected = templateElems[0];
			else
				selected = SafePtr<sma::TagTemplate>();
		}
	}
		
	if (selected)
	{
		m_dataDispatcher.sendControl(new control::function::tag::SetCurrentTagTemplate(selected));
		m_dataDispatcher.updateInformation(new GuiDataTagTemplateSelected(selected), this);
	}

	m_ui.TemplateComboBox->blockSignals(false);
}

void ToolBarTemplateGroup::changeTemplateCombo(int id)
{
	SafePtr<sma::TagTemplate> select = templateElems[id];
	m_dataDispatcher.sendControl(new control::function::tag::SetCurrentTagTemplate(select));
	m_dataDispatcher.updateInformation(new GuiDataTagTemplateSelected(select), this);
}

void ToolBarTemplateGroup::changeTagColor(Color32 color)
{
	m_dataDispatcher.sendControl(new control::attributesEdition::SetDefaultColor(color));
}

void ToolBarTemplateGroup::changeMarkerIcon(scs::MarkerIcon icon)
{
    m_dataDispatcher.sendControl(new control::function::tag::SetDefaultIcon(icon));
    m_ui.iconTagButton->setIcon(QIcon(scs::markerStyleDefs.at(icon).qresource));
}

void ToolBarTemplateGroup::clickIconTag()
{
    m_iconSelectionDialog->show();
}

void ToolBarTemplateGroup::onTemplateSelected(IGuiData* data)
{
	SafePtr<sma::TagTemplate> sTemp = static_cast<GuiDataTagTemplateSelected*>(data)->m_temp;
	int comboId = 0;

	for (SafePtr<sma::TagTemplate> templ : templateElems)
	{
		if (templ == sTemp)
			break;
		comboId++;
	}

	m_ui.TemplateComboBox->blockSignals(true);
	m_ui.TemplateComboBox->setCurrentIndex(comboId >= templateElems.size() ? 0 : comboId);
	m_ui.TemplateComboBox->blockSignals(false);
}