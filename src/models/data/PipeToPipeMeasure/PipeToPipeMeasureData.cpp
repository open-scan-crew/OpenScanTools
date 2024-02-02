#include "models/data/PipeToPipeMeasure/PipeToPipeMeasureData.h"

PipeToPipeMeasureData::PipeToPipeMeasureData()
{
}

PipeToPipeMeasureData::~PipeToPipeMeasureData()
{
}

void PipeToPipeMeasureData::copyPipeToPipeMeasureData(const PipeToPipeMeasureData& data)
{
	m_centerP1ToAxeP2 = data.getCenterP1ToAxeP2();
	m_P1ToP2Horizontal = data.getP1ToP2Horizontal();
	m_P1ToP2Vertical = data.getP1ToP2Vertical();
	m_freeDist = data.getFreeDist();
	m_freeDistHorizontal = data.getFreeDistVertical();
	m_freeDistVertical = data.getFreeDistVertical();
	m_totalFootprint = data.getTotalFootprint();
	m_pipe1Diameter = data.getPipe1Diameter();
	m_pipe2Diameter = data.getPipe2Diameter();
	m_pipe1Center = data.getPipe1Center();
	m_pipe2Center = data.getPipe2Center();
	m_projPoint = data.getProjPoint();
	m_pipe2CenterToProj = data.getPipe2CenterToProj();
}

void PipeToPipeMeasureData::setCenterP1ToAxeP2(float centerP1ToAxeP2)
{
	m_centerP1ToAxeP2 = centerP1ToAxeP2;
}

void PipeToPipeMeasureData::setP1ToP2Horizontal(float P1ToP2Horizontal)
{
	m_P1ToP2Horizontal = P1ToP2Horizontal;
}

void PipeToPipeMeasureData::setP1ToP2Vertical(float P1ToP2Vertical)
{
	m_P1ToP2Vertical = P1ToP2Vertical;
}

void PipeToPipeMeasureData::setFreeDist(float freeD)
{
	m_freeDist = freeD;
}

void PipeToPipeMeasureData::setFreeDistHorizontal(float freeDHorizontal)
{
	m_freeDistHorizontal = freeDHorizontal;
}

void PipeToPipeMeasureData::setFreeDistVertical(float freeDVertical)
{
	m_freeDistVertical = freeDVertical;
}

void PipeToPipeMeasureData::setTotalFootprint(float totalF)
{
	m_totalFootprint = totalF;
}

void PipeToPipeMeasureData::setPipe1Diameter(float pipe1Diameter)
{
	m_pipe1Diameter = pipe1Diameter;
}

void PipeToPipeMeasureData::setPipe2Diameter(float pipe2Diameter)
{
	m_pipe2Diameter = pipe2Diameter;
}

void PipeToPipeMeasureData::setPipe1Center(const Pos3D & pipe1Center)
{
	m_pipe1Center = pipe1Center;
}

void PipeToPipeMeasureData::setPipe2Center(const Pos3D & pipe2Center)
{
	m_pipe2Center = pipe2Center;
}

void PipeToPipeMeasureData::setProjPoint(const Pos3D & projPoint)
{
	m_projPoint = projPoint;
}

void PipeToPipeMeasureData::setPipe2CenterToProj(float pipe2CenterToProj)
{
	m_pipe2CenterToProj = pipe2CenterToProj;
}

float PipeToPipeMeasureData::getCenterP1ToAxeP2() const
{
	return (m_centerP1ToAxeP2);
}

float PipeToPipeMeasureData::getP1ToP2Horizontal() const
{
	return (m_P1ToP2Horizontal);
}

float PipeToPipeMeasureData::getP1ToP2Vertical() const
{
	return (m_P1ToP2Vertical);
}

float PipeToPipeMeasureData::getFreeDist() const
{
	return (m_freeDist);
}

float PipeToPipeMeasureData::getFreeDistHorizontal() const
{
	return (m_freeDistHorizontal);
}

float PipeToPipeMeasureData::getFreeDistVertical() const
{
	return (m_freeDistVertical);
}

float PipeToPipeMeasureData::getTotalFootprint() const
{
	return m_totalFootprint;
}

float PipeToPipeMeasureData::getPipe1Diameter() const
{
	return (m_pipe1Diameter);
}

float PipeToPipeMeasureData::getPipe2Diameter() const
{
	return (m_pipe2Diameter);
}

const Pos3D& PipeToPipeMeasureData::getPipe1Center() const
{
	return (m_pipe1Center);
}

const Pos3D& PipeToPipeMeasureData::getPipe2Center() const
{
	return (m_pipe2Center);
}

const Pos3D& PipeToPipeMeasureData::getProjPoint() const
{
	return (m_projPoint);
}

float PipeToPipeMeasureData::getPipe2CenterToProj() const
{
	return (m_pipe2CenterToProj);
}

Measure PipeToPipeMeasureData::getFirstMeasure() const
{
	return Measure({ m_pipe1Center, m_projPoint });
}

Measure PipeToPipeMeasureData::getSecondMeasure() const
{
	return Measure({ m_projPoint, m_pipe2Center });
}

std::vector<Measure> PipeToPipeMeasureData::getMeasures() const
{
	return { getFirstMeasure(), getSecondMeasure() };
}
