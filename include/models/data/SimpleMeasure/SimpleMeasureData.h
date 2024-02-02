#ifndef SIMPLEMEASUREDATA_H_
#define SIMPLEMEASUREDATA_H_

#include "models/3d/Measures.h"
#include "models/data/Measure/MeasureData.h"
#include "models/OpenScanToolsModelEssentials.h"

class SimpleMeasureData : public MeasureData
{
public:
	SimpleMeasureData();
	~SimpleMeasureData();

	void copySimpleMeasureData(const SimpleMeasureData& data);

	void setOriginPos(const Pos3D& pos);
	void setDestinationPos(const Pos3D& pos);
	void setMeasure(Measure measure);

	const Pos3D& getOriginPos() const;
	const Pos3D& getDestinationPos() const;
	const Measure& getMeasure() const;

	std::vector<Measure> getMeasures() const override;

protected:
	Measure m_measure = { Pos3D(), Pos3D() };
};

#endif