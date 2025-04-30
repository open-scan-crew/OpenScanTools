#ifndef SCANTRA_INTERFACE_H
#define SCANTRA_INTERFACE_H

#include "scantra/ScantraInterprocessMemory.h"

#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "utils/safe_ptr.h"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

#include <filesystem>
#include <thread>
#include <unordered_set>

class Controller;
class IDataDispatcher;
class GraphManager;
class AGraphNode;
class BoxNode;

class ScantraInterface
{
public:
    ScantraInterface(Controller& controller, IDataDispatcher& data_dispatcher, GraphManager& graph);
    ~ScantraInterface();

    int startInterface();
    int stopInterface();

    void project_created(const std::filesystem::path& path, const std::wstring& name);
    void project_opened(const std::filesystem::path& path);

private:
    void run();

    struct ScantraStation
    {
        std::wstring station_id;
        std::wstring group_id;
        std::wstring point_cloud;
        int is_active;
        int is_on;
        int type;
        int planes_detected;
        int spheres_detected;
        int targets_detected;
    };

    ScantraStation readStation();
    void addStation();
    void editStationChanged();
    void editIntersectionPlane();
    void editStationColor();
    void editStationAdjustment();
    void createProject();
    void openProject();

    SafePtr<AGraphNode> getScanOnName(std::wstring _name);
    void applyRegistration();
    void manageVisibility(int current_idx, int total_station, SafePtr<AGraphNode> scan);
    void changeGraphicSettings();
    void deactivateIntersectionBox();

    SafePtr<BoxNode> getIntersectionBox();

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

    struct Registration
    {
        SafePtr<AGraphNode> referential;
        SafePtr<AGraphNode> station;
        glm::dvec3 position;
        glm::dquat rotation;
    };

    std::vector<Registration> referentials_;
    std::vector<Registration> scans_registered_;
    std::unordered_set<SafePtr<AGraphNode>> scan_selection_;
    GraphManager& graph_;
};

#endif