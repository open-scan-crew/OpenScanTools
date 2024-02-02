#ifndef POINTTOPIPEMEASUREDATA_H_
#define POINTTOPIPEMEASUREDATA_H_

#include "models/OpenScanToolsModelEssentials.h"
#include "models/data/Measure/MeasureData.h"
#include "models/3d/Measures.h"

class PointToPipeMeasureData : public MeasureData
{
public:
	PointToPipeMeasureData();
	~PointToPipeMeasureData();

	void copyPointToPipeMeasureData(const PointToPipeMeasureData& data);

	void setPointToAxeDist(float dPToAxe);
	void setPointToAxeHorizontal(float pToAxeHorizontal);
	void setPointToAxeVertical(float pToAxeVertical);
	void setFreeD(float freeD);
	void setFreeDistHorizontal(float freeDHorizontal);
	void setFreeDistVertical(float freeDVertical);
	void setTotalFootprint(float totalF);

	void setPipeDiameter(float pipeDiameter);
	void setPipeCenter(const Pos3D& pipeCenter);
	void setPointCoord(const Pos3D& pointCoord);
	void setProjPoint(const Pos3D& projPoint);
	void setPipeCenterToProj(float pipeCenterToProj);

	float getPointToAxeDist() const;
	float getPointToAxeHorizontal() const;
	float getPointToAxeVertical() const;
	float getFreeDist() const;
	float getFreeDistHorizontal() const;
	float getFreeDistVertical() const;
	float getTotalFootprint() const;
	float getPipeDiameter() const;
	const Pos3D& getPipeCenter() const;
	const Pos3D& getPointCoord() const;
	const Pos3D& getProjPoint() const;
	float getPipeCenterToProj() const;

	Measure getFirstMeasure() const;
	Measure getSecondMeasure() const;
	std::vector<Measure> getMeasures() const override;

protected:
	float m_pointToAxeDist = 0.0f;
	float m_pointToAxeHorizontal = 0.0f;
	float m_pointToAxeVertical = 0.0f;
	float m_freeDist = 0.0f;
	float m_freeDistHorizontal = 0.0f;
	float m_freeDistVertical = 0.0f;
	float m_totalFootprint = 0.0f;

	float m_pipeDiameter = 0.0f;
	Pos3D m_pipeCenter = Pos3D({ 0, 0, 0 });
	Pos3D m_pointCoord = Pos3D({ 0, 0, 0 });
	Pos3D m_projPoint = Pos3D({ 0, 0, 0 });
	float m_pipeCenterToProj = 0.0f;
};
#endif