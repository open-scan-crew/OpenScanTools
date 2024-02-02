#include "models/data/PointToPlaneMeasure/PointToPlaneMeasureData.h"

PointToPlaneMeasureData::PointToPlaneMeasureData()
{ }

PointToPlaneMeasureData::~PointToPlaneMeasureData()
{ }

void PointToPlaneMeasureData::copyPointToPlaneMeasureData(const PointToPlaneMeasureData& data)
{
	m_pointToPlaneD = data.getPointToPlaneD();
	m_horizontal = data.getHorizontal();
	m_vertical = data.getVertical();
	m_pointCoord = data.getPointCoord();
	m_pointOnPlane = data.getPointOnPlane();
	m_normalToPlane = data.getNormalToPlane();
	m_ProjPoint = data.getProjPoint();
}

void PointToPlaneMeasureData::setPointToPlaneD(float pointToPlaneD)
{
	m_pointToPlaneD = pointToPlaneD;
}

void PointToPlaneMeasureData::setHorizontal(float horizontal)
{
	m_horizontal = horizontal;
}

void PointToPlaneMeasureData::setVertical(float vertical)
{
	m_vertical = vertical;
}

void PointToPlaneMeasureData::setPointCoord(const Pos3D & pointCoord)
{
	m_pointCoord = pointCoord;
}

void PointToPlaneMeasureData::setpointOnPlane(const Pos3D & pointOnPlane)
{
	m_pointOnPlane = pointOnPlane;
}

void PointToPlaneMeasureData::setNormalToPlane(const Pos3D & normalToPlane)
{
	m_normalToPlane = normalToPlane;
}

void PointToPlaneMeasureData::setProjPoint(const Pos3D & projPoint)
{
	m_ProjPoint = projPoint;
}

float PointToPlaneMeasureData::getPointToPlaneD() const
{
	return (m_pointToPlaneD);
}

float PointToPlaneMeasureData::getHorizontal() const
{
	return (m_horizontal);
}

float PointToPlaneMeasureData::getVertical() const
{
	return (m_vertical);
}

float PointToPlaneMeasureData::getPointProjToPlaneD() const
{
	return (float)glm::distance(m_pointOnPlane, m_ProjPoint);
}

const Pos3D& PointToPlaneMeasureData::getPointCoord() const
{
	return (m_pointCoord);
}

const Pos3D& PointToPlaneMeasureData::getPointOnPlane() const
{
	return (m_pointOnPlane);
}

const Pos3D& PointToPlaneMeasureData::getNormalToPlane() const
{
	return (m_normalToPlane);
}

const Pos3D& PointToPlaneMeasureData::getProjPoint() const
{
	return (m_ProjPoint);
}

Measure PointToPlaneMeasureData::getFirstMeasure() const
{
	return Measure({ m_pointOnPlane, m_ProjPoint });
}

Measure PointToPlaneMeasureData::getSecondMeasure() const
{
	return Measure({ m_pointCoord, m_ProjPoint });
}

std::vector<Measure> PointToPlaneMeasureData::getMeasures() const
{
	return { getFirstMeasure(), getSecondMeasure() };
}