#include "test/PerformanceMonitor.h"


PerformanceMonitor::PerformanceMonitor(std::string name, int reserveFrame) : m_name(name)
{
    if (reserveFrame > 0)
        m_frames.reserve(reserveFrame);
}

void PerformanceMonitor::startNextFrame()
{
    m_frames.push_back(FrameDetails());
    m_frames.back().startClock = std::chrono::steady_clock::now();

}

void PerformanceMonitor::endFrame()
{
    m_frames.back().endClock = std::chrono::steady_clock::now();
}

void PerformanceMonitor::reset()
{
    m_frames.clear();
}

std::string PerformanceMonitor::editReport()
{
    std::string report = "\n//**** Performance Monitor -- " + m_name + " ****//";

    if (m_frames.size() == 0) {
        report += "\n// No frame recorded.";
        report += "\n//****//\n";
        return report;
    }

    float mspf = 0.f;  // milliseconds per frame
    float fps;   // frame per seconds

    float maxFrameTime = 0.f;

    for (int i = 0; i < m_frames.size(); i++)
    {
        // duration in milliseconds
        float d_ms = std::chrono::duration_cast<std::chrono::microseconds>(m_frames[i].endClock - m_frames[i].startClock).count() / 1000.f;

        mspf += d_ms;
        maxFrameTime = std::max(maxFrameTime, d_ms);
    }

    mspf = mspf / m_frames.size();
    float td = std::chrono::duration_cast<std::chrono::milliseconds>(m_frames.back().endClock - m_frames[0].startClock).count() / 1000.f;
    fps = m_frames.size() / td;

    report += "\n//**** Performance Monitor -- " + m_name + " ****//";
    report += "\n";
    report += "\n - Frame count: " + std::to_string(m_frames.size());
    report += "\n - Mean frame time: " + std::to_string(mspf) + " ms (" + std::to_string(fps) + " fps)";
    report += "\n - Max Frame Time: " + std::to_string(maxFrameTime);
    report += "\n";

    return report;
}