#include "models/data/Piping/StandardRadiusData.h"

StandardRadiusData::StandardRadiusData(const double& detectedRadius)
	: m_detectedRadius(detectedRadius)
	, m_forcedRadius(detectedRadius)
	, m_standardRadius(detectedRadius)
	, m_diameterSet(DiameterSet::Standard)
{}

StandardRadiusData::StandardRadiusData()
	: m_detectedRadius(1.)
	, m_forcedRadius(1.)
	, m_standardRadius(1.)
	, m_diameterSet(DiameterSet::Standard)
{}

StandardRadiusData::~StandardRadiusData()
{}

void StandardRadiusData::copyStandardRadiusData(const StandardRadiusData& data)
{
	m_detectedRadius = data.m_detectedRadius;
	m_forcedRadius = data.m_forcedRadius;
	m_standardRadius = data.m_standardRadius;
	m_diameterSet = data.m_diameterSet;
}

void StandardRadiusData::setDetectedRadius(const double& radius)
{
	m_detectedRadius = radius;
}

void StandardRadiusData::setForcedRadius(const double& radius)
{
	m_forcedRadius = radius;
}

void StandardRadiusData::setStandard(const SafePtr<StandardList>& standard)
{
	m_standard = standard;
	checkStandard();
}

void StandardRadiusData::setDiameterSet(const StandardRadiusData::DiameterSet& diameter)
{
	m_diameterSet = diameter;
}

void StandardRadiusData::checkStandard()
{
	ReadPtr<StandardList> rStandard = m_standard.cget();
	if (!rStandard)
	{
		m_standardRadius = m_detectedRadius - m_insulationRadius;
		return;
	}

	const double& detectedDiameter = (m_detectedRadius - m_insulationRadius)* 2.0;
	double diameter(detectedDiameter), difference(std::numeric_limits<double>::max());
	for (const double& value : rStandard->clist())
	{
		double diff(abs(detectedDiameter - value));
		if (difference > diff)
		{
			diameter = value;
			difference = diff;
		}
	}

	m_standardRadius = (diameter / 2.0);
}

const SafePtr<StandardList>& StandardRadiusData::getStandard() const
{
	return (m_standard);
}

double StandardRadiusData::getDetectedRadius() const
{
	return m_detectedRadius;
}

double StandardRadiusData::getForcedRadius() const
{
	return m_forcedRadius;
}

double StandardRadiusData::getStandardRadius() const
{
	return m_standardRadius;
}

StandardRadiusData::DiameterSet StandardRadiusData::getDiameterSet() const
{
	return m_diameterSet;
}
