#ifndef _SIGNALHANDLER_H_
#define _SIGNALHANDLER_H_
#include "gui/IDataDispatcher.h"

#include <mutex>
#include <thread>

class SignalHandler
{
public:
	SignalHandler(IDataDispatcher* dataDispatcher);
	~SignalHandler();
	void registerSignals();
	void handleSignal(int signal);
	bool isRunning();
	void stopRunning();
private:
	IDataDispatcher* m_dataDispatcher;
	std::mutex m_dataDispatcherMutex;
	std::thread m_checker;
	bool m_running;
};

#endif 
