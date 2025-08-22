#include "ScantraInterface.h"

#include "controller/Controller.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/controls/ControlProject.h"
#include "controller/controls/ControlViewport.h"

#include "gui/GuiData/GuiDataRendering.h"

#include "models/graph/GraphManager.hxx"
#include "models/graph/ClusterNode.h"
#include "models/graph/BoxNode.h"
#include "models/graph/CameraNode.h"

#include "io/SaveLoadSystem.h"

#include "utils/Logger.h"

#include <boost/interprocess/shared_memory_object.hpp>

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

void ScantraInterface::project_created(const std::filesystem::path& path, const std::wstring& name)
{
    if (!data_)
        return;
    {
        scoped_lock<interprocess_mutex> lock(data_->mutex); /// or mutex ?
        std::filesystem::path formated_path = path;
        formated_path.make_preferred();
        std::wcsncpy(data_->w_array[0], formated_path.native().c_str(), 256);
        std::wcsncpy(data_->w_array[1], name.c_str(), 256);
        data_->n_w = 2;

        observer_->message_ = ScantraInterprocessObserver::hasCreatedProject;
        Logger::log(LoggerMode::IOLog) << "+++ Scantra-project-created +++" << Logger::endl;
    }
    data_->update(observer_);
}

void ScantraInterface::project_opened(const std::filesystem::path& path)
{
    if (!data_)
        return;
    {
        scoped_lock<interprocess_mutex> lock(data_->mutex);

        std::wcsncpy(data_->w_array[0], path.c_str(), 256);
        data_->n_w = 1;

        observer_->message_ = ScantraInterprocessObserver::hasOpenedProject;
        Logger::log(LoggerMode::IOLog) << "+++ Scantra-project-opened +++" << Logger::endl;
    }
    data_->update(observer_);
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
        case ScantraInterprocessObserver::stationNew:
        {
            addStation(); // NEW
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
        case ScantraInterprocessObserver::createProject:
        {
            createProject(); // NEW
            break;
        }
        case ScantraInterprocessObserver::openProject:
        {
            openProject(); // NEW
            break;
        }
        default:
            break;
        }

        observer_->setReturn(return_value);
    }
}

ScantraInterface::ScantraStation ScantraInterface::readStation()
{
    ScantraStation station;
    if (data_->n_w < 3 || data_->n_i < 6)
        return station;

    station.station_id = data_->w_array[0];
    station.group_id = data_->w_array[1];
    station.point_cloud = data_->w_array[2];

    station.is_active = data_->i_array[0];
    station.is_on = data_->i_array[1];
    station.type = data_->i_array[2];
    station.planes_detected = data_->i_array[3];
    station.spheres_detected = data_->i_array[4];
    station.targets_detected = data_->i_array[5];

    return station;
}

void ScantraInterface::addStation()
{
    SubLogger& log = Logger::log(LoggerMode::IOLog);
    log << "+++ Scantra-add-station +++\n";

    ScantraStation station = readStation();

    std::filesystem::path pc_path = station.point_cloud;
    SaveLoadSystem::ErrorCode err;
    SafePtr<ScanNode> scan = SaveLoadSystem::ImportNewTlsFile(pc_path, controller_, err);
}

