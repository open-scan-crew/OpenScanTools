#include "models/data/Tag/TagData.h"

#include "controller/ControllerContext.h"

TagData::TagData()
{ }

TagData::TagData(const TagData & data)
{
	copyTagData(data);
}

TagData::~TagData()
{ }

void TagData::copyTagData(const TagData& data)
{
	//Data::icon_ = data.getMarkerIcon();
	m_fields = data.getFields();
	m_template = data.getTemplate();
}

void TagData::setValue(sma::tFieldId id, std::wstring newValue)
{
	if (m_fields.find(id) != m_fields.end())
		m_fields.at(id) = newValue;
	else
		m_fields.insert({ id, newValue });
}

void TagData::addField(sma::tFieldId id, std::wstring newValue)
{
	if (m_fields.find(id) == m_fields.end())
		m_fields.insert({ id, newValue });
}

void TagData::removeField(sma::tFieldId id)
{
	m_fields.erase(id);
}

void TagData::setDefaultData(const ControllerContext& context)
{
	setTemplate(context.getCurrentTemplate());
}

void TagData::setFields(const std::unordered_map<sma::tFieldId, std::wstring>& fields)
{
	for(const auto& iterator : fields)
		setValue(iterator.first, iterator.second);
}

void TagData::setTemplate(const SafePtr<sma::TagTemplate>& temp)
{
	m_template = temp;
}

SafePtr<sma::TagTemplate> TagData::getTemplate() const
{
	return (m_template);
}

std::wstring TagData::getValue(sma::tFieldId id) const
{
	if (m_fields.find(id) != m_fields.end())
		return (m_fields.at(id));

	{
		ReadPtr<sma::TagTemplate> rTemp = m_template.cget();
		if (rTemp && rTemp->getFields().find(id) != rTemp->getFields().end())
			return rTemp->getFields().at(id).m_defaultValue;
	}

	return (L"");
}

const std::unordered_map<sma::tFieldId, std::wstring>& TagData::getFields() const
{
	return (m_fields);
}