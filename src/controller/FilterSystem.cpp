#include "controller/FilterSystem.h"
#include "utils/Logger.h"
#include "models/graph/AGraphNode.h"
#include "models/graph/TagNode.h"
#include "models/graph/PointCloudNode.h"
#include "models/application/Author.h"

#include "magic_enum/magic_enum.hpp"
#include <string>

FilterSystem::FilterSystem()
	: m_isKeywordFiltered(false)
	, m_isDisciplineFiltered(false)
	, m_isPhaseFiltered(false)
	, m_isUserFiltered(false)
	, m_iconFilterStatus(false)
	, m_timeFilter(false)
	, m_minTime(-1)
	, m_maxTime(-1)
{
	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::Tag, &FilterSystem::filterAdvTag));
	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::Scan, &FilterSystem::filterScan));
	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::Box, &FilterSystem::filterGeneric));
	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::Sphere, &FilterSystem::filterGeneric));
	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::Cylinder, &FilterSystem::filterGeneric));
	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::Torus, &FilterSystem::filterGeneric));
	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::Piping, &FilterSystem::filterGeneric));
	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::Point, &FilterSystem::filterGeneric));
	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::MeshObject, &FilterSystem::filterGeneric));
	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::PCO, &FilterSystem::filterGeneric));

	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::SimpleMeasure, &FilterSystem::filterGeneric));
	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::PolylineMeasure, &FilterSystem::filterGeneric));
	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::BeamBendingMeasure, &FilterSystem::filterGeneric));
	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::ColumnTiltMeasure, &FilterSystem::filterGeneric));
	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::PointToPipeMeasure, &FilterSystem::filterGeneric));
	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::PointToPlaneMeasure, &FilterSystem::filterGeneric));
	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::PipeToPipeMeasure, &FilterSystem::filterGeneric));
	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::PipeToPlaneMeasure, &FilterSystem::filterGeneric));

	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::Cluster, &FilterSystem::filterGeneric));
	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::MasterCluster, &FilterSystem::filterGeneric));

	m_methods.insert(std::pair<ElementType, filterMethod>(ElementType::ViewPoint, &FilterSystem::filterGeneric));
}

FilterSystem::~FilterSystem()
{
}

void FilterSystem::setKeywordFilterStatus(const bool& status)
{
	m_isKeywordFiltered = status;
}


void FilterSystem::applyNewKeyWord(const std::wstring& keyword)
{
	std::list<std::wstring> list;
	std::wstring tmp;
	std::wstring::const_iterator it = keyword.begin();
	std::wstring sepList = L";";

	while (it != keyword.end())
	{
		if (sepList.find(*it) != std::wstring::npos)
		{
			if (tmp.size() == 0)
			{
				it++;
				continue;
			}
			list.push_back(tmp);
			tmp.clear();
		}
		else
			tmp += *it;
		it++;
		if (it == keyword.end() && tmp.size() > 0)
			list.push_back(tmp);
	}

	m_filterKeywords = list;
}

void FilterSystem::removeTypeSearched(const ElementType& type)
{
	CONTROLLOG << "remove searched type " << magic_enum::enum_name<ElementType>(type) << LOGENDL;
	m_typeSearched.erase(type);
}

void FilterSystem::addTypeSearched(const ElementType& type)
{
	CONTROLLOG << "attached searched type " << magic_enum::enum_name<ElementType>(type) << LOGENDL;
	m_typeSearched.insert(type);
}

bool FilterSystem::addColorFilter(const Color32& color)
{
	if (m_colorFilter.find(color) == m_colorFilter.end())
	{
		m_colorFilter.insert(color);
		return (true);
	}
	return (false);
}

bool FilterSystem::removeColorFilter(const Color32& color)
{
	if (m_colorFilter.find(color) != m_colorFilter.end())
	{
		m_colorFilter.erase(color);
		return (true);
	}
	return (false);
}

void FilterSystem::clearColorFilter()
{
	m_colorFilter.clear();
}

void FilterSystem::setIconFilterStatus(const bool& status)
{
	m_iconFilterStatus = status;
}

bool FilterSystem::addIconFilter(const scs::MarkerIcon& icon)
{
	if (m_iconFiltered.find(icon) == m_iconFiltered.end())
	{
		m_iconFiltered.insert(icon);
		return (true);
	}
	return (false);
}

