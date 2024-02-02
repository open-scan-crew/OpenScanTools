#ifndef _PERFORMANCE_MONITOR_H_
#define _PERFORMANCE_MONITOR_H_

#include <string>
#include <vector>
#include <chrono>


struct FrameDetails
{
    std::chrono::time_point<std::chrono::steady_clock> startClock;
    std::chrono::time_point<std::chrono::steady_clock> endClock;
};

class PerformanceMonitor
{
public:
    PerformanceMonitor(std::string name, int reserveFrame);


    void startNextFrame();
    void endFrame();
    void reset();

    // Edit when asked various statistics:
    // FrameCount
    // Mean, Var, Max, max 5%
    // Mean, max per step
    std::string editReport();


private:
    std::string m_name;
    std::vector<FrameDetails> m_frames;
};

#endif