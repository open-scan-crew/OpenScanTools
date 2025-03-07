#ifndef TAG_TEMPLATE_H
#define TAG_TEMPLATE_H

#include <string>
#include "crossguid/guid.hpp"
#include "utils/safe_ptr.h"

#include "gui/LanguageType.h"
#include "models/application/List.h"

namespace sma
{
	typedef xg::Guid templateId;
	typedef xg::Guid tFieldId;

	enum class tFieldType
	{
		none,
		string,
		multiLine,
		number,
		list,
		date,
		hyperlink
	};

	struct tField
	{
		tFieldId m_id;
		std::wstring m_name;
		tFieldType m_type;

		xg::Guid m_fieldReferenceId; //Only for export without using list ptr
		SafePtr<UserList> m_fieldReference;

		std::wstring m_defaultValue;

		tField(tFieldId id, std::wstring name, tFieldType type, xg::Guid listId, std::wstring defaultValue)
			: m_id(id), m_name(name), m_type(type), m_fieldReferenceId(listId), m_fieldReference(), m_defaultValue(defaultValue)
		{}

		tField()
			: m_id(), m_name(), m_type(), m_fieldReferenceId(), m_fieldReference(), m_defaultValue()
		{}

		void setListPtr(SafePtr<UserList> userList)
		{
			m_fieldReference = userList;
		}

		bool operator== (tField t1) const
		{
			return this->m_id == t1.m_id && 
				this->m_name == t1.m_name && 
				this->m_type == t1.m_type && 
				(this->m_fieldReference == t1.m_fieldReference || this->m_fieldReferenceId == t1.m_fieldReferenceId) && 
				this->m_defaultValue == t1.m_defaultValue;
		}
	};

	class TagTemplate
	{
	public:
		TagTemplate();
		TagTemplate(templateId id, std::wstring name);
		TagTemplate(std::wstring name);
		TagTemplate(const TagTemplate& temp);

		void setId(const templateId& id);
		std::wstring renameTagTemplate(std::wstring newName);

		void mergeTemplate(TagTemplate tmp);

		// Using this method means that you already have generated/writen the field id
		// Using this method will NOT alterate the templateId
		void addNewField(tField newField);

		//Using this method will alterate the templateId
		void addNewField(std::wstring name, tFieldType type);

		std::wstring modifyFieldName(tFieldId idToModify, std::wstring newName);
		//Using this method will alterate the templateId
		void modifyFieldType(tFieldId idToModify, tFieldType newType);
		//Using this method will alterate the templateId
		void modifyFieldReference(tFieldId idToModify, SafePtr<UserList> newfParam);
		std::wstring modifyFieldDefaultValue(tFieldId idToModify, std::wstring newValue);

		//Using this method will alterate the templateId
		void removeField(tFieldId id);

		//Using this method will alterate the templateId
		void clearFields();

		void setOriginTemplate(bool value);

		const std::wstring& getName() const;
		bool isAOriginTemplate() const;
		bool isValid() const;
		templateId getId() const;
		std::vector<tField> getFieldsCopy() const;
		const std::unordered_map<tFieldId, tField>& getFields() const;
		int getFieldSize();

		bool operator==(const TagTemplate& rhs) const;
		bool operator<(const TagTemplate& rhs) const;
		size_t operator()(const TagTemplate& auth) const;

	private:
		bool m_valid;
		bool m_origin = false;

		std::wstring m_templateName;
		templateId m_id;

		std::list<tFieldId> m_fieldOrder;
		std::unordered_map<tFieldId, tField> m_fields;
	};

	std::vector<TagTemplate> GenerateDefaultTemplates(LanguageType lang);
}

namespace std
{
	template<>
	struct hash<sma::TagTemplate>
	{
		std::size_t operator()(sma::TagTemplate const& temp) const
		{
			return std::hash<xg::Guid>()(temp.getId());
		}
	};
}

#endif // !TAGTEMPLATE_H_
