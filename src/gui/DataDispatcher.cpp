#include "gui/DataDispatcher.h"
#include "controller/controls/IControl.h"
#include "controller/IControlListener.h"
#include "utils/Logger.h"

#define DATALOG Logger::log(LoggerMode::DataLog)

DataDispatcher::DataDispatcher()
{
    m_controlListener = nullptr;

    timer.setInterval(20);
    timer.start();
    QObject::connect(&timer, &QTimer::timeout, this, &DataDispatcher::update);

	m_active = true;
}

DataDispatcher::~DataDispatcher()
{
    Logger::log(LoggerMode::LogConfig) << "Destroying DataDispatcher..." << LOGENDL;
}

//*** Data ***//
void DataDispatcher::updateInformation(IGuiData *value, IPanel* owner)
{
	if (m_active == false)
		return;

    std::lock_guard<std::mutex> lock(m_eventMutex);

	//Ajout dans la file (liste FIFO) le message IGuiData _value_ provenant de _owner_ (ne peut pas être reçu par _owner_)
	m_eventQueue.push({ value, owner });
}


void DataDispatcher::unregisterObserver(IPanel* panel)
{
    std::lock_guard<std::mutex> locker(m_observerMutex);

    for (auto it = m_dataKeyObservers.begin(); it != m_dataKeyObservers.end(); ++it)
        it->second.erase(panel);
}

void DataDispatcher::registerObserverOnKey(IPanel* panel, guiDType type)
{
    std::lock_guard<std::mutex> locker(m_observerMutex);

    if (m_dataKeyObservers.find(type) == m_dataKeyObservers.end())
        m_dataKeyObservers.insert({ type, std::set<IPanel*>() });

    if (m_dataKeyObservers.find(type)->second.find(panel) == m_dataKeyObservers.at(type).end())
        m_dataKeyObservers.at(type).insert(panel);
}

void DataDispatcher::unregisterObserverOnKey(IPanel* panel, guiDType type)
{
    std::lock_guard<std::mutex> locker(m_observerMutex);

    if (m_dataKeyObservers.empty())
        return;

    if (m_dataKeyObservers.find(type) != m_dataKeyObservers.end())
        m_dataKeyObservers.at(type).erase(panel);
}


//*** Controls ***///
void DataDispatcher::InitializeControlListener(IControlListener * listener)
{
    m_controlListener = listener;
}

void DataDispatcher::sendControl(AControl *control)
{
	if (m_active == false)
		return;
    if (m_controlListener != nullptr)
    {
        m_controlListener->notifyUIControl(control);
    }
    else
    {
        DATALOG << "Control Listener missing" << LOGENDL;
    }
}

void DataDispatcher::setActive(bool state)
{
	m_active = state;
}

void DataDispatcher::update()
{
	if (m_active == false)
		return;

    std::pair<IGuiData*,IPanel*> data;

    while (m_eventQueue.size() > 0)
    {
        {
            std::lock_guard<std::mutex> locker(m_eventMutex);
            data = m_eventQueue.front();
            m_eventQueue.pop();
        }

        std::unordered_map<guiDType, std::set<IPanel*>> observers;
        {
            std::lock_guard<std::mutex> locker(m_observerMutex);
            observers = m_dataKeyObservers;
        }

        if (observers.find(data.first->getType()) != observers.end())
        {
            for (auto it = observers.at(data.first->getType()).begin(); it != observers.at(data.first->getType()).end(); it++)
				if((*it) != data.second)
					(*it)->informData(data.first);
        }

        delete(data.first);
    }
}
