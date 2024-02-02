#ifndef FUNCTION_MANAGER_H_
#define FUNCTION_MANAGER_H_

#include <list>
#include <unordered_map>
#include <string>
#include <mutex>

#include "controller/functionSystem/ContextFactory.h"

#include "utils/IdGiver.hpp"

/*
struct FunctionData
{
	ContextId id;
	std::string name;
	ContextState state;
};*/

class FunctionManager
{
public:
	FunctionManager();
	~FunctionManager();

	ContextId launchFunction(Controller& controller, const ContextType& type);
	ContextId launchBackgroundFunction(Controller& controller, const ContextType& type, const ContextId& parentId);
	ContextType isActiveContext();

	void feedMessage(Controller& controller, IMessage* message);
	bool feedMessageToSpecificContext(Controller& controller, IMessage* message, const ContextType& type);
	bool feedMessageToSpecificContext(Controller& controller, IMessage* message, const ContextId& id);
	bool feedResultToSpecificContext(Controller& controller, IMessage* message, const ContextId& id);

	void validate(Controller& controller);
	void abort(Controller& controller);

	void updateContexts(Controller& controller);
	AContext* getActiveContext();
	//methode pour virer un contexte

private:
	// WARINING - must not be used inside a mutex lock
	void killContext(Controller& controller, ContextId idToKill);
	AContext* getContext(ContextId id);
	void validate(Controller& controller, const ContextId& id);
	void abort(Controller& controller, const ContextId& id);

private:
	static IdGiver<ContextId> s_contextIdGiver;
	std::mutex m_mutex;
	ContextFactory m_cFactory;
	ContextId m_actualCId;
	ContextType m_lastFunction;
	std::unordered_map<ContextId, AContext*> m_contextList;
	std::unordered_map<ContextId, std::list<IMessage*>> m_contextMessages;
	std::unordered_map<ContextId, std::list<IMessage*>> m_contextResults;

	std::unordered_map<ContextId, std::thread*> m_threads;
	std::vector<ContextId> m_contextToValidate;
	std::vector<ContextId> m_contextToAbort;
};

#endif // !FUNCTION_MANAGER_H_