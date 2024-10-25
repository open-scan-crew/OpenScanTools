#ifndef SCANTRA_INTERFACE_H
#define SCANTRA_INTERFACE_H


#include "../../Scantra/TestScantraInterprocess/Test_Frank/ScantraInterprocessMemory.h"

#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>

#include <thread>

//class Controller;
class IDataDispatcher;
class GraphManager;

class ScantraInterface
{
public:
    ScantraInterface(IDataDispatcher& data_dispatcher, GraphManager& graph);
    ~ScantraInterface();

    int startInterface();
    int stopInterface();

private:
    void run();

    void editStationChanged();
    void editIntersectionPlane();
    void editStationColor();

private:
    // interprocess memory
    boost::interprocess::windows_shared_memory shm_;
    boost::interprocess::mapped_region* region_ = nullptr;

    ScantraInterprocessMemory* data_ = nullptr;
    ScantraInterprocessObserver* observer_ = nullptr;

    std::atomic<bool> done_ = false;
    std::thread thread_;

    // OpenScanTools accessors
    //Controller& controller_;
    IDataDispatcher& data_dispatcher_;
    GraphManager& graph_;
};

#endif