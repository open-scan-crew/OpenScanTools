#include "models/data/Clipping/ClippingData.h"
#include "controller/ControllerContext.h"
#include "models/Types.hpp"

ClippingData::ClippingData(const ClippingData& data)
{
    copyClippingData(data);
}

ClippingData::ClippingData()
{ }

ClippingData::~ClippingData()
{ }

void ClippingData::copyClippingData(const ClippingData& data)
{
    m_clippingMode = data.getClippingMode();
    m_clippingActive = data.isClippingActive();
    m_minClipDist = data.getMinClipDist();
    m_maxClipDist = data.getMaxClipDist();

    m_rampActive = data.isRampActive();
    m_rampMin = data.getRampMin();
    m_rampMax = data.getRampMax();
    m_rampSteps = data.getRampSteps();
    m_rampClamped = data.isRampClamped();
}

void ClippingData::setClippingMode(ClippingMode mode)
{
    m_clippingMode = mode;
}

void ClippingData::setClippingActive(bool active)
{
    m_clippingActive = active;
}

void ClippingData::setMinClipDist(float minDist)
{
    m_minClipDist = minDist;
}

void ClippingData::setMaxClipDist(float maxDist)
{
    m_maxClipDist = maxDist;
}

void ClippingData::setRampActive(bool active)
{
    m_rampActive = active;
}

void ClippingData::setRampMin(float min)
{
    m_rampMin = min;
}

void ClippingData::setRampMax(float max)
{
    m_rampMax = max;
}

void ClippingData::setRampSteps(int steps)
{
    m_rampSteps = steps;
}

void ClippingData::setRampClamped(bool clamped)
{
    m_rampClamped = clamped;
}

void ClippingData::setDefaultData(const ControllerContext& context, ElementType type)
{
    const ProjectInfos& projInfos = context.cgetProjectInfo();
    setClippingMode(projInfos.m_defaultClipMode);
    setMinClipDist(projInfos.m_defaultMinClipDistance);
    setMaxClipDist(projInfos.m_defaultMaxClipDistance);
    setRampMin(projInfos.m_defaultMinRampDistance);
    setRampMax(projInfos.m_defaultMaxRampDistance);
    setRampSteps(projInfos.m_defaultRampSteps);
}

ClippingMode ClippingData::getClippingMode() const
{
    return (m_clippingMode);
}

bool ClippingData::isClippingActive() const
{
    return (m_clippingActive);
}

float ClippingData::getMinClipDist() const
{
    return (m_minClipDist);
}

float ClippingData::getMaxClipDist() const
{
    return (m_maxClipDist);
}

bool ClippingData::isRampActive() const
{
    return m_rampActive;
}

float ClippingData::getRampMin() const
{
    return m_rampMin;
}

float ClippingData::getRampMax() const
{
    return m_rampMax;
}

int ClippingData::getRampSteps() const
{
    return m_rampSteps;
}

bool ClippingData::isRampClamped() const
{
    return m_rampClamped;
}