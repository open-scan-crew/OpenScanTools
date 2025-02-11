#include "ScantraInterface.h"

#include "controller/Controller.h"
#include "controller/ControlListener.h"
#include "controller/controls/ControlViewport.h"
#include "models/graph/GraphManager.hxx"
#include "models/graph/ClusterNode.h"
#include "models/graph/ScanNode.h"
#include "models/graph/BoxNode.h"
#include "models/graph/ClusterNode.h"
#include "models/graph/CameraNode.h"
#include "gui/DataDispatcher.h"
#include "gui/GuiData/GuiDataTree.h"

#include "utils/math/trigo.h"
#include "utils/Logger.h"

#include <wchar.h>

ScantraInterface::ScantraInterface(Controller& controller, IDataDispatcher& data_dispatcher, GraphManager& graph)
    : controller_(controller)
    , data_dispatcher_(data_dispatcher)
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
        case ScantraInterprocessObserver::newAdjustmentResult:
        {
            editStationAdjustment();
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

    std::function<bool(ReadPtr<AGraphNode>&)> filter_type =
        [group_id](ReadPtr<AGraphNode>& r_node) {
            return (r_node->getType() == ElementType::Cluster) &&
                   (r_node->getName().compare(group_id) == 0);
        };

    std::function<bool(ReadPtr<ClusterNode>&)> filter_tree_type =
        [](const ReadPtr<ClusterNode>& node) {
            return (node->getClusterTreeType() == TreeType::Hierarchy);
        };

    auto clusters = graph_.getNodesOnFilter<ClusterNode>(filter_type, filter_tree_type);

    if (scan.size() != 1)
        return;
    {
        WritePtr<AGraphNode> wPtr = scan.begin()->get();
        wPtr->setVisible(is_on);
    }

    if (clusters.size() == 1)
    {
        AGraphNode::addOwningLink(*clusters.begin(), *scan.begin());
    }

    controller_.actualizeTreeView(scan);
}

