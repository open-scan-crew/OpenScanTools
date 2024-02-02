#include "models/data/PipeToPlaneMeasure/PipeToPlaneMeasureData.h"

PipeToPlaneMeasureData::PipeToPlaneMeasureData()
{
	m_centerToPlaneDist = 0.0f;
	m_planeCenterHorizontal = 0.0f;
	m_planeCenterVertical = 0.0f;
	m_freeDist = 0.0f;
	m_freeDistHorizontal = 0.0f;
	m_freeDistVertical = 0.0f;
	m_pipeDiameter = 0.0f;
	m_pipeCenter = Pos3D({ 0, 0, 0 });
	m_pointOnPlane = Pos3D({ 0, 0, 0 });
	m_normalOnPlane = Pos3D({ 0, 0, 0 });
	m_projPoint = Pos3D({ 0, 0, 0 });
	m_pointOnPlaneToProject = 0.0f;
}

PipeToPlaneMeasureData::~PipeToPlaneMeasureData()
{
}

void PipeToPlaneMeasureData::copyPipeToPlaneMeasureData(const PipeToPlaneMeasureData& data)
{
	m_centerToPlaneDist = data.getCenterToPlaneDist();
	m_planeCenterHorizontal = data.getPlaneCenterHorizontal();
	m_planeCenterVertical = data.getPlaneCenterVertical();
	m_freeDist = data.getFreeDist();
	m_freeDistHorizontal = data.getFreeDistHorizontal();
	m_freeDistVertical = data.getFreeDistVertical();
	m_totalFootprint = data.getTotalFootprint();
	m_pipeDiameter = data.getPipeDiameter();
	m_pipeCenter = data.getPipeCenter();
	m_pointOnPlane = data.getPointOnPlane();
	m_normalOnPlane = data.getNormalOnPlane();
	m_projPoint = data.getProjPoint();
	m_pointOnPlaneToProject = data.getPointOnPlaneToProj();
}

void PipeToPlaneMeasureData::setCenterToPlaneDist(float centerToPlaneD)
{
	m_centerToPlaneDist = centerToPlaneD;
}

void PipeToPlaneMeasureData::setPlaneCenterHorizontal(float planeCenterHorizontal)
{
	m_planeCenterHorizontal = planeCenterHorizontal;
}

void PipeToPlaneMeasureData::setPlaneCenterVertical(float planeCenterVertical)
{
	m_planeCenterVertical = planeCenterVertical;
}

void PipeToPlaneMeasureData::setFreeDist(float freeDist)
{
	m_freeDist = freeDist;
}

void PipeToPlaneMeasureData::setFreeDistHorizontal(float freeDistHorizontal)
{
	m_freeDistHorizontal = freeDistHorizontal;
}

void PipeToPlaneMeasureData::setFreeDistVertical(float freeDistVertical)
{
	m_freeDistVertical = freeDistVertical;
}

void PipeToPlaneMeasureData::setTotalFootprint(float totalF)
{
	m_totalFootprint = totalF;
}

void PipeToPlaneMeasureData::setPipeDiameter(float pipeDiameter)
{
	m_pipeDiameter = pipeDiameter;
}

void PipeToPlaneMeasureData::setPipeCenter(const Pos3D & pipeCenter)
{
	m_pipeCenter = pipeCenter;
}

void PipeToPlaneMeasureData::setPointOnPlane(const Pos3D & pointOnPlane)
{
	m_pointOnPlane = pointOnPlane;
}

void PipeToPlaneMeasureData::setNormalOnPlane(const Pos3D & normalOnPlane)
{
	m_normalOnPlane = normalOnPlane;
}

void PipeToPlaneMeasureData::setProjPoint(const Pos3D & projPoint)
{
	m_projPoint = projPoint;
}

void PipeToPlaneMeasureData::setPointOnPlaneToProj(float pointOnPlaneToProject)
{
	m_pointOnPlaneToProject = pointOnPlaneToProject;
}

float PipeToPlaneMeasureData::getCenterToPlaneDist() const
{
	return (m_centerToPlaneDist);
}

float PipeToPlaneMeasureData::getPlaneCenterHorizontal() const
{
	return (m_planeCenterHorizontal);
}

float PipeToPlaneMeasureData::getPlaneCenterVertical() const
{
	return (m_planeCenterVertical);
}

float PipeToPlaneMeasureData::getFreeDist() const
{
	return (m_freeDist);
}

float PipeToPlaneMeasureData::getFreeDistHorizontal() const
{
	return (m_freeDistHorizontal);
}

float PipeToPlaneMeasureData::getFreeDistVertical() const
{
	return (m_freeDistVertical);
}

float PipeToPlaneMeasureData::getTotalFootprint() const
{
	return m_totalFootprint;
}

float PipeToPlaneMeasureData::getPipeDiameter() const
{
	return (m_pipeDiameter);
}

const Pos3D& PipeToPlaneMeasureData::getPipeCenter() const
{
	return (m_pipeCenter);
}

const Pos3D& PipeToPlaneMeasureData::getPointOnPlane() const
{
	return (m_pointOnPlane);
}

const Pos3D& PipeToPlaneMeasureData::getNormalOnPlane() const
{
	return (m_normalOnPlane);
}

const Pos3D& PipeToPlaneMeasureData::getProjPoint() const
{
	return (m_projPoint);
}

float PipeToPlaneMeasureData::getPointOnPlaneToProj() const
{
	return (m_pointOnPlaneToProject);
}

Measure PipeToPlaneMeasureData::getFirstMeasure() const
{
	return Measure({ m_pipeCenter, m_projPoint });
}

Measure PipeToPlaneMeasureData::getSecondMeasure() const
{
	return Measure({ m_projPoint, m_pointOnPlane });
}

std::vector<Measure> PipeToPlaneMeasureData::getMeasures() const
{
	return { getFirstMeasure(), getSecondMeasure() };
}