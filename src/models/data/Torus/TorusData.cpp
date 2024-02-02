#include "models/data/Torus/TorusData.h"

TorusData::TorusData(const double& mainAngle, const double& mainRadius, const double& tubeRadius)
	: m_mainRadius(mainRadius)
	, m_mainAngle(mainAngle)
	, m_tubeRadius(tubeRadius)
{}

TorusData::~TorusData()
{}

void TorusData::copyTorusData(const TorusData& torus)
{
	m_mainRadius = torus.getMainRadius();
	m_mainAngle = torus.getMainAngle();
	m_tubeRadius = torus.getTubeRadius();
}

void TorusData::setMainRadius(const double& radius)
{
	m_mainRadius = abs(radius);
}

void TorusData::setMainAngle(const double& angle)
{
	m_mainAngle = abs(angle);
}

void TorusData::setTubeRadius(const double& radius)
{
	m_tubeRadius = abs(radius);
}

const double& TorusData::getMainRadius() const
{
	return m_mainRadius;
}

const double& TorusData::getMainAngle() const
{
	return m_mainAngle;
}

const double& TorusData::getTubeRadius() const
{
	return m_tubeRadius;
}

double TorusData::getAdjustedTubeRadius() const
{
	return m_tubeRadius - m_insulationRadius;
}

glm::vec3 TorusData::calculateOffset() const
{
	return glm::vec3(m_mainRadius * cos(m_mainAngle / 2.0f), m_mainRadius * sin(m_mainAngle / 2.0f), 0.0f);
}