bool FilterSystem::removeIconFilter(const scs::MarkerIcon& icon)
{
	if (m_iconFiltered.find(icon) != m_iconFiltered.end())
	{
		m_iconFiltered.erase(icon);
		return (true);
	}
	return (false);
}

void FilterSystem::clearIconFilter()
{
	m_iconFiltered.clear();
}

std::wstring lowerString(std::wstring origin)
{
	std::for_each(origin.begin(), origin.end(), [](wchar_t& c) {
		c = ::tolower(c);
		});
	return (origin);
}

void FilterSystem::setUserFilterStatus(const bool& status)
{
	m_isUserFiltered = status;
}

bool FilterSystem::getUserFilterStatus()
{
    return m_isUserFiltered;
}

void FilterSystem::setUserFilter(const xg::Guid& authorId)
{
	m_authorId = authorId;
}

void FilterSystem::setDisciplineFilterStatus(const bool& status)
{
	m_isDisciplineFiltered = status;
}

bool FilterSystem::getDisciplineFilterStatus()
{
    return m_isDisciplineFiltered;
}

void FilterSystem::setDisciplineFilter(const std::wstring& filter)
{
	m_discipline = filter;
}

void FilterSystem::setPhaseFilterStatus(const bool& status)
{
	m_isPhaseFiltered = status;
}

bool FilterSystem::getPhaseFilterStatus()
{
    return m_isPhaseFiltered;
}

void FilterSystem::setPhaseFilter(const std::wstring& filter)
{
	m_phase = filter;
}

void FilterSystem::setTimeFilterStatus(const bool& status)
{
	m_timeFilter = status;
	/*if (status && m_minTime < m_maxTime && m_minTime != 0 && m_maxTime != 0)
		m_timeFilter = true;
	else
		m_timeFilter = false;*/
}

bool FilterSystem::setMinTime(const time_t& time)
{
	m_minTime = time; 
	if (m_minTime < m_maxTime && m_minTime != 0 && m_maxTime != 0)
		return true;
	/*else
		m_timeFilter = false;*/
	return (false);
}

bool FilterSystem::setMaxTime(const time_t& time)
{
	m_maxTime = time;
	if (m_minTime < m_maxTime && m_minTime != 0 && m_maxTime != 0)
		return true;
	/*else
		m_timeFilter = false;*/
	return (false);
}

void FilterSystem::invalidTimeFilter()
{
	m_timeFilter = false;
}

bool checkAdvTagCustomField(const std::unordered_map<sma::tFieldId, std::wstring>& fields, const std::wstring& keyword)
{
	for (auto it = fields.cbegin(); it != fields.cend(); it++)
		if (lowerString(it->second).find(keyword) != std::wstring::npos)
			return (true);
	return (false);
}

xg::Guid getAuthorId(SafePtr<Author> auth)
{
	ReadPtr<Author> rAuth = auth.cget();
	if (!rAuth)
		return xg::Guid();
	return rAuth->getId();
}

bool FilterSystem::filterAdvTag(SafePtr<AGraphNode> marker)
{
	ReadPtr<TagNode> rTag = static_pointer_cast<TagNode>(marker).cget();
	if (!rTag)
		return false;

	if (m_typeSearched.find(ElementType::Tag) == m_typeSearched.end())
		return (false);
	if (m_colorFilter.size() != 0 && m_colorFilter.find(rTag->getColor()) == m_colorFilter.end())
		return (false);
	if (m_timeFilter == true && (m_minTime < m_maxTime && m_minTime != 0 && m_maxTime != 0) && (m_minTime > rTag->getCreationTime() || rTag->getCreationTime() > m_maxTime))
		return (false);
	if (m_isDisciplineFiltered == true && rTag->getDiscipline() != m_discipline)
		return (false);
	if (m_isPhaseFiltered == true && rTag->getPhase() != m_phase)
		return (false);
	if (m_isUserFiltered == true && getAuthorId(rTag->getAuthor()) != m_authorId)
		return (false);
	if (m_iconFilterStatus == true && m_iconFiltered.find(rTag->getMarkerIcon()) == m_iconFiltered.end())
		return (false);
	if (m_isKeywordFiltered == true)
		for (auto it = m_filterKeywords.begin(); it != m_filterKeywords.end(); it++)
		{
			std::wstring compareStr = lowerString(*it);
			if (lowerString(rTag->getDescription()).find(compareStr) == std::string::npos
				&& lowerString(rTag->getName()).find(compareStr) == std::string::npos
				&& lowerString(rTag->getDiscipline()).find(compareStr) == std::string::npos
				&& lowerString(rTag->getPhase()).find(compareStr) == std::string::npos
				&& lowerString(rTag->getIdentifier()).find(compareStr) == std::string::npos
				&& checkAdvTagCustomField(rTag->getFields(), compareStr) == false)
				return (false);
		}

	return (true);
}

