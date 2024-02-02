#include "gui/widgets/QPFields/QPFactory.h"
#include "gui/widgets/QPFields/QPStringField.h"
#include "gui/widgets/QPFields/QPDateField.h"
#include "gui/widgets/QPFields/QPNumberField.h"
#include "gui/widgets/QPFields/QPHyperLinkField.h"
#include "gui/widgets/QPFields/QPListField.h"
#include "gui/IPanel.h"

#include "magic_enum/magic_enum.hpp"

QPFactory::QPFactory()
{}

QPFactory::~QPFactory()
{}

AQPField* QPFactory::getField(const sma::tField field)
{
	switch (field.m_type)
	{
		case sma::tFieldType::string:
			return new QPStringField(field, nullptr);
			break;
		case sma::tFieldType::date:
			return new QPDateField(field, nullptr);
			break;
		case sma::tFieldType::list:
			return new QPListField(field, nullptr);
			break;
		case sma::tFieldType::number:
			return new QPNumberField(field, nullptr);
			break;
		case sma::tFieldType::hyperlink:
			return new QPHyperLinkField(field, nullptr);
			break;
		default:
			PANELLOG << "field creation : field not recognized : " << magic_enum::enum_name(field.m_type) << LOGENDL;
			return nullptr;
	}
	return nullptr;
}

/*TemplatePropertiesPanel * QPFactory::createTemplatePanel(IDataDispatcher& dispat, sma::TagTemplate temp, QWidget *parent)
{
	TemplatePropertiesPanel *panel = new TemplatePropertiesPanel(dispat, parent);

	for (auto it = temp.getFields().begin(); it != temp.getFields().end(); it++)
	{
		IQPField *newField = getField(it->second);
		if (newField != nullptr)
			panel->addField(newField->getFieldId(), newField);
	}

	return (panel);
}*/