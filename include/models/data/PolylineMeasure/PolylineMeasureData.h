#ifndef POLYLINE_MEASURE_DATA_H
#define POLYLINE_MEASURE_DATA_H

#include "models/3d/Measures.h"
#include "models/data/Measure/MeasureData.h"
#include "models/OpenScanToolsModelEssentials.h"

class PolylineMeasureData : public MeasureData
{
public:
	PolylineMeasureData();
	~PolylineMeasureData();

	void copyPolylineMeasureData(const PolylineMeasureData& data);

	void addMeasure(const Measure& mesure);
	void removeMeasureBack();
	void setMeasures(const std::vector<Measure>& measure);

	const Pos3D& getFirstPos() const;
	const Pos3D& getLastPos() const;

	std::vector<Measure> getMeasures() const;

	glm::dvec3 computeAreaOfPolyline() const;

protected:
	std::vector<Measure> m_measures;
};
#endif