#ifndef PIPETOPLANEMEASUREDATA_H_
#define PIPETOPLANEMEASUREDATA_H_

#include "models/OpenScanToolsModelEssentials.h"
#include "models/data/Measure/MeasureData.h"
#include "models/3d/Measures.h"

class PipeToPlaneMeasureData : public MeasureData
{
public:
	PipeToPlaneMeasureData();
	virtual ~PipeToPlaneMeasureData();
	
	void copyPipeToPlaneMeasureData(const PipeToPlaneMeasureData& data);

	void setCenterToPlaneDist(float centerToPlaneD);
	void setPlaneCenterHorizontal(float planeCenterHorizontal);
	void setPlaneCenterVertical(float planeCenterVertical);
	void setFreeDist(float freeDist);
	void setFreeDistHorizontal(float freeDistHorizontal);
	void setFreeDistVertical(float freeDistVertical);
	void setTotalFootprint(float totalF);

	void setPipeDiameter(float pipeDiameter);
	void setPipeCenter(const Pos3D & pipeCenter);
	void setPointOnPlane(const Pos3D & pointOnPlane);
	void setNormalOnPlane(const Pos3D & normalOnPlane);
	void setProjPoint(const Pos3D & projPoint);
	void setPointOnPlaneToProj(float pointOnPlaneToProject);

	float getCenterToPlaneDist() const;
	float getPlaneCenterHorizontal() const;
	float getPlaneCenterVertical() const;
	float getFreeDist() const;
	float getFreeDistHorizontal() const;
	float getFreeDistVertical() const;
	float getTotalFootprint() const;
	float getPipeDiameter() const;
	const Pos3D& getPipeCenter() const;
	const Pos3D& getPointOnPlane() const;
	const Pos3D& getNormalOnPlane() const;
	const Pos3D& getProjPoint() const;
	float getPointOnPlaneToProj() const;

	Measure getFirstMeasure() const;
	Measure getSecondMeasure() const;
	std::vector<Measure> getMeasures() const override;

protected:
	float m_centerToPlaneDist = 0.0f;
	float m_planeCenterHorizontal = 0.0f;
	float m_planeCenterVertical = 0.0f;
	float m_freeDist = 0.0f;
	float m_freeDistHorizontal = 0.0f;
	float m_freeDistVertical = 0.0f;
	float m_totalFootprint = 0.0f;

	float m_pipeDiameter = 0.0f;
	Pos3D m_pipeCenter = Pos3D({ 0, 0, 0 });
	Pos3D m_pointOnPlane = Pos3D({ 0, 0, 0 });
	Pos3D m_normalOnPlane = Pos3D({ 0, 0, 0 });
	Pos3D m_projPoint = Pos3D({ 0, 0, 0 });
	float m_pointOnPlaneToProject = 0.0f;
};

#endif