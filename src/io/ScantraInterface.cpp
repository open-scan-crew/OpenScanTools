#include "ScantraInterface.h"
#include "models/3d/Graph/OpenScanToolsGraphManager.h"
#include "models/3d/graph/ClusterNode.h"
#include "utils/Logger.h"

#include <wchar.h>

ScantraInterface::ScantraInterface(IDataDispatcher& data_dispatcher, OpenScanToolsGraphManager& graph)
    : data_dispatcher_(data_dispatcher)
    , graph_(graph)
{
}

ScantraInterface::~ScantraInterface()
{
    stopInterface();
}

int ScantraInterface::startInterface()
{
    if (data_)
    {
        Logger::log(LoggerMode::IOLog) << "Already connected to Scantra Interface." << Logger::endl;
        return -1;
    }

    // ----------------------------------
    // Creates or opens the shared memory
    // ----------------------------------
    bool exists = true;

    try
    {
        // Tries to open a native windows shared memory object.
        shm_ = windows_shared_memory(open_only, "ScantraInterprocessMemory", read_write);
    }
    catch (...)
    {
        exists = false;
    }

    if (!exists)
    {
        try
        {
            // Tries to create a native windows shared memory object.
            shm_ = windows_shared_memory(create_only, "ScantraInterprocessMemory", read_write, sizeof(ScantraInterprocessMemory));
        }
        catch (...)
        {
            Logger::log(LoggerMode::IOLog) << "Failed to create the Scantra Interface." << Logger::endl;
            return -2;
        }
    }

    // Map the whole shared memory in this process
    region_ = new mapped_region(shm_, read_write);

    // Get the address of the mapped region
    void* addr = region_->get_address();

    // Construct the shared structure in memory
    if (!exists)
        data_ = new (addr) ScantraInterprocessMemory;
    // Adopting the shared structure in memory
    else
        data_ = static_cast<ScantraInterprocessMemory*>(addr);

    // ---------------------
    // Register the observer
    // ---------------------
    observer_ = data_->registerObserver();

    if (!observer_)
    {
        Logger::log(LoggerMode::IOLog) << "Connection failed" << Logger::endl;
        return -3;
    }
    else if (data_->countObserver() == 1)
        Logger::log(LoggerMode::IOLog) << "No partner process available" << Logger::endl;
    else
    {
        Logger::log(LoggerMode::IOLog) << "Partner process available" << Logger::endl;

        {
            scoped_lock<interprocess_mutex> lock(data_->mutex);
            wcsncpy_s(data_->w_array[0], L"OpenScanTools", 256);
            observer_->message_ = ScantraInterprocessObserver::hasConnected;
        }
        data_->update(observer_);
    }

    observer_->shared_memory_ = data_;

    // -----------------------
    // Starts the worker tread
    // -----------------------
    thread_ = std::move(std::thread(&ScantraInterface::run, this));

    return 0;
}

int ScantraInterface::stopInterface()
{
    if (!data_)
        return -1;

    // Stops the worker thread
    done_ = true;
    {
        scoped_lock<interprocess_mutex> lock(data_->mutex);
        observer_->new_message_ = true;
    }
    observer_->cond_.notify_all();
    thread_.join();

    // Releases the shared memory when no partner processes are accessing it
    if (data_->countObserver() == 1)
        shared_memory_object::remove("ScantraInterfaceMemory");
    else
    {
        {
            scoped_lock<interprocess_mutex> lock(data_->mutex);
            wcsncpy_s(data_->w_array[0], L"OpenScanTools", 256);
            observer_->message_ = ScantraInterprocessObserver::hasDisconnected;
        }
        data_->update(observer_);
        data_->deregisterObserver(observer_);
    }

    observer_ = nullptr;
    data_ = nullptr;

    Logger::log(LoggerMode::IOLog) << "Disconnected" << Logger::endl;

    return 0;
}

void ScantraInterface::run()
{
    while (!done_)
    {
        scoped_lock<interprocess_mutex> lock(data_->mutex);
        observer_->cond_.wait(lock, [this] { return observer_->new_message_; });

        observer_->new_message_ = false;
        if (done_) return;

        int return_value = 0;

        SubLogger& log = Logger::log(LoggerMode::IOLog);

        switch (observer_->message_)
        {
        case ScantraInterprocessObserver::hasConnected:
        {
            std::wstring partner = data_->w_array[0];
            std::wstring text = L"Partner process has connected: " + partner;
            log << text << Logger::endl;
            break;
        }
        case ScantraInterprocessObserver::hasDisconnected:
        {
            std::wstring partner = data_->w_array[0];
            std::wstring text = L"Partner process has disconnected: " + partner;
            log << text << Logger::endl;
            break;
        }
        case ScantraInterprocessObserver::logText:
        {
            std::wstring text = data_->w_array[0];
            log << text << Logger::endl << Logger::endl;
            break;
        }
        case ScantraInterprocessObserver::stationChanged:
        {
            editStationChanged();
            break;
        }
        case ScantraInterprocessObserver::intersectionPlane:
        {
            editIntersectionPlane();
            break;
        }
        case ScantraInterprocessObserver::stationColor:
        {
            editStationColor();
            break;
        }
        default:
            break;
        }

        observer_->setReturn(return_value);
    }
}

