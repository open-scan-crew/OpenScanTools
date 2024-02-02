#ifndef SMART_BUFFER_WORKLOAD_H
#define SMART_BUFFER_WORKLOAD_H

#include "pointCloudEngine/SmartBuffer.h"

#include <cstdint>
#include <vector>
#include <future>

class SmartBufferWorkload
{
public:
    SmartBufferWorkload(/*int priority*/);
    // Automatically unlock the cell buffers locked
    ~SmartBufferWorkload();

    void addCellBuffer(uint32_t id, SmartBuffer& sbuf);
    void setMissingBuffers(std::vector<uint32_t>& cellIds);
    std::vector<uint32_t> getMissingBuffers();
    bool isBufferMissing();
    std::future<bool> getFuture();
    void setPromise(bool loadResult);

    void incrementPassCount();
    uint32_t getPassCount();

private:
    std::vector<SmartBuffer*> m_buffersLocked;
    std::vector<uint32_t> m_missingBuffers;
    std::promise<bool> m_loadPromise;
    uint32_t m_passCount;
};

#endif