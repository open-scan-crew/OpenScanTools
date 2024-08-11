#include "gui/widgets/QPFields/TemplatePropertiesPanel.h"
#include "gui/GuiData/GuiDataTag.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "controller/Controller.h"
#include "controller/controls/ControlTagEdition.h"
#include "controller/controls/ControlDataEdition.h"
#include "controller/controls/ControlAnimation.h"
#include "gui/widgets/QPFields/QPListField.h"
#include "services/MarkerDefinitions.hpp"
#include "controller/ControllerContext.h"

#include "controller/controls/ControlClippingEdition.h"
#include "controller/Controls/ControlObject3DEdition.h"

#include "utils/math/trigo.h"

#include <glm/gtx/norm.hpp>

#include "models/3d/Graph/TagNode.h"

#include <cctype>
#include <QHeaderView>

#include <QtWidgets/QScrollBar>

void cleanLayout(QLayout* layout)
{
	for (uint64_t iterator(0); iterator < layout->count(); iterator++)
	{
		QLayoutItem* child = layout->takeAt(0);
		if (child->layout() != 0)
		{
			cleanLayout(child->layout());
		}
		else if (child->widget() != 0)
		{
			delete child->widget();
		}
		delete child;
	}
}

TemplatePropertiesPanel::TemplatePropertiesPanel(Controller& controller, QWidget* parent)
	: APropertyGeneral(controller.getDataDispatcher(), parent)
	, m_iconSelector(this)
{
	m_ui.setupUi(this);
	m_ui.genericPropsHeadWidget->setControllerInfo(controller);
	m_ui.genericPropsFeetWidget->setDataDispatcher(controller.getDataDispatcher());
	m_ui.subPropertyClipping->setDataDispatcher(&controller.getDataDispatcher());

	connect(m_ui.ShapeToolButton, &QPushButton::released, this, &TemplatePropertiesPanel::onShapePress);

	connect(m_ui.XLineEdit, &QLineEdit::editingFinished, this, &TemplatePropertiesPanel::onXEdit);
	connect(m_ui.YLineEdit, &QLineEdit::editingFinished, this, &TemplatePropertiesPanel::onYEdit);
	connect(m_ui.ZLineEdit, &QLineEdit::editingFinished, this, &TemplatePropertiesPanel::onZEdit);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);

	m_ui.XLineEdit->setType(NumericType::DISTANCE);
	m_ui.YLineEdit->setType(NumericType::DISTANCE);
	m_ui.ZLineEdit->setType(NumericType::DISTANCE);

}

TemplatePropertiesPanel::~TemplatePropertiesPanel()
{
}

void TemplatePropertiesPanel::addField(sma::tFieldId id, AQPField* field, QVBoxLayout* layout)
{
	layout->addWidget(field);
	m_fields.insert(std::pair<sma::tFieldId, AQPField*>(id, field));
}

void TemplatePropertiesPanel::informData(IGuiData *data)
{
	APropertyGeneral::informData(data);
	if (m_tagsMethods.find(data->getType()) != m_tagsMethods.end())
	{
		TemplatePropertiesMethod method = m_tagsMethods.at(data->getType());
		(this->*method)(data);
	}
}

bool TemplatePropertiesPanel::actualizeProperty(SafePtr<AGraphNode> object )
{
	if (object)
		m_tag = static_pointer_cast<TagNode>(object);

	m_ui.genericPropsHeadWidget->setObject(m_tag);
	m_ui.genericPropsFeetWidget->setObject(m_tag);
	m_ui.subPropertyClipping->setObject(m_tag);

	for (auto it = m_fields.begin(); it != m_fields.end(); it++)
		delete (it->second);

	cleanLayout(m_ui.TemplateFieldsLayout);
	m_fields.clear();
	{
		ReadPtr<TagNode> rTag = m_tag.cget();
		if (rTag)
		{
			ReadPtr<sma::TagTemplate> rTagTemp = rTag->getTemplate().cget();
			if (rTagTemp)
			{
				m_templateName = rTagTemp->getName();
				for (auto fieldPair : rTagTemp->getFields())
				{
					AQPField* field = m_fieldFactory.getField(fieldPair.second);
					field->setParent(this);
					QObject::connect(field, &AQPField::edited, this, &TemplatePropertiesPanel::getFieldModification);
					if (field->getType() == sma::tFieldType::list && field->getFieldData().m_fieldReference)
					{
						ReadPtr<UserList> rFieldList = field->getFieldData().m_fieldReference.cget();
						if (rFieldList)
							static_cast<QPListField*>(field)->setList(*&rFieldList);
					}
					if (field != nullptr)
					{
						addField(field->getFieldId(), field, m_ui.TemplateFieldsLayout);
						field->setValue(QString::fromStdWString(fieldPair.second.m_defaultValue));
					}
				}
			}
		}
	}

	return updateTag();
}

bool TemplatePropertiesPanel::updateTag()
{
	ReadPtr<TagNode> tagNode = m_tag.cget();
	if (!tagNode)
		return false; 

	m_ui.XLineEdit->blockSignals(true);
	m_ui.YLineEdit->blockSignals(true);
	m_ui.ZLineEdit->blockSignals(true);

	m_ui.XLineEdit->setValue(tagNode->getCenter().x);
	m_ui.YLineEdit->setValue(tagNode->getCenter().y);
	m_ui.ZLineEdit->setValue(tagNode->getCenter().z);

	m_ui.XLineEdit->blockSignals(false);
	m_ui.YLineEdit->blockSignals(false);
	m_ui.ZLineEdit->blockSignals(false);


	m_ui.ShapeToolButton->setIcon(QIcon(scs::markerStyleDefs.at(tagNode->getMarkerIcon()).qresource));


	for (auto it = tagNode->getFields().begin(); it != tagNode->getFields().end(); it++)
	{
		if (m_fields.find(it->first) != m_fields.end())
			m_fields.at(it->first)->setValue(QString::fromStdWString(it->second));
	}

	emit nameChanged(windowTitle() + " : " + QString::fromStdWString(m_templateName));

	return true;
}

void TemplatePropertiesPanel::getFieldModification(sma::tFieldId id, std::wstring newData)
{
	PANELLOG << "data received on field : " << id.str() << " : " << newData << LOGENDL;
	m_dataDispatcher.sendControl(new control::tagEdition::SetFieldData(m_tag, id, newData));
}

void TemplatePropertiesPanel::onShapePress()
{
	connect(&m_iconSelector, &MarkerIconSelectionDialog::markerSelected, this, &TemplatePropertiesPanel::changeMarkerIcon);
	m_iconSelector.show();
}

void TemplatePropertiesPanel::changeMarkerIcon(scs::MarkerIcon icon)
{
	m_dataDispatcher.sendControl(new control::tagEdition::SetMarkerIcon(m_tag, icon));
	m_iconSelector.hide();
	m_ui.ShapeToolButton->setIcon(QIcon(scs::markerStyleDefs.at(icon).qresource));
}

void TemplatePropertiesPanel::changePosition()
{
	glm::dvec3 position(m_ui.XLineEdit->getValue(),
						m_ui.YLineEdit->getValue(),
						m_ui.ZLineEdit->getValue());
	m_dataDispatcher.sendControl(new control::object3DEdition::SetCenter(m_tag, position));
}

void TemplatePropertiesPanel::onXEdit()
{
	changePosition();
}

void TemplatePropertiesPanel::onYEdit()
{
	changePosition();
}

void TemplatePropertiesPanel::onZEdit()
{
	changePosition();
}