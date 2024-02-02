#ifndef BEAMBENDINGMEASUREDATA_H_
#define BEAMBENDINGMEASUREDATA_H_

#include "models/data/Measure/MeasureData.h"
#include "models/OpenScanToolsModelEssentials.h"
#include "models/3d/Measures.h"

class BeamBendingMeasureData : public MeasureData
{
public:
	BeamBendingMeasureData();
	virtual ~BeamBendingMeasureData();

	void copyBeamBendingData(const BeamBendingMeasureData& data);
	void setPoint1Pos(const Pos3D& pos);
	void setPoint2Pos(const Pos3D& pos);
	virtual void setMaxBendingPos(const Pos3D& pos);
	void setBendingValue(float value);
	void setLength(float length); 
	void setRatio(float ratio);
	void setMaxRatio(float maxRatio);
	void setRatioSup(RatioSup ratioSup);
	void setResultReliability(Reliability result);

	const Pos3D& getPoint1Pos() const;
	const Pos3D& getPoint2Pos() const;
	const Pos3D& getMaxBendingPos() const;
	float getBendingValue() const;
	float getLength() const;
	float getRatio() const;
	float getMaxRatio() const;
	RatioSup getRatioSup() const;
	Reliability getResult() const;

	Measure getFirstMeasure() const;
	Measure getSecondMeasure() const;
	std::vector<Measure> getMeasures() const override;

protected:
	Pos3D m_p1 = Pos3D();
	Pos3D m_p2 = Pos3D();
	Pos3D m_maxBendPos = Pos3D();

	float m_bendingValue = 0;
	float m_length = 0;
	float m_ratio = 0;
	float m_maxRatio = 0;
	RatioSup m_ratioSup = RatioSup::NA;
	Reliability m_result = Reliability::NA;

};

#endif