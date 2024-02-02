#ifndef PIPETOPIPEMEASUREDATA_H_
#define PIPETOPIPEMEASUREDATA_H_

#include "models/OpenScanToolsModelEssentials.h"
#include "models/data/Measure/MeasureData.h"
#include "models/3d/Measures.h"

class PipeToPipeMeasureData : public MeasureData
{
public:
	PipeToPipeMeasureData();
	~PipeToPipeMeasureData();

	void copyPipeToPipeMeasureData(const PipeToPipeMeasureData& data);

	void setCenterP1ToAxeP2(float centerP1ToAxeP2);
	void setP1ToP2Horizontal(float P1ToP2Horizontal);
	void setP1ToP2Vertical(float P1ToP2Vertical);
	void setFreeDist(float freeD);
	void setFreeDistHorizontal(float freeDHorizontal);
	void setFreeDistVertical(float freeDVertical);
	void setTotalFootprint(float totalF);

	void setPipe1Diameter(float pipe1Diameter);
	void setPipe2Diameter(float pipe2Diameter);
	void setPipe1Center(const Pos3D& pipe1Center);
	void setPipe2Center(const Pos3D& pipe2Center);
	void setProjPoint(const Pos3D& projPoint);
	void setPipe2CenterToProj(float pipe2CenterToProj);


	float getCenterP1ToAxeP2() const;
	float getP1ToP2Horizontal() const;
	float getP1ToP2Vertical() const;
	float getFreeDist() const;
	float getFreeDistHorizontal() const;
	float getFreeDistVertical() const;
	float getTotalFootprint() const;

	float getPipe1Diameter() const;
	float getPipe2Diameter() const;
	const Pos3D& getPipe1Center() const;
	const Pos3D& getPipe2Center() const;
	const Pos3D& getProjPoint() const;
	float getPipe2CenterToProj() const;

	Measure getFirstMeasure() const;
	Measure getSecondMeasure() const;
	std::vector<Measure> getMeasures() const override;

protected:
	float m_centerP1ToAxeP2 = 0.0f;
	float m_P1ToP2Horizontal = 0.0f;
	float m_P1ToP2Vertical = 0.0f;
	float m_freeDist = 0.0f;
	float m_freeDistHorizontal = 0.0f;
	float m_freeDistVertical = 0.0f;
	float m_totalFootprint = 0.0f;

	float m_pipe1Diameter = 0.0f;
	float m_pipe2Diameter = 0.0f;
	Pos3D m_pipe1Center = Pos3D({ 0, 0, 0 });
	Pos3D m_pipe2Center = Pos3D({ 0, 0, 0 });
	Pos3D m_projPoint = Pos3D({ 0, 0, 0 });
	float m_pipe2CenterToProj = 0.0f;
};

#endif