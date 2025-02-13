#include "Controller/Controller_p.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataTree.h"
#include "utils/Logger.h"

#include <chrono>

Controller_p::Controller_p(IDataDispatcher& dataDispatcher, GraphManager& graphManager, Controller& publicController)
    : clock_lock_(1)
    , dataDispatcher(dataDispatcher)
    , graphManager(graphManager)
    , public_controller_(publicController)
    , controlListener()
    , autosave_active_(false)
    , autosave_period_min_(5)
    , autosave_tp_(std::chrono::system_clock::now())
    , scantra_interface_(publicController, dataDispatcher, graphManager)
{
    // Launch the controller in a separate thread with a fixed framerate
    main_thread_ = std::thread(&Controller_p::run, this, 60);
}

Controller_p::~Controller_p()
{
    stop();
    resetHistoric();
    main_thread_.join();
}

void Controller_p::run(int refreshPerSecond)
{
    // Guaranted to start the controller only once
    bool expected_value = false;
    if (thread_working_.compare_exchange_strong(expected_value, true) == false)
        return;

    clock_lock_.resetClock(refreshPerSecond);
    continue_run_.store(true);

    while (continue_run_.load() == true)
    {
        update();

        clock_lock_.frame();
    }

    thread_working_.store(false);

    return;
}

void Controller_p::stop()
{
    continue_run_.store(false);
}

void Controller_p::undoLastControl()
{
    if (!meta_control_creation_)
    {
        if (to_undo_.empty())
        {
            dataDispatcher.updateInformation(new GuiDataUndoRedoAble(false, !to_redo_.empty()));
            return;
        }
        AControl* toUndoControl = to_undo_.top();
        to_undo_.pop();
        toUndoControl->undoFunction(public_controller_);
        to_redo_.push(toUndoControl);
        dataDispatcher.updateInformation(new GuiDataUndoRedoAble(!to_undo_.empty(), true));
    }
}

void Controller_p::redoLastControl()
{
    if (!meta_control_creation_)
    {
        if (to_redo_.empty())
        {
            dataDispatcher.updateInformation(new GuiDataUndoRedoAble(!to_undo_.empty(), false));
            return;
        }
        AControl* toRedoControl = to_redo_.top();
        to_redo_.pop();
        toRedoControl->redoFunction(public_controller_);
        to_undo_.push(toRedoControl);
        dataDispatcher.updateInformation(new GuiDataUndoRedoAble(true, !to_redo_.empty()));
    }
}

void Controller_p::activateAutosave(const uint64_t& period_min)
{
    autosave_active_ = true;
    autosave_period_min_ = period_min == 0 ? 5 : period_min;
}

void Controller_p::deactivateAutosave()
{
    autosave_active_ = false;
}

void Controller_p::autosave()
{
    if (!autosave_active_)
        return;
    int mins_elapsed = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::system_clock::now() - autosave_tp_).count();
    if (mins_elapsed > autosave_period_min_)
    {
        funcManager.launchBackgroundFunction(public_controller_, ContextType::saveProject, 0);
        autosave_tp_ = std::chrono::system_clock::now();
    }
}

void Controller_p::addTreeViewActualization(const std::unordered_set<SafePtr<AGraphNode>>& toActualizeDatas)
{
    pending_tree_actualization_.insert(toActualizeDatas.begin(), toActualizeDatas.end());
}

void Controller_p::update()
{
    autosave();
    updateControls();
    applyWaitingActualize();
    funcManager.updateContexts(public_controller_);
}

bool Controller_p::updateControls()
{
    std::list<AControl*> events = controlListener.popBlockControls();

    if (events.size() > 0)
    {
        for (AControl* actualControl : events)
        {
            actualControl->doFunction(public_controller_);
            if (actualControl->canUndo() == false)
                delete (actualControl);
            else
            {
                context.setIsCurrentProjectSaved(false);

                if (meta_control_creation_)
                    meta_undo_.push_front(actualControl);
                else
                    to_undo_.push(actualControl);
                cleanRedoStack();
                dataDispatcher.updateInformation(new GuiDataUndoRedoAble(!to_undo_.empty(), !to_redo_.empty()));
            }
        }
        return (true);
    }
    return (false);
}

void Controller_p::applyWaitingActualize()
{
    // Actualize Properties
    if (!pending_tree_actualization_.empty())
    {
        dataDispatcher.updateInformation(new  GuiDataTreeActualizeNodes(pending_tree_actualization_));
        dataDispatcher.updateInformation(new GuiDataObjectSelected());
        pending_tree_actualization_.clear();
    }
}

void Controller_p::logEndStats()
{
    Logger::log(LoggerMode::LogConfig) << "Nbr frame       : " << clock_lock_.getNbrFrame() << LOGENDL;
    Logger::log(LoggerMode::LogConfig) << "tt Time elapsed : " << std::setprecision(6) << clock_lock_.getTotalTimeSeconds() << LOGENDL;
    Logger::log(LoggerMode::LogConfig) << "Worked seconds  : " << std::setprecision(6) << clock_lock_.getSecondWorked() << LOGENDL;
    Logger::log(LoggerMode::LogConfig) << "Sleeped seconds : " << std::setprecision(6) << clock_lock_.getSecondsSleeped() << LOGENDL;
    Logger::log(LoggerMode::LogConfig) << "Average time(%) worked per frame : " << std::setprecision(6) << clock_lock_.getSecondWorked() / clock_lock_.getTotalTimeSeconds() * 100.0 << "%" << LOGENDL;
}

void Controller_p::resetHistoric()
{
    while (!to_undo_.empty())
    {
        delete(to_undo_.top());
        to_undo_.pop();
    }
    cleanRedoStack();
}

void Controller_p::cleanRedoStack()
{
    while (!to_redo_.empty())
    {
        delete(to_redo_.top());
        to_redo_.pop();
    }
}
