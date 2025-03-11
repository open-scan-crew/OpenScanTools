#include "models/application/TagTemplate.h"
#include "models/application/Ids.hpp"

#include <map>

namespace sma
{
	TagTemplate::TagTemplate()
	{
		m_templateName = L"";
	}

	TagTemplate::TagTemplate(templateId id, std::wstring name)
	{
		m_id = id;
		m_templateName = name;
	}

	TagTemplate::TagTemplate(std::wstring name)
	{
		m_id = xg::newGuid();
		m_templateName = name;
	}

	TagTemplate::TagTemplate(const TagTemplate& temp)
		: m_fieldOrder(temp.m_fieldOrder)
		, m_fields(temp.m_fields)
		, m_id(temp.m_id)
		, m_origin(temp.m_origin)
		, m_templateName(temp.m_templateName)
		, m_valid(temp.m_valid)
	{}

	std::wstring TagTemplate::renameTagTemplate(std::wstring newName)
	{
		std::wstring oldName(m_templateName);
		m_templateName = newName;
		return (oldName);
	}

	void TagTemplate::mergeTemplate(TagTemplate tmp)
	{
		for (auto field : tmp.getFields())
			addNewField(field.second);
	}

	void TagTemplate::setId(const templateId& id)
	{
		m_id = id;
	}

	void TagTemplate::addNewField(tField newField)
	{
		if (m_fields.find(newField.m_id) == m_fields.end())
		{
			m_fields.insert(std::pair<tFieldId, tField>(newField.m_id, newField));
			m_fieldOrder.push_back(newField.m_id);
		}
	}

	void TagTemplate::addNewField(std::wstring name, tFieldType type)
	{
		tField field;
		field.m_id = xg::newGuid();
		field.m_name = name;
		field.m_type = type;
		field.m_defaultValue = L"";

		m_fields.insert(std::pair<tFieldId, tField>(field.m_id, field));
		m_fieldOrder.push_back(field.m_id);
	}

	std::wstring TagTemplate::modifyFieldName(tFieldId idToModify, std::wstring newName)
	{
		std::wstring oldKey = L"";

		if (m_fields.find(idToModify) != m_fields.end())
		{
			oldKey = m_fields.at(idToModify).m_name;
			m_fields.at(idToModify).m_name = newName;
		}
		return (oldKey);
	}

	void TagTemplate::modifyFieldType(tFieldId idToModify, tFieldType newType)
	{

		if (m_fields.find(idToModify) != m_fields.end())
		{
			m_fields.at(idToModify).m_type = newType;
			m_fields.at(idToModify).m_fieldReference.reset();
			m_fields.at(idToModify).m_defaultValue = L"";
			//_id = xg::newGuid();
		}
	}

	void TagTemplate::modifyFieldReference(tFieldId idToModify, SafePtr<UserList> newfParam)
	{

		if (m_fields.find(idToModify) != m_fields.end())
		{
			m_fields.at(idToModify).m_fieldReference = newfParam;
			m_fields.at(idToModify).m_defaultValue = L""; 
			//_id = xg::newGuid();
		}

	}

	std::wstring TagTemplate::modifyFieldDefaultValue(tFieldId idToModify, std::wstring newValue)
	{
		std::wstring oldValue = L"";

		if (m_fields.find(idToModify) != m_fields.end())
		{
			oldValue = m_fields.at(idToModify).m_defaultValue;
			m_fields.at(idToModify).m_defaultValue = newValue;
		}
		return (oldValue);
	}

	void TagTemplate::removeField(tFieldId id)
	{
		m_fields.erase(id);
	}

	void TagTemplate::clearFields()
	{
		m_fields.clear();
		//_id = xg::newGuid();
	}

	void TagTemplate::setOriginTemplate(bool value)
	{
		m_origin = value;
	}

	const std::wstring & TagTemplate::getName() const
	{
		return (m_templateName);
	}

	bool TagTemplate::isAOriginTemplate() const
	{
		return (m_origin);
	}

	bool TagTemplate::isValid() const
	{
		return (m_id.isValid());
	}

	templateId TagTemplate::getId() const
	{
		return (m_id);
	}

