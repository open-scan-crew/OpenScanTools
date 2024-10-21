#include "controller/ControlListener.h"
//#include "controller/FilteredFonctionnalities.h"


ControlListener::ControlListener()
{
}

ControlListener::~ControlListener()
{
	m_listenerMutex.lock();

	for (AControl* ctrl : m_eventQueue)
		delete (ctrl);

	m_eventQueue.clear();
	m_listenerMutex.unlock();
}

void ControlListener::notifyUIControl(AControl *event)
{
	m_listenerMutex.lock();

	m_eventQueue.push_back(event);

	m_listenerMutex.unlock();
}

std::list<AControl*> ControlListener::popBlockControls()
{
	std::list<AControl*> list;
	m_listenerMutex.lock();
	list.insert(list.begin(), m_eventQueue.begin(), m_eventQueue.end());
	m_eventQueue.clear();
	m_listenerMutex.unlock();
	return (list);
}

/*
bool ControlListener::isControlFiltred(const ControlType& type) const
{
	return false;
	if (FilteredFonctionnalitesMap.find(type) == FilteredFonctionnalitesMap.end())
	{
		assert(false); //ControlType don't have associated licence permission (Free, Lite or Standard) or is null
		return true;
	}

	LicenceVersion licVersion = getLicenseVersion();
	std::unordered_set<LicenceVersion> currentContentVersion = { licVersion };
	if (AssociatedLicenceVersionMap.find(licVersion) != AssociatedLicenceVersionMap.end())
	{
		const std::unordered_set<LicenceVersion>& associatedVersion = AssociatedLicenceVersionMap.at(licVersion);
		currentContentVersion.insert(associatedVersion.begin(), associatedVersion.end());
	}

	for (LicenceVersion v : currentContentVersion)
		if (FilteredFonctionnalitesMap.at(type).find(v) != FilteredFonctionnalitesMap.at(type).end())
			return false;
			

	return true;
}*/