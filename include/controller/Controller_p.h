#ifndef CONTROLLER_P_H
#define CONTROLLER_P_H

#include <stack>
#include <atomic>
#include <thread>

#include "gui/IDataDispatcher.h"
#include "controller/ControlListener.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/FilterSystem.h"
#include "models/graph/GraphManager.h"
#include "utils/ClockLock.hpp"
#include "io/ScantraInterface.h"

class Controller_p
{
public:
    Controller_p(IDataDispatcher& dataDispatcher, GraphManager& graphManager, Controller& publicController);
    ~Controller_p();

    /*! \brief this function launch a thread that run the update() at a fixed refresh rate
     * Can only be called once per Controller instance.
     * The function stop() must be called before the Controller is destroyed.
     */
    void run(int refreshPerSecond);

    /*!
     * \brief this function stop the thread launched by run().
     * If there is no thread running, does nothing.
     */
    void stop();

    void undoLastControl();
    void redoLastControl();
    void resetHistoric();

    void activateAutosave(const uint64_t& timing);
    void deactivateAutosave();
    void autosave();

    void addTreeViewActualization(const std::unordered_set<SafePtr<AGraphNode>>& toActualizeDatas);

private:
    /*!
    ** \brief this method is the method that will allow all the subsystem of the controller to be updated
    ** This method must be called every frame.
    */
    void update();
    bool updateControls();

    void logEndStats();

    void cleanRedoStack();

    void applyWaitingActualize();

public:
    // Controller run resources
    std::atomic<bool> continue_run_ = true;
    std::atomic<bool> thread_working_ = false;
    ClockLock clock_lock_;

    std::thread main_thread_;

    // Control historic
    std::stack<AControl*> to_undo_;
    std::stack<AControl*> to_redo_;

    bool meta_control_creation_;
    std::list<AControl*> meta_undo_;

    // Waiting nodes to send to actualize the tree view
    std::unordered_set<SafePtr<AGraphNode>> pending_tree_actualization_;

    // Dedecated module classes
    IDataDispatcher& dataDispatcher;
    GraphManager& graphManager;
    Controller& public_controller_;

    ControlListener controlListener;
    FunctionManager funcManager;
    ControllerContext context;
    FilterSystem filterSystem;

    bool autosave_active_;
    double autosave_period_min_;
    std::chrono::system_clock::time_point autosave_tp_;
};

#endif