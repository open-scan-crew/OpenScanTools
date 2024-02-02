#ifndef POINTTOPLANEMEASUREDATA_H_
#define POINTTOPLANEMEASUREDATA_H_

#include "models/OpenScanToolsModelEssentials.h"
#include "models/data/Measure/MeasureData.h"
#include "models/3d/Measures.h"

class PointToPlaneMeasureData : public MeasureData
{
public:
	PointToPlaneMeasureData();
	~PointToPlaneMeasureData();

	void copyPointToPlaneMeasureData(const PointToPlaneMeasureData& data);

	void setPointToPlaneD(float pointToPlaneD);
	void setHorizontal(float horizontal);
	void setVertical(float vertical);

	void setPointCoord(const Pos3D& pointCoord);
	void setpointOnPlane(const Pos3D& pointOnPlane);
	void setNormalToPlane(const Pos3D& normalToPlane);
	void setProjPoint(const Pos3D& projPoint);

	float getPointToPlaneD() const;
	float getHorizontal() const;
	float getVertical() const;
	float getPointProjToPlaneD() const;

	const Pos3D& getPointCoord() const;
	const Pos3D& getPointOnPlane() const;
	const Pos3D& getNormalToPlane() const;
	const Pos3D& getProjPoint() const;

	Measure getFirstMeasure() const;
	Measure getSecondMeasure() const;
	std::vector<Measure> getMeasures() const override;

protected:
	float m_pointToPlaneD = 0.0f;
	float m_horizontal = 0.0f;
	float m_vertical = 0.0f;

	Pos3D m_pointCoord = Pos3D({ 0,0,0 });
	Pos3D m_pointOnPlane = Pos3D({ 0,0,0 });
	Pos3D m_normalToPlane = Pos3D({ 0,0,0 });
	Pos3D m_ProjPoint = Pos3D({ 0,0,0 });
};
#endif