	std::vector<tField> TagTemplate::getFieldsCopy() const
	{
		std::vector<tField> copy;

		for (auto it = m_fields.begin(); it != m_fields.end(); it++)
			copy.push_back(it->second);
		return (copy);
	}

	const std::unordered_map<tFieldId, tField>& TagTemplate::getFields() const
	{
		return (m_fields);
	}

	int TagTemplate::getFieldSize()
	{
		return (int)(m_fields.size());
	}

	bool TagTemplate::operator==(const TagTemplate& rhs) const
	{
		return this->getId() == rhs.getId();
	}

	bool TagTemplate::operator<(const TagTemplate& rhs) const
	{
		return this->getId() < rhs.getId();
	}

	size_t TagTemplate::operator()(const TagTemplate& tagTemp) const
	{
		return std::hash<xg::Guid>()(tagTemp.getId());
	}
}

std::map<xg::Guid, std::array<std::wstring, 2>> tmpl_names = {
    { xg::Guid(ANNOTATION_TEMP_ID),       { L"Annotation", L"Annotation" } },
    { xg::Guid(WARNING_TEMP_ID),          { L"Risks", L"Risques" } },
    { xg::Guid(MODELING_GUIDE_TEMP_ID),   { L"Modeling guide", L"Guide de modélisation" } },
    { xg::Guid(WARNING_RISK_ID),          { L"Risks", L"Risques" } },
    { xg::Guid(MODELING_GUIDE_LOD_ID),    { L"LOD required", L"Niveau de détail" }},
    { xg::Guid(MODELING_GUIDE_ACCU_ID),   { L"Accuracy", L"Précision" }},
    { xg::Guid(MODELING_GUIDE_STATUS_ID), { L"Status", L"Statut" }},
    { xg::Guid(MODELING_GUIDE_TYPE_ID),   { L"Modeling type", L"Type de modélisation" }}
};

sma::TagTemplate init_tag_tmpl(sma::templateId id, LanguageType lang)
{
	std::wstring name = tmpl_names.at(id)[(int)lang];
	return sma::TagTemplate(id, name);
}

sma::tField init_field(sma::templateId id, xg::Guid listId, LanguageType lang)
{
	sma::tField field;
	field.m_id = id;
	field.m_name = tmpl_names[id][(int)lang];
	field.m_type = sma::tFieldType::list;
	field.m_fieldReferenceId = listId;
	return field;
}

std::vector<sma::TagTemplate> sma::GenerateDefaultTemplates(LanguageType lang)
{
	std::vector<TagTemplate> templates;

	//Annotation
	{
		TagTemplate annotation = init_tag_tmpl(xg::Guid(ANNOTATION_TEMP_ID), lang);
		annotation.setOriginTemplate(true);
		templates.push_back(annotation);
	}

	//Warning
	{
		TagTemplate warning = init_tag_tmpl(xg::Guid(WARNING_TEMP_ID), lang);
		warning.setOriginTemplate(true);

		tField risk = init_field(xg::Guid(WARNING_RISK_ID), xg::Guid(LIST_RISKS_ID), lang);
		warning.addNewField(risk);
		templates.push_back(warning);
	}
	
	//Modeling Guide
	{
		TagTemplate modeling = init_tag_tmpl(xg::Guid(MODELING_GUIDE_TEMP_ID), lang);
		modeling.setOriginTemplate(true);

		tField lod = init_field(xg::Guid(MODELING_GUIDE_LOD_ID), xg::Guid(LIST_LOD_ID), lang);
		modeling.addNewField(lod);

		tField accuracy = init_field(xg::Guid(MODELING_GUIDE_ACCU_ID), xg::Guid(LIST_MOD_ACCU_ID), lang);
		modeling.addNewField(accuracy);

		tField status = init_field(xg::Guid(MODELING_GUIDE_STATUS_ID), xg::Guid(LIST_STATUS_ID), lang);
		modeling.addNewField(status);

		tField type = init_field(xg::Guid(MODELING_GUIDE_TYPE_ID), xg::Guid(LIST_MODELING_TYPE_ID), lang);
		modeling.addNewField(type);
		templates.push_back(modeling);
	}

	return templates;
}