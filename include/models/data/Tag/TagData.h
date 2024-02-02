#ifndef TAGDATA_H_
#define TAGDATA_H_

#include "utils/Color32.hpp"
#include "models/project/Marker.h"
#include "models/application/TagTemplate.h"

class ControllerContext;

class TagData
{
public:
	TagData();
	TagData(const TagData& data);
	~TagData();

	void copyTagData(const TagData& data);

	void setDefaultData(const ControllerContext& context);

	virtual void setMarkerIcon(scs::MarkerIcon icon);
	virtual void setValue(sma::tFieldId id, std::wstring newValue);
	virtual void setFields(const std::unordered_map<sma::tFieldId, std::wstring>& fields);
	virtual void setTemplate(const SafePtr<sma::TagTemplate>& templateId);
	virtual void addField(sma::tFieldId id, std::wstring newValue);
	virtual void removeField(sma::tFieldId id);

	SafePtr<sma::TagTemplate> getTemplate() const;
	const scs::MarkerIcon& getMarkerIcon() const;
	std::wstring getValue(sma::tFieldId id) const;
	const std::unordered_map<sma::tFieldId, std::wstring>& getFields() const;

protected:
	SafePtr<sma::TagTemplate> m_template = SafePtr<sma::TagTemplate>();
	scs::MarkerIcon m_markerIcon = scs::MarkerIcon::Tag_Base;

	std::unordered_map<sma::tFieldId, std::wstring> m_fields;
};

#endif // !UIDATA_H_