#include "models/data/ColumnTiltMeasure/ColumnTiltMeasureData.h"

ColumnTiltMeasureData::ColumnTiltMeasureData()
{
}

ColumnTiltMeasureData::~ColumnTiltMeasureData()
{
}

void ColumnTiltMeasureData::copyColumnTiltMeasureData(const ColumnTiltMeasureData& data)
{
	m_p1 = data.getPoint1();
	m_p2 = data.getPoint2();
	m_bp = data.getBottomPoint();
	m_tp = data.getTopPoint();
	m_tiltValue = data.getTiltValue();
	m_height = data.getHeight();
	m_ratio = data.getRatio();
	m_maxRatio = data.getMaxRatio();
	m_ratioSup = data.getRatioSup();
	m_result = data.getResultReliability();
}

void ColumnTiltMeasureData::setPoint1(const Pos3D & p1)
{
	m_p1 = p1;
}

void ColumnTiltMeasureData::setPoint2(const Pos3D & p2)
{
	m_p2 = p2;
}

void ColumnTiltMeasureData::setBottomPoint(const Pos3D & bp)
{
	m_bp = bp;
}

void ColumnTiltMeasureData::setTopPoint(const Pos3D & tp)
{
	m_tp = tp;
}

void ColumnTiltMeasureData::setTiltValue(float tiltvalue)
{
	m_tiltValue = tiltvalue;
}

void ColumnTiltMeasureData::setHeight(float height)
{
	m_height = height;
}

void ColumnTiltMeasureData::setRatio(float ratio)
{
	m_ratio = ratio;
}

void ColumnTiltMeasureData::setMaxRatio(float maxRatio)
{
	m_maxRatio = maxRatio;
}

void ColumnTiltMeasureData::setRatioSup(RatioSup ratioSup)
{
	m_ratioSup = ratioSup;
}

void ColumnTiltMeasureData::setResultReliability(Reliability result)
{
	m_result = result;
}

const Pos3D& ColumnTiltMeasureData::getPoint1() const
{
	return (m_p1);
}

const Pos3D& ColumnTiltMeasureData::getPoint2() const
{
	return (m_p2);
}

const Pos3D& ColumnTiltMeasureData::getBottomPoint() const
{
	return (m_bp);
}

const Pos3D& ColumnTiltMeasureData::getTopPoint() const
{
	return (m_tp);
}

float ColumnTiltMeasureData::getTiltValue() const
{
	return (m_tiltValue);
}

float ColumnTiltMeasureData::getHeight() const
{
	return (m_height);
}

float ColumnTiltMeasureData::getRatio() const
{
	return (m_ratio);
}

float ColumnTiltMeasureData::getMaxRatio() const
{
	return (m_maxRatio);
}

RatioSup ColumnTiltMeasureData::getRatioSup() const
{
	return (m_ratioSup);
}

Reliability ColumnTiltMeasureData::getResultReliability() const
{
	return (m_result);
}
