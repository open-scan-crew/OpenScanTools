#ifndef COLUMNTILTMEASUREDATA_H_
#define COLUMNTILTMEASUREDATA_H_

#include "models/OpenScanToolsModelEssentials.h"
#include "models/3d/Measures.h"

class ColumnTiltMeasureData
{
public:
	ColumnTiltMeasureData();
	virtual ~ColumnTiltMeasureData();

	void copyColumnTiltMeasureData(const ColumnTiltMeasureData& data);
	void setPoint1(const Pos3D& p1);
	void setPoint2(const Pos3D& p2);
	void setBottomPoint(const Pos3D& bp);
	virtual void setTopPoint(const Pos3D& tp);

	void setTiltValue(float tiltvalue);
	void setHeight(float height);
	void setRatio(float ratio);
	void setMaxRatio(float maxRatio);
	void setRatioSup(RatioSup ratioSup);
	void setResultReliability(Reliability result);

	const Pos3D& getPoint1() const;
	const Pos3D& getPoint2() const;
	const Pos3D& getBottomPoint() const;
	const Pos3D& getTopPoint() const;

	float getTiltValue() const;
	float getHeight() const;
	float getRatio() const;
	float getMaxRatio() const;
	RatioSup getRatioSup() const;
	Reliability getResultReliability() const;

protected:
	Pos3D m_p1 = Pos3D();
	Pos3D m_p2 = Pos3D();

	Pos3D m_bp = Pos3D();
	Pos3D m_tp = Pos3D();

	float m_tiltValue = 0;
	float m_height = 0;
	float m_ratio = 0;
	float m_maxRatio = 0;
	RatioSup m_ratioSup = RatioSup::NA;
	Reliability m_result = Reliability::NA;

};

#endif