#ifndef MEASURE_DATA_H
#define MEASURE_DATA_H

#include "models/3d/Measures.h"
#include <vector>

class MeasureData
{
public:
	MeasureData() {};
	~MeasureData() {};

	virtual std::vector<Measure> getMeasures() const = 0;
};
#endif