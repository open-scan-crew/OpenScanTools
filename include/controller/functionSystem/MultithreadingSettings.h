#ifndef MULTITHREADING_SETTINGS_H
#define MULTITHREADING_SETTINGS_H

#include "utils/Config.h"

#include <algorithm>
#include <thread>

struct MultithreadingSettings
{
    bool enabled = false;
    int requestedThreads = 1;
    int maxHardwareThreads = 1;

    int resolveThreadCount(size_t taskCount) const
    {
        if (!enabled || taskCount <= 1)
            return 1;
        int cappedRequested = std::max(1, requestedThreads);
        int cappedHardware = std::max(1, maxHardwareThreads);
        int cappedTasks = static_cast<int>(std::max<size_t>(1, taskCount));
        return std::max(1, std::min({ cappedRequested, cappedHardware, cappedTasks }));
    }
};

inline MultithreadingSettings getMultithreadingSettings()
{
    MultithreadingSettings settings;
    settings.enabled = Config::getMultithreadingEnabled();
    settings.requestedThreads = Config::getMultithreadingThreads();
    settings.maxHardwareThreads = static_cast<int>(std::thread::hardware_concurrency());
    if (settings.maxHardwareThreads <= 0)
        settings.maxHardwareThreads = 1;
    return settings;
}

#endif
