#include "pointCloudEngine/SmartBufferWorkload.h"
#include "vulkan/VulkanManager.h"

SmartBufferWorkload::SmartBufferWorkload()
    : m_passCount(0)
{

}

SmartBufferWorkload::~SmartBufferWorkload()
{
    for (SmartBuffer* sbuf : m_buffersLocked)
    {
        VulkanManager::unlockSmartBuffer(*sbuf);
    }
}

void SmartBufferWorkload::addCellBuffer(uint32_t id, SmartBuffer& sbuf)
{
    if (VulkanManager::lockSmartBuffer(sbuf))
        m_buffersLocked.push_back(&sbuf);

    if (sbuf.state != TlDataState::LOADED)
        m_missingBuffers.push_back(id);
}

void SmartBufferWorkload::setMissingBuffers(std::vector<uint32_t>& cellIds)
{
    m_missingBuffers = cellIds;
}

std::vector<uint32_t> SmartBufferWorkload::getMissingBuffers()
{
    return m_missingBuffers;
}

bool SmartBufferWorkload::isBufferMissing()
{
    return (!m_missingBuffers.empty());
}

std::future<bool> SmartBufferWorkload::getFuture()
{
    return (m_loadPromise.get_future());
}

void SmartBufferWorkload::setPromise(bool loadResult)
{
    m_loadPromise.set_value(loadResult);
}

void SmartBufferWorkload::incrementPassCount()
{
    m_passCount++;
}

uint32_t SmartBufferWorkload::getPassCount()
{
    return (m_passCount);
}