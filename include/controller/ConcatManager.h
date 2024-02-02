#ifndef _DESCRIPTIONHISTORICALCONTROLLER_H_
#define _DESCRIPTIONHISTORICALCONTROLLER_H_

#include "controls/IControl.h"
#include "models/ScansapModelEssentials.h"

#include <map> 
#include <string>
#include <list>
#include <chrono>

class ConcatManager
{
	public:
		ConcatManager(std::list<IControl*>* historicptr, const std::chrono::seconds& maxDuration=std::chrono::seconds(5), const std::string& concatRule=std::string(". ;\n"));
		~ConcatManager();

		bool manageConcat(const UIControl& type, const dataId& elementId, const std::string& newStr);

		void setMaxConcatDuration(const std::chrono::seconds& maxDuration);
		void setConcatRule(const std::string& concatRule);

		const std::chrono::seconds& getMaxDuration() const;
		const std::string& getConcatRule() const;

	private:
		bool isValidToConcat(const std::string& str);

		template<class T>
		bool concat(T* oldEvent, const dataId& elementId, const std::string& newEvent);

	private:
		std::list<IControl*>* historic;
		std::chrono::steady_clock::time_point m_timer;
		std::chrono::seconds m_maxConcatDuration;
		std::string m_concatRule;
};

template<class T>
bool ConcatManager::concat(T* oldEvent, const dataId& elementId, const std::string& newEvent)
{
	dataId id(oldEvent->getId());
	bool idb = elementId == oldEvent->getId();
	bool secs = (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_timer) < m_maxConcatDuration);
	if ((elementId == oldEvent->getId()) && secs && isValidToConcat(newEvent))
	{
		oldEvent->setNewDesc(newEvent);
		return true;
	}
	m_timer = std::chrono::steady_clock::now();
	return false;
}

#endif // !_DESCRIPTIONHISTORICALCONTROLLER_H_
