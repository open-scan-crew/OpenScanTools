#ifndef SCANTRA_INTERFACE_H
#define SCANTRA_INTERFACE_H

#include "scantra/ScantraInterprocessMemory.h"

#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>

#include "utils/safe_ptr.h"

#include <thread>
#include <unordered_set>

class Controller;
class IDataDispatcher;
class GraphManager;
class AGraphNode;


class ScantraInterface
{
public:
    ScantraInterface(Controller& controller, IDataDispatcher& data_dispatcher, GraphManager& graph);
    ~ScantraInterface();

    int startInterface();
    int stopInterface();

private:
    void run();

    void editStationChanged();
    void editIntersectionPlane();
    void editStationColor();
    void editStationAdjustment();

    void manageVisibility(int current_idx, int total_station, SafePtr<AGraphNode> scan);

private:
    // interprocess memory
    boost::interprocess::windows_shared_memory shm_;
    boost::interprocess::mapped_region* region_ = nullptr;

    ScantraInterprocessMemory* data_ = nullptr;
    ScantraInterprocessObserver* observer_ = nullptr;

    std::atomic<bool> done_ = false;
    std::thread thread_;

    // OpenScanTools accessors
    Controller& controller_;
    IDataDispatcher& data_dispatcher_;

    std::unordered_set<SafePtr<AGraphNode>> scan_selection_;
    GraphManager& graph_;
};

#endif