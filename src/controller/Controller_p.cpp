#include "Controller/Controller_p.h"
#include <chrono>
#include <condition_variable>

Controller_p::Controller_p(IDataDispatcher& dataDispatcher, OpenScanToolsGraphManager& graphManager)
    : clockLock(1)
    , dataDispatcher(dataDispatcher)
	, graphManager(graphManager)
    , controlListener()
	, m_autosaveThread(nullptr)
	, m_autoSaveThreadRunning(false)
{
}

Controller_p::~Controller_p()
{
	if (m_autosaveThread)
		stopAutosaveThread();
}

bool Controller_p::startAutosaveThread(const uint64_t& timing, Controller& controller)
{
	if (!timing)
		return false;
	if (m_autosaveThread)
		stopAutosaveThread();
	m_autoSaveThreadRunning = true;
	m_autosaveThread = new std::thread([this, &controller, timing]()
		{
			while (m_autoSaveThreadRunning)
			{
				auto chrono = std::chrono::minutes(timing);
				bool sleep(true);
				while (sleep)
				{
					auto microSleep = std::chrono::microseconds(100);
					std::this_thread::sleep_for(microSleep);
					chrono -= std::chrono::duration_cast<std::chrono::minutes>(microSleep);
					if (chrono.count() <= 0)
					{
						sleep = false;
						break;
					}
					sleep = m_autoSaveThreadRunning;
				}
				if (!m_autoSaveThreadRunning)
					return;
				funcManager.launchBackgroundFunction(controller, ContextType::autosaveProject, 0);
			}
		});
	return true;
}

bool Controller_p::stopAutosaveThread()
{
	if (!m_autosaveThread)
		return false;
	m_autoSaveThreadRunning = false;
	m_autosaveThread->join();
	delete m_autosaveThread;
	m_autosaveThread = nullptr;
	return true;
}