void ScantraInterface::editIntersectionPlane()
{
    SubLogger& log = Logger::log(LoggerMode::IOLog);
    log << "+++ intersection plane +++\n";

    double q[4];
    double t[3];
    memcpy(q, data_->d_array, 4 * sizeof(double));
    memcpy(t, data_->d_array + 4, 3 * sizeof(double));

    log << "t = (" << t[0] << ", " << t[1] << ", " << t[2] << ")\n";
    log << "q = (" << q[0] << ", " << q[1] << ", " << q[2] << ", " << q[3] << ")\n";
    log << Logger::endl;

    glm::dquat inv_q = glm::conjugate(glm::dquat(q[0], q[1], q[2], q[3]));
    glm::dvec3 pos(t[0], t[1], t[2]);
    glm::dvec3 plane_pos = glm::mat3_cast(inv_q) * -pos;

    SafePtr<BoxNode> box;
    bool create_box = false;
    // Retreive the box or create it
    {
        std::function<bool(const ReadPtr<AGraphNode>&)> filter_box =
            [](const ReadPtr<AGraphNode>& r_node) {
            return (r_node->getType() == ElementType::Box) &&
                (r_node->getName().compare(L"intersection_plane") == 0);
            };
        auto boxes = graph_.getNodesOnFilter<BoxNode>(filter_box);

        create_box = boxes.size() == 0;
        box = create_box ? make_safe<BoxNode>(true) : *boxes.begin();
        graph_.addNodesToGraph({ box });
    }

    {
        WritePtr<BoxNode> w_box = box.get();
        if (!w_box)
            return;
        w_box->setPosition(plane_pos);
        w_box->setRotation(inv_q);
        w_box->setVisible(false);
        w_box->setClippingMode(ClippingMode::showInterior);
        w_box->setClippingActive(true);
        if (create_box)
        {
            w_box->setName(L"intersection_plane");
            w_box->setSize(glm::dvec3(1000.0, 1000.0, 0.1));
        }
    }

    controller_.actualizeTreeView(box);

    // Move and align the camera
    {
        SafePtr<CameraNode> camera = graph_.getCameraNode();
        WritePtr<CameraNode> w_cam = camera.get();
        if (!w_cam)
            return;
        w_cam->setProjectionMode(ProjectionMode::Orthographic);
        w_cam->setRotation(inv_q);
        w_cam->addPreRotation(glm::dquat(0.0, 1.0, 0.0, 0.0));
        w_cam->setPosition(plane_pos);
    }
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

void ScantraInterface::editStationAdjustment()
{
    assert(data_->n_i == 2);
    SubLogger& log = Logger::log(LoggerMode::IOLog);
    int current_entry = data_->i_array[0];
    int total_entry = data_->i_array[1];
    log << "+++ Station adjustment (" << current_entry + 1 << "/" << total_entry << ") +++\n";

    std::wstring station_id = data_->w_array[0];
    std::wstring datum_id = data_->w_array[1];

    log << "station id:  " << station_id << "\n";
    log << "datum id:    " << datum_id << "\n";

    if (data_->n_d != 7)
        return;
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

    std::function<bool(ReadPtr<AGraphNode>&)> filter_station =
        [station_id](const ReadPtr<AGraphNode>& r_node) {
        return (r_node->getType() == ElementType::Scan) &&
               (r_node->getName().compare(station_id) == 0);
        };

    std::function<bool(ReadPtr<AGraphNode>&)> filter_datum =
        [datum_id](const ReadPtr<AGraphNode>& r_node) {
        return (r_node->getType() == ElementType::Scan) &&
               (r_node->getName().compare(datum_id) == 0);
        };

    auto scan_uset = graph_.getNodesOnFilter<AGraphNode>(filter_station);
    if (scan_uset.size() != 1)
        return;
    SafePtr<AGraphNode> scan = *scan_uset.begin();

    // We try a naive approach, just place the scan at the coordinates received
    if (datum_id.compare(L"GlobalCoordinateSystem") == 0)
    {
        WritePtr<AGraphNode> wPtr = scan.get();
        wPtr->setPosition(glm::dvec3(tx, ty, tz));
        wPtr->setRotation(glm::dquat(q0, qx, qy, qz));
    }
    else
    {
        auto datum_uset = graph_.getNodesOnFilter<AGraphNode>(filter_datum);
        if (scan_uset.size() != 1)
            return;
        SafePtr<AGraphNode> datum = *datum_uset.begin();
        {
            if (datum != scan)
                AGraphNode::addGeometricLink(datum, scan);
            WritePtr<AGraphNode> wPtr = scan.get();
            wPtr->setPosition(glm::dvec3(tx, ty, tz));
            wPtr->setRotation(glm::dquat(q0, qx, qy, qz));
        }
    }

    // On recoit les stations une par une.
    // Il faut rendre invisible les stations que l’on ne recevra pas
    // -> On stocke les entrée que l’on reçoit
    // -> On active/désactive la visibilité des scan lors de la dernière entrée.
    manageVisibility(current_entry, total_entry, scan);
}

void ScantraInterface::manageVisibility(int current_station, int total_station, SafePtr<AGraphNode> scan)
{
    if (current_station == 0)
        scan_selection_.clear();

    // Ajoute le scan à la liste des scans à rendre visibles
    scan_selection_.insert(scan);

    // Change visibility when we have all the scans
    if (current_station == total_station - 1)
    {
        std::unordered_set<SafePtr<AGraphNode>> tree_update;
        
        std::function<bool(ReadPtr<AGraphNode>&)> filter =
            [](ReadPtr<AGraphNode>& r_node) {
            return (r_node->getType() == ElementType::Scan) &&
                   (r_node->isVisible());
            };
        // Mettre les scan visibles
        auto visi_scans = graph_.getNodesOnFilter<AGraphNode>(filter);

        // Hide scans that are not in the selection
        for (SafePtr<AGraphNode> scan : visi_scans)
        {
            if (scan_selection_.find(scan) == scan_selection_.end())
            {
                WritePtr<AGraphNode> w_scan = scan.get();
                if (!w_scan)
                    continue;
                w_scan->setVisible(false);
                tree_update.insert(scan);
            }
        }

        // Show scans in the selection
        for (SafePtr<AGraphNode> scan : scan_selection_)
        {
            WritePtr<AGraphNode> w_scan = scan.get();
            if (!w_scan)
                continue;
            if (w_scan->isVisible() == false)
            {
                w_scan->setVisible(true);
                tree_update.insert(scan);
            }
        }
        controller_.actualizeTreeView(tree_update);
        controller_.getControlListener()->notifyUIControl(new control::viewport::AdjustZoomToScene(SafePtr<CameraNode>()));
    }
}