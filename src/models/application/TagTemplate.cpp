#include "models/application/TagTemplate.h"
#include "utils/Logger.h"
#include "gui/texts/DefaultTemplatesTexts.hpp"
#include "models/application/Ids.hpp"

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

std::vector<sma::TagTemplate> sma::GenerateDefaultTemplates()
{
	std::vector<TagTemplate> templates;

	//Annotation
	{
		TagTemplate annotation = TagTemplate(xg::Guid(ANNOTATION_TEMP_ID), ANNOTATION_TEMPLATE_NAME.toStdWString());
		annotation.setOriginTemplate(true);
		templates.push_back(annotation);
	}

	//Warning
	{
		TagTemplate warning = TagTemplate(xg::Guid(WARNING_TEMP_ID), WARNING_TEMPLATE_NAME.toStdWString());
		warning.setOriginTemplate(true);
		//risk
		tField risk(xg::Guid(WARNING_RISK_ID), RISK_FIELD_NAME.toStdWString() , tFieldType::list, xg::Guid(LIST_RISKS_ID),L"");
		warning.addNewField(risk);
		templates.push_back(warning);
	}
	
	//Modeling Guide
	{
		TagTemplate modeling = TagTemplate(xg::Guid(MODELING_GUIDE_TEMP_ID), MODELINGGUIDE_TEMPLATE_NAME.toStdWString());
		modeling.setOriginTemplate(true);
		//lod
		tField lod({ xg::Guid(MODELING_GUIDE_LOD_ID), LOD_FIELD_NAME.toStdWString() , tFieldType::list, xg::Guid(LIST_LOD_ID),L"" });
		modeling.addNewField(lod);
		//lod
		tField accuracy({ xg::Guid(MODELING_GUIDE_ACCU_ID), ACCURACY_FIELD_NAME.toStdWString() , tFieldType::list, xg::Guid(LIST_MOD_ACCU_ID),L"" });
		modeling.addNewField(accuracy);
		//lod
		tField status({ xg::Guid(MODELING_GUIDE_STATUS_ID), STATUS_FIELD_NAME.toStdWString() , tFieldType::list, xg::Guid(LIST_STATUS_ID),L"" });
		modeling.addNewField(status);
		//lod
		tField type({ xg::Guid(MODELING_GUIDE_TYPE_ID), MODELING_FIELD_NAME.toStdWString() , tFieldType::list, xg::Guid(LIST_MODELING_TYPE_ID),L"" });
		modeling.addNewField(type);
		templates.push_back(modeling);
	}

	return templates;
}