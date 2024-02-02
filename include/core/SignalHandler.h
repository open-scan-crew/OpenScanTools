#ifndef _SIGNALHANDLER_H_
#define _SIGNALHANDLER_H_
#include "gui/DataDispatcher.h"
#include "controller/controls/ControlSignals.h"
#include <thread>


class SignalHandler
{
public:
	SignalHandler(DataDispatcher* dataDispatcher);
	~SignalHandler();
	void registerSignals();
	void handleSignal(int signal);
	bool isRunning();
	void stopRunning();
private:
	DataDispatcher* m_dataDispatcher;
	std::mutex m_dataDispatcherMutex;
	std::thread m_checker;
	bool m_running;
};

#endif 
