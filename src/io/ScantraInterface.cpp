#include "ScantraInterface.h"
#include "utils/Logger.h"

#include <wchar.h>

ScantraInterface::ScantraInterface(IDataDispatcher& data_dispatcher)
    : data_dispatcher_(data_dispatcher)
{
}

ScantraInterface::~ScantraInterface()
{
    stopInterface();
}

int ScantraInterface::startInterface()
{
    if (data_)
        return -1;

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
            wcsncpy_s(data_->w_array[0], L"OST Interprocess", 256);
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
            wcsncpy_s(data_->w_array[0], L"OST Interprocess", 256);
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
            log << "+++ Station changed +++" << Logger::endl;

            std::wstring station_id = data_->w_array[0];
            std::wstring group_id = data_->w_array[1];
            std::wstring point_cloud = data_->w_array[2];

            log << "station id:  " << station_id << Logger::endl;
            log << "group id:    " << group_id << Logger::endl;
            log << "point cloud: " << point_cloud << Logger::endl;

            int is_active = data_->i_array[0];
            int is_on = data_->i_array[1];
            int type = data_->i_array[2];
            int planes_detected = data_->i_array[3];
            int spheres_detected = data_->i_array[4];
            int targets_detected = data_->i_array[5];

            log << "is active:   " << is_active << Logger::endl;
            log << "is on:       " << is_on << Logger::endl;
            log << "type:        " << type << Logger::endl;
            log << "planes detected:  " << planes_detected << Logger::endl;
            log << "spheres detected: " << spheres_detected << Logger::endl;
            log << "targets detected: " << targets_detected << Logger::endl;
            log << Logger::endl;

            break;
        }
        case ScantraInterprocessObserver::intersectionPlane:
        {
            log << "+++ intersection plane +++" << Logger::endl;

            double q0 = data_->d_array[0];
            double qx = data_->d_array[1];
            double qy = data_->d_array[2];
            double qz = data_->d_array[3];
            double tx = data_->d_array[4];
            double ty = data_->d_array[5];
            double tz = data_->d_array[6];

            log << q0 << Logger::endl;
            log << qx << Logger::endl;
            log << qy << Logger::endl;
            log << qz << Logger::endl;
            log << tx << Logger::endl;
            log << ty << Logger::endl;
            log << tz << Logger::endl;
            log << Logger::endl;
            break;
        }
        case ScantraInterprocessObserver::stationColor:
        {
            log << "+++ station color +++" << Logger::endl;

            std::wstring station_id = data_->w_array[0];
            log << "station id: " << station_id << Logger::endl;

            int r = data_->i_array[0];
            int g = data_->i_array[1];
            int b = data_->i_array[2];

            log << "(" << r << ", " << g << ", " << b << ")" << Logger::endl;

            break;
        }
        }

        observer_->setReturn(return_value);
    }
}