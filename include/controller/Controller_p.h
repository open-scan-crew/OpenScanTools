#ifndef CONTROLLER_P_H
#define CONTROLLER_P_H

#include <stack>
#include <atomic>

#include "gui/IDataDispatcher.h"
#include "controller/ControlListener.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/FilterSystem.h"
#include "models/3d/Graph/OpenScanToolsGraphManager.h"
#include "utils/ClockLock.hpp"

class Controller_p
{
public:
    Controller_p(IDataDispatcher& dataDispatcher, OpenScanToolsGraphManager& graphManager);
    ~Controller_p();

    bool startAutosaveThread(const uint64_t& timing, Controller& controller);
    bool stopAutosaveThread();

public:
    // Controller run resources
    std::atomic<bool> continueRun = true;
    std::atomic<bool> threadWorking = false;
    ClockLock clockLock;


    std::stack<AControl*> toUndo;
    std::stack<AControl*> toRedo;

    IDataDispatcher& dataDispatcher;
    OpenScanToolsGraphManager& graphManager;

    ControlListener controlListener;
    FunctionManager funcManager;
    ControllerContext context;
    FilterSystem filterSystem;
private:
    bool                    m_autoSaveThreadRunning;
    std::thread*            m_autosaveThread;
};

#endif