bool FilterSystem::filterScan(SafePtr<AGraphNode> element)
{
	ReadPtr<PointCloudNode> rScan = static_pointer_cast<PointCloudNode>(element).cget();
	if (!rScan)
		return false;

	if (m_typeSearched.find(ElementType::Scan) == m_typeSearched.end())
		return (false);
	if (m_timeFilter == true && (m_minTime > rScan->getCreationTime() || rScan->getCreationTime() > m_maxTime))
		return (false);
	if (m_isDisciplineFiltered == true && rScan->getDiscipline() != m_discipline)
		return (false);
	if (m_isPhaseFiltered == true && rScan->getPhase() != m_phase)
		return (false);
	/*if (m_isUserFiltered == true && element->getAuthor() != m_user)
		return (false);*/
	if (m_isKeywordFiltered == true)
		for (auto it = m_filterKeywords.begin(); it != m_filterKeywords.end(); it++)
		{
			std::wstring compareStr = lowerString(*it);
			if (lowerString(rScan->getDescription()).find(compareStr) == std::wstring::npos
				&& lowerString(rScan->getName()).find(compareStr) == std::wstring::npos
				&& lowerString(rScan->getSensorModel()).find(compareStr) == std::wstring::npos
				&& lowerString(rScan->getSensorSerialNumber()).find(compareStr) == std::wstring::npos
				&& lowerString(rScan->getStringAcquisitionTime()).find(compareStr) == std::wstring::npos)
				return (false);
		}
	return (true);
}

bool FilterSystem::filterGeneric(SafePtr<AGraphNode> data)
{
	ReadPtr<AGraphNode> rData = static_pointer_cast<AGraphNode>(data).cget();
	if (!rData)
		return false;

	if (m_typeSearched.find(rData->getType()) == m_typeSearched.end())
		return (false);
	if (m_timeFilter == true && (m_minTime > rData->getCreationTime() || rData->getCreationTime() > m_maxTime))
		return (false);
	if (m_isDisciplineFiltered == true && rData->getDiscipline() != m_discipline)
		return (false);
	if (m_isPhaseFiltered == true && rData->getPhase() != m_phase)
		return (false);
	if (m_isUserFiltered == true && getAuthorId(rData->getAuthor()) != m_authorId)
		return (false);
	if (m_colorFilter.size() != 0 && m_colorFilter.find(rData->getColor()) == m_colorFilter.end())
		return (false);
	if (m_isKeywordFiltered == true)
		for (auto it = m_filterKeywords.begin(); it != m_filterKeywords.end(); it++)
		{
			std::wstring compareStr = lowerString(*it);
			if (lowerString(rData->getDescription()).find(compareStr) == std::wstring::npos
				&& lowerString(rData->getDiscipline()).find(compareStr) == std::wstring::npos
				&& lowerString(rData->getPhase()).find(compareStr) == std::wstring::npos
				&& lowerString(rData->getIdentifier()).find(compareStr) == std::wstring::npos
				&& lowerString(rData->getName()).find(compareStr) == std::wstring::npos)
				return (false);
		}
	return (true);
}

bool FilterSystem::filter(SafePtr<AGraphNode> data)
{
	assert(data);
	ElementType type;
	{
		ReadPtr<AGraphNode> rData = data.cget();
		if (!rData)
			return false;
		type = rData->getType();
	}

	if(m_methods.find(type) == m_methods.end())
		return false;
	filterMethod method = m_methods.at(type);
	bool toFilter = (this->*method)(data);

	return toFilter;
}
