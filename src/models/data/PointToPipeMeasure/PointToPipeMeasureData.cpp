#include "models/data/PointToPipeMeasure/PointToPipeMeasureData.h"

PointToPipeMeasureData::PointToPipeMeasureData()
{
}

PointToPipeMeasureData::~PointToPipeMeasureData()
{
}

void PointToPipeMeasureData::copyPointToPipeMeasureData(const PointToPipeMeasureData& data)
{
	m_pointToAxeDist = data.getPointToAxeDist();
	m_pointToAxeHorizontal = data.getPointToAxeHorizontal();
	m_pointToAxeVertical = data.getPointToAxeVertical();
	m_freeDist = data.getFreeDist();
	m_freeDistHorizontal = data.getFreeDistHorizontal();
	m_freeDistVertical = data.getFreeDistVertical();
	m_totalFootprint = data.getTotalFootprint();
	m_pipeDiameter = data.getPipeDiameter();
	m_pipeCenter = data.getPipeCenter();
	m_pointCoord = data.getPointCoord();
	m_projPoint = data.getProjPoint();
	m_pipeCenterToProj = data.getPipeCenterToProj();
}

void PointToPipeMeasureData::setPointToAxeDist(float dPToAxe)
{
	m_pointToAxeDist = dPToAxe;
}

void PointToPipeMeasureData::setPointToAxeHorizontal(float pToAxeHorizontal)
{
	m_pointToAxeHorizontal = pToAxeHorizontal;
}

void PointToPipeMeasureData::setPointToAxeVertical(float pToAxeVertical)
{
	m_pointToAxeVertical = pToAxeVertical;
}

void PointToPipeMeasureData::setFreeD(float freeD)
{
	m_freeDist = freeD;
}

void PointToPipeMeasureData::setFreeDistHorizontal(float freeDHorizontal)
{
	m_freeDistHorizontal = freeDHorizontal;
}

void PointToPipeMeasureData::setFreeDistVertical(float freeDVertical)
{
	m_freeDistVertical = freeDVertical;
}

void PointToPipeMeasureData::setTotalFootprint(float totalF)
{
	m_totalFootprint = totalF;
}

void PointToPipeMeasureData::setPipeDiameter(float pipeDiameter)
{
	m_pipeDiameter = pipeDiameter;
}

void PointToPipeMeasureData::setPipeCenter(const Pos3D & pipeCenter)
{
	m_pipeCenter = pipeCenter;
}

void PointToPipeMeasureData::setPointCoord(const Pos3D & pointCoord)
{
	m_pointCoord = pointCoord;
}

void PointToPipeMeasureData::setProjPoint(const Pos3D & projPoint)
{
	m_projPoint = projPoint;
}

void PointToPipeMeasureData::setPipeCenterToProj(float pipeCenterToProj)
{
	m_pipeCenterToProj = pipeCenterToProj;
}

float PointToPipeMeasureData::getPointToAxeDist() const
{
	return (m_pointToAxeDist);
}

float PointToPipeMeasureData::getPointToAxeHorizontal() const
{
	return (m_pointToAxeHorizontal);
}

float PointToPipeMeasureData::getPointToAxeVertical() const
{
	return (m_pointToAxeVertical);
}

float PointToPipeMeasureData::getFreeDist() const
{
	return (m_freeDist);
}

float PointToPipeMeasureData::getFreeDistHorizontal() const
{
	return (m_freeDistHorizontal);
}

float PointToPipeMeasureData::getFreeDistVertical() const
{
	return (m_freeDistVertical);
}

float PointToPipeMeasureData::getTotalFootprint() const
{
	return m_totalFootprint;
}

float PointToPipeMeasureData::getPipeDiameter() const
{
	return (m_pipeDiameter);
}

const Pos3D& PointToPipeMeasureData::getPipeCenter() const
{
	return (m_pipeCenter);
}

const Pos3D& PointToPipeMeasureData::getPointCoord() const
{
	return (m_pointCoord);
}

const Pos3D& PointToPipeMeasureData::getProjPoint() const
{
	return (m_projPoint);
}

float PointToPipeMeasureData::getPipeCenterToProj() const
{
	return (m_pipeCenterToProj);
}

Measure PointToPipeMeasureData::getFirstMeasure() const
{
	return Measure({ m_pipeCenter, m_projPoint });
}

Measure PointToPipeMeasureData::getSecondMeasure() const
{
	return Measure({ m_projPoint, m_pointCoord });
}

std::vector<Measure> PointToPipeMeasureData::getMeasures() const
{
	return { getFirstMeasure(), getSecondMeasure() };
}