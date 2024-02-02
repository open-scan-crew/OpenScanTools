#include "models/data/BeamBendingMeasure/BeamBendingMeasureData.h"

BeamBendingMeasureData::BeamBendingMeasureData()
{ }

BeamBendingMeasureData::~BeamBendingMeasureData()
{ }


void BeamBendingMeasureData::copyBeamBendingData(const BeamBendingMeasureData& data)
{
	m_p1 = data.getPoint1Pos();
	m_p2 = data.getPoint2Pos();
	m_maxBendPos = data.getMaxBendingPos();
	m_bendingValue = data.getBendingValue();
	m_length = data.getLength();
	m_ratio = data.getRatio();
	m_maxRatio = data.getMaxRatio();
	m_ratioSup = data.getRatioSup();
	m_result = data.getResult();
}

void BeamBendingMeasureData::setPoint1Pos(const Pos3D & pos)
{
	m_p1 = pos;
}

void BeamBendingMeasureData::setPoint2Pos(const Pos3D & pos)
{
	m_p2 = pos;
}

void BeamBendingMeasureData::setMaxBendingPos(const Pos3D & pos)
{
	m_maxBendPos = pos;
}

void BeamBendingMeasureData::setBendingValue(float value)
{
	m_bendingValue = value;
}

void BeamBendingMeasureData::setLength(float lenght)
{
	m_length = lenght;
}

void BeamBendingMeasureData::setRatio(float ratio)
{
	m_ratio = ratio;
}

void BeamBendingMeasureData::setMaxRatio(float maxRatio)
{
	m_maxRatio = maxRatio;
}

void BeamBendingMeasureData::setRatioSup(RatioSup ratioSup)
{
	m_ratioSup = ratioSup;
}

void BeamBendingMeasureData::setResultReliability(Reliability result)
{
	m_result = result;
}

const Pos3D& BeamBendingMeasureData::getPoint1Pos() const
{
	return (m_p1);
}

const Pos3D& BeamBendingMeasureData::getPoint2Pos() const
{
	return (m_p2);
}

const Pos3D& BeamBendingMeasureData::getMaxBendingPos() const
{
	return (m_maxBendPos);
}

float BeamBendingMeasureData::getBendingValue() const
{
	return (m_bendingValue);
}

float BeamBendingMeasureData::getLength() const
{
	return (m_length);
}

float BeamBendingMeasureData::getRatio() const
{
	return (m_ratio);
}

float BeamBendingMeasureData::getMaxRatio() const
{
	return (m_maxRatio);
}

RatioSup BeamBendingMeasureData::getRatioSup() const
{
	return (m_ratioSup);
}

Reliability BeamBendingMeasureData::getResult() const
{
	return (m_result);
}

Measure BeamBendingMeasureData::getFirstMeasure() const
{
	return Measure({ m_p1, m_p2 });
}

Measure BeamBendingMeasureData::getSecondMeasure() const
{
	return Measure({ m_p1, m_maxBendPos });
}

std::vector<Measure> BeamBendingMeasureData::getMeasures() const
{
	return { getFirstMeasure(), getSecondMeasure() };
}