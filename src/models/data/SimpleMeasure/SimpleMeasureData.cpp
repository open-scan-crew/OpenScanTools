#include "models/data/SimpleMeasure/SimpleMeasureData.h"


SimpleMeasureData::SimpleMeasureData()
{ }

SimpleMeasureData::~SimpleMeasureData()
{ }

void SimpleMeasureData::copySimpleMeasureData(const SimpleMeasureData& data)
{
	m_measure = data.getMeasure();
}

void SimpleMeasureData::setOriginPos(const Pos3D& pos)
{
	m_measure.origin = pos;
}

void SimpleMeasureData::setDestinationPos(const Pos3D& pos)
{
	m_measure.final = pos;
}

void SimpleMeasureData::setMeasure(Measure measure)
{
	m_measure = measure;
}

const Pos3D& SimpleMeasureData::getOriginPos() const
{
	return (m_measure.origin);
}

const Pos3D& SimpleMeasureData::getDestinationPos() const
{
	return (m_measure.final);
}

const Measure& SimpleMeasureData::getMeasure() const
{
	return (m_measure);
}

std::vector<Measure> SimpleMeasureData::getMeasures() const
{
	return { m_measure };
}