void ScantraInterface::editStationChanged()
{
    SubLogger& log = Logger::log(LoggerMode::IOLog);
    log << "+++ Station changed +++\n";

    std::wstring station_id = data_->w_array[0];
    std::wstring group_id = data_->w_array[1];
    std::wstring point_cloud = data_->w_array[2];

    log << "station id:  " << station_id << "\n";
    log << "group id:    " << group_id << "\n";
    log << "point cloud: " << point_cloud << "\n";

    int is_active = data_->i_array[0];
    int is_on = data_->i_array[1];
    int type = data_->i_array[2];
    int planes_detected = data_->i_array[3];
    int spheres_detected = data_->i_array[4];
    int targets_detected = data_->i_array[5];

    log << "is active:   " << is_active << "\n";
    log << "is on:       " << is_on << "\n";
    log << "type:        " << type << "\n";
    log << "planes detected:  " << planes_detected << "\n";
    log << "spheres detected: " << spheres_detected << "\n";
    log << "targets detected: " << targets_detected << "\n";
    log << Logger::endl;

    bool exist = graph_.isFilePathOrScanExists(station_id, point_cloud);
    Logger::log(LoggerMode::IOLog) << "Scan found:" << exist << Logger::endl;

    std::function<bool(const SafePtr<AGraphNode>&)> filter_scan =
        [station_id](const SafePtr<AGraphNode>& node) {
        ReadPtr<AGraphNode> rPtr = node.cget();
        if (!rPtr)
            return false;
        return (rPtr->getType() == ElementType::Scan) &&
               (rPtr->getName().compare(station_id) == 0);
        };

    auto scan = graph_.getNodesOnFilter(filter_scan);

    std::function<bool(const SafePtr<AGraphNode>&)> filter_group =
        [group_id](const SafePtr<AGraphNode>& node) {
        ReadPtr<AGraphNode> rPtr = node.cget();
        if (!rPtr)
            return false;
        return (rPtr->getType() == ElementType::Cluster) &&
               (rPtr->getName().compare(group_id) == 0);
        };

    auto group = graph_.getNodesOnFilter(filter_group);

    if (scan.size() != 1)
        return;
    {
        WritePtr<AGraphNode> wPtr = scan.begin()->get();
        wPtr->setVisible(is_active);

    }

    // TODO - add a test for (TreeType == Hierarchy)
    if (group.size() == 0)
    {
        SafePtr<ClusterNode> new_group = make_safe<ClusterNode>();
        {
            WritePtr<ClusterNode> w_new_group = new_group.get();
            if (!w_new_group)
                return;
            w_new_group->setName(group_id);
        }
        // Do not work
        graph_.addNodesToGraph({ new_group });
        group.insert(new_group);
    }
    else if (group.size() == 1)
    {
        AGraphNode::addOwningLink(*group.begin(), *scan.begin());
    }
}

void ScantraInterface::editIntersectionPlane()
{
    SubLogger& log = Logger::log(LoggerMode::IOLog);
    log << "+++ intersection plane +++\n";

    double q0 = data_->d_array[0];
    double qx = data_->d_array[1];
    double qy = data_->d_array[2];
    double qz = data_->d_array[3];
    double tx = data_->d_array[4];
    double ty = data_->d_array[5];
    double tz = data_->d_array[6];

    log << "q = (" << q0 << ", " << qx << ", " << qy << ", " << qz << ")\n";
    log << "t = (" << tx << ", " << ty << ", " << tz << ")\n";
    log << Logger::endl;
}

void ScantraInterface::editStationColor()
{
    SubLogger& log = Logger::log(LoggerMode::IOLog);
    log << "+++ station color +++\n";

    std::wstring station_id = data_->w_array[0];
    log << "station id: " << station_id << "\n";

    int r = data_->i_array[0];
    int g = data_->i_array[1];
    int b = data_->i_array[2];

    log << "(" << r << ", " << g << ", " << b << ")" << Logger::endl;

    std::function<bool(const SafePtr<AGraphNode>&)> filter_scan =
        [station_id](const SafePtr<AGraphNode>& node) {
        ReadPtr<AGraphNode> rPtr = node.cget();
        if (!rPtr)
            return false;
        return (rPtr->getType() == ElementType::Scan) &&
            (rPtr->getName().compare(station_id) == 0);
        };

    auto scan = graph_.getNodesOnFilter(filter_scan);

    if (scan.size() != 1)
        return;
    {
        WritePtr<AGraphNode> wPtr = scan.begin()->get();
        wPtr->setColor(Color32(r, g, b));
    }
}