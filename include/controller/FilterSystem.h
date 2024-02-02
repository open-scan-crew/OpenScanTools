#ifndef FILTER_SYSTEM_CONTROLLER_H
#define FILTER_SYSTEM_CONTROLLER_H

#include "models/data/Data.h"
#include "utils/Color32.hpp"
#include "models/project/Marker.h"

#include <list>
#include <vector>
#include <map>
#include <set>

#include <unordered_set>

class FilterSystem;
class AObjectNode;

typedef bool (FilterSystem::*filterMethod)(SafePtr<AObjectNode>);

class FilterSystem
{
public:
	FilterSystem();
	~FilterSystem();

	void setKeywordFilterStatus(const bool& status);
	void applyNewKeyWord(const std::wstring& keyword);

	void removeTypeSearched(const ElementType& type);
	void addTypeSearched(const ElementType& type);

	bool addColorFilter(const Color32& color);
	bool removeColorFilter(const Color32& color);
	void clearColorFilter();

	void setIconFilterStatus(const bool& status);
	bool addIconFilter(const scs::MarkerIcon& icon);
	bool removeIconFilter(const scs::MarkerIcon& icon);
	void clearIconFilter();

	void setUserFilterStatus(const bool& status);
    bool getUserFilterStatus();
	void setUserFilter(const xg::Guid& authorId);

	void setDisciplineFilterStatus(const bool& status);
    bool getDisciplineFilterStatus();
	void setDisciplineFilter(const std::wstring& filter);

	void setPhaseFilterStatus(const bool& status);
    bool getPhaseFilterStatus();
	void setPhaseFilter(const std::wstring& filter);

	void setTimeFilterStatus(const bool& status);
	bool setMinTime(const time_t& time);
	bool setMaxTime(const time_t& time);
	void invalidTimeFilter();

	bool filter(SafePtr<AObjectNode> data);

	bool filterAdvTag(SafePtr<AObjectNode> tag);
	bool filterScan(SafePtr<AObjectNode> scan);
	bool filterGeneric(SafePtr<AObjectNode> data);

private:
	bool m_timeFilter;
	time_t m_minTime;
	time_t m_maxTime;

	bool m_isKeywordFiltered;
	std::list<std::wstring> m_filterKeywords;

	bool m_isDisciplineFiltered;
	std::wstring m_discipline;

	bool m_isPhaseFiltered;
	std::wstring m_phase;

	bool m_isUserFiltered;
	xg::Guid m_authorId;

	// true for enable
	bool m_iconFilterStatus;
	std::set<scs::MarkerIcon> m_iconFiltered;

	std::unordered_set<ElementType> m_typeSearched;

	std::set<Color32> m_colorFilter;

	std::unordered_map<ElementType, filterMethod> m_methods;
};

#endif // !FILTER_SYSTEM_CONTROLLER_H_
