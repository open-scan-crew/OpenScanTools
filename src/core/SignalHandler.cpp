#include "core/SignalHandler.h"
#include "controller/controls/ControlSignals.h"
#include "utils/Logger.h"
#include "Baduit/Timer.hpp"

#include <csignal>

volatile std::sig_atomic_t gSignalStatus;

void handle(int signal)
{
	Logger::log(LoggerMode::ControllerLog) << "SIG raise: " << signal << LOGENDL;
	gSignalStatus = signal;
}

SignalHandler::SignalHandler(IDataDispatcher* dataDispatcher) : m_dataDispatcher(dataDispatcher), m_running(false)
{
	registerSignals();
}

SignalHandler::~SignalHandler()
{
	stopRunning();
}

void SignalHandler::registerSignals()
{
	if(!m_running)
	{
		std::signal(SIGINT, handle);
		std::signal(SIGTERM, handle);
		std::signal(SIGABRT, handle);
		std::signal(SIGFPE, handle);
		std::signal(SIGILL, handle);
		std::signal(SIGSEGV, handle);
		m_running = true;
		m_checker = std::thread([this]() { Baduit_Timer::Sleeper sleeper(std::chrono::milliseconds(50)); while (this->isRunning()) { if (gSignalStatus) { this->handleSignal(gSignalStatus); gSignalStatus = 0; } sleeper.sleep(std::chrono::milliseconds(50)); }});
	}
}

void SignalHandler::handleSignal(int signal)
{
	std::lock_guard<std::mutex> locker(m_dataDispatcherMutex);
	switch (signal)
	{
		case SIGINT:
			m_dataDispatcher->sendControl(new control::signalHandling::SigInt());
			break;
		case SIGTERM:
			m_dataDispatcher->sendControl(new control::signalHandling::SigTerm());
			break;
		case SIGABRT:
			m_dataDispatcher->sendControl(new control::signalHandling::SigAbrt());
			break;
		case SIGFPE:
			m_dataDispatcher->sendControl(new control::signalHandling::SigFpe());
			break;
		case SIGILL:
			m_dataDispatcher->sendControl(new control::signalHandling::SigIll());
			break;
		case SIGSEGV:
			m_dataDispatcher->sendControl(new control::signalHandling::SigSegv());
			break;
	}
}

bool SignalHandler::isRunning()
{
	return m_running;
}

void  SignalHandler::stopRunning()
{
	m_running = false;
	m_checker.join();
}