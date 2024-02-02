#include "controller/ConcatManager.h"
#include "controller/controls/ControlTagEdition.h"
#include "controller/controls/ControlScanEdition.h"
#include "utils/StrCompare.h"
#include <set>


ConcatManager::ConcatManager(std::list<IControl*>* historicptr, const std::chrono::seconds& maxDuration, const std::string& concatRule)
	: historic(historicptr), m_maxConcatDuration(maxDuration), m_concatRule(concatRule) {}

ConcatManager::~ConcatManager() {}

void ConcatManager::setMaxConcatDuration(const std::chrono::seconds& maxDuration)
{
	m_maxConcatDuration = maxDuration;
}

const std::chrono::seconds& ConcatManager::getMaxDuration() const
{
	return (m_maxConcatDuration);
}

void ConcatManager::setConcatRule(const std::string& concatRule)
{}

const std::string& ConcatManager::getConcatRule() const
{
	return (m_concatRule);
}

bool ConcatManager::manageConcat(const UIControl& type, const dataId& elementId, const std::string& newStr)
{
	if (historic->empty())
	{
		m_timer = std::chrono::steady_clock::now();
		return false;
	}
	IControl* eventControl = (*std::prev(historic->end()));
	if (eventControl->getType() == type && type == UIControl::setDescriptionTagEdit)
	{
		//FIXME
		/*control::tagEdition::SetDescription *eventDescn = static_cast<control::tagEdition::SetDescription*>(eventControl);
		return concat(eventDescn, elementId, newStr);*/
	}
	else if (eventControl->getType() == type && type == UIControl::setDescriptionScanEdit)
	{
		control::scanEdition::SetDescription *eventNamen = static_cast<control::scanEdition::SetDescription*>(eventControl);
		return concat(eventNamen, elementId, newStr);
	}
	return false;
}

bool ConcatManager::isValidToConcat(const std::string& str)
{
	if (str.empty())
		return false;
	return m_concatRule.find(str.at(str.size() - 1)) == std::string::npos;
}