void ScantraInterface::editStationChanged()
{
    SubLogger& log = Logger::log(LoggerMode::IOLog);
    log << "+++ Station changed +++\n";

    ScantraStation station = readStation();

    std::function<bool(const SafePtr<AGraphNode>&)> filter_scan =
        [&](const SafePtr<AGraphNode>& node) {
        ReadPtr<AGraphNode> rPtr = node.cget();
        if (!rPtr)
            return false;
        return (rPtr->getType() == ElementType::Scan) &&
               (rPtr->getName().compare(station.station_id) == 0);
        };

    auto scan = graph_.getNodesOnFilter(filter_scan);

    std::function<bool(ReadPtr<AGraphNode>&)> filter_type =
        [&](ReadPtr<AGraphNode>& r_node) {
            return (r_node->getType() == ElementType::Cluster) &&
                   (r_node->getName().compare(station.group_id) == 0);
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
        wPtr->setVisible(station.is_on);
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

    // Retreive the box or create it
    SafePtr<BoxNode> box = getIntersectionBox();
    if (!box)
    {
        box = make_safe<BoxNode>();
        {
            WritePtr<BoxNode> w_box = box.get();
            if (!w_box)
                return;
            w_box->setName(L"intersection_plane");
            w_box->setSize(glm::dvec3(1000.0, 1000.0, 0.1));
        }
        graph_.addNodesToGraph({ box });
    }

    // Edit the box with the properties received
    {
        WritePtr<BoxNode> w_box = box.get();
        if (!w_box)
            return;
        w_box->setPosition(plane_pos);
        w_box->setRotation(inv_q);
        w_box->setVisible(false);
        w_box->setClippingMode(ClippingMode::showInterior);
        w_box->setClippingActive(true);
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

    changeGraphicSettings();
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

    changeGraphicSettings();

    deactivateIntersectionBox();
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

    glm::dvec3 station_pos(tx, ty, tz);
    glm::dquat station_rot(q0, qx, qy, qz);

    SafePtr<AGraphNode> scan = getScanOnName(station_id);
    SafePtr<AGraphNode> datum = getScanOnName(datum_id);

    // Reset the geometric link. All computation are done in the global space.
    AGraphNode::addGeometricLink(graph_.getRoot(), scan);

    if (datum == scan)
    {
        WritePtr<AGraphNode> wScan = scan.get();
        if (wScan)
        {
            wScan->setPosition(station_pos); // should be (0, 0, 0)
            wScan->setRotation(station_rot); // should be (1, 0, 0, 0)
        }
    }
    else
    {
        // Wait for registration
        scans_registered_.push_back({
            datum,
            scan,
            station_pos,
            station_rot
            });
    }

    if (current_entry == total_entry - 1)
        applyRegistration();

    // On recoit les stations une par une.
    // Il faut rendre invisible les stations que l’on ne recevra pas
    // -> On stocke les entrée que l’on reçoit
    // -> On active/désactive la visibilité des scan lors de la dernière entrée.
    manageVisibility(current_entry, total_entry, scan);

    changeGraphicSettings();

    deactivateIntersectionBox();
}

void ScantraInterface::createProject()
{
    SubLogger& log = Logger::log(LoggerMode::IOLog);
    log << "+++ Scantra-create-project +++\n";
    if (data_->n_w != 2)
    {
        log << "Wrong parameter count" << Logger::endl;
        return;
    }

    std::wstring w_folder(data_->w_array[0]);
    std::wstring w_name(data_->w_array[1]);
    IOLOG << "Received project folder: " << w_folder << ", name: " << w_name << Logger::endl;

    controller_.getControlListener()->notifyUIControl(new control::project::SaveCreate(w_folder, w_name, L"Scantra"));
}

void ScantraInterface::openProject()
{
    SubLogger& log = Logger::log(LoggerMode::IOLog);
    log << "+++ Scantra-open-project +++\n";
    if (data_->n_w != 1)
    {
        log << "Wrong parameter count" << Logger::endl;
        return;
    }

    std::filesystem::path project_path(data_->w_array[0]);
    log << "Received project path: " << project_path << Logger::endl;

    controller_.getControlListener()->notifyUIControl(new control::project::SaveCloseLoad(project_path));
}

SafePtr<AGraphNode> ScantraInterface::getScanOnName(std::wstring _name)
{
    std::function<bool(ReadPtr<AGraphNode>&)> filter_station =
        [_name](const ReadPtr<AGraphNode>& r_node) {
        return (r_node->getType() == ElementType::Scan) &&
            (r_node->getName().compare(_name) == 0);
        };

    auto scan_uset = graph_.getNodesOnFilter<AGraphNode>(filter_station);
    if (scan_uset.size() != 1)
        return SafePtr<AGraphNode>();

    return *scan_uset.begin();
}

void ScantraInterface::applyRegistration()
{
    for (auto reg : scans_registered_)
    {
        if (reg.referential == reg.station)
            continue;

        glm::dvec3 ref_pos(0.0, 0.0, 0.0);
        glm::dquat ref_rot(1.0, 0.0, 0.0, 0.0);
        
        // If the referential is null, then the ref coordinates are unchanged.
        {
            ReadPtr<AGraphNode> rDatum = reg.referential.cget();
            if (rDatum)
            {
                ref_pos = rDatum->getCenter();
                ref_rot = rDatum->getRotation();
            }
        }

        {
            WritePtr<AGraphNode> wScan = reg.station.get();
            if (wScan)
            {
                wScan->setPosition(ref_pos);
                wScan->setRotation(ref_rot);

                wScan->addLocalTranslation(reg.position);
                wScan->addPreRotation(reg.rotation);
            }
        }
    }

    scans_registered_.clear();
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

void ScantraInterface::changeGraphicSettings()
{
    // Color mode : colored by scans
    data_dispatcher_.updateInformation(new GuiDataRenderColorMode(UiRenderMode::Scans_Color, SafePtr<CameraNode>()));

    // Desactivate the normals
    PostRenderingNormals normal_settings;
    normal_settings.show = false;
    normal_settings.inverseTone = false;
    normal_settings.normalStrength = 0.5f;
    //normal_settings.blendColor = true;
    //normal_settings.gloss = 1.f;
    data_dispatcher_.updateInformation(new GuiDataPostRenderingNormals(normal_settings, true, SafePtr<CameraNode>()));
}

void ScantraInterface::deactivateIntersectionBox()
{
    SafePtr<BoxNode> box = getIntersectionBox();

    if (!box)
        return;

    WritePtr<BoxNode> w_box = box.get();
    if (!w_box)
        return;
    //w_box->setPosition(plane_pos);
    //w_box->setRotation(inv_q);
    w_box->setVisible(false);
    //w_box->setClippingMode(ClippingMode::showInterior);
    w_box->setClippingActive(false);
}

SafePtr<BoxNode> ScantraInterface::getIntersectionBox()
{
    std::function<bool(const ReadPtr<AGraphNode>&)> filter_box =
        [](const ReadPtr<AGraphNode>& r_node) {
        return (r_node->getType() == ElementType::Box) &&
            (r_node->getName().compare(L"intersection_plane") == 0);
        };
    auto boxes = graph_.getNodesOnFilter<BoxNode>(filter_box);

    if (boxes.size() != 0)
        return *boxes.begin();

    return SafePtr<BoxNode>();
}
