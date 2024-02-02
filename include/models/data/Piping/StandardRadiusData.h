#ifndef STANDARD_RADIUS_DATA_H_
#define STANDARD_RADIUS_DATA_H_

#include "models/OpenScanToolsModelEssentials.h"
#include "models/data/Piping/InsulationData.h"
#include "models/application/List.h"

class StandardRadiusData : public InsulationData
{
public:
	enum class DiameterSet { Standard, Forced, Detected };
public:
	StandardRadiusData(const double& detectedRadius);
	StandardRadiusData();
	~StandardRadiusData();

	void copyStandardRadiusData(const StandardRadiusData& data);
	
	void setDetectedRadius(const double& radius);
	void setForcedRadius(const double& radius);
	void setStandard(const SafePtr<StandardList>& standard);
	void setDiameterSet(const DiameterSet& diameter);

	void checkStandard();

	const SafePtr<StandardList>& getStandard() const;
	double getDetectedRadius() const;
	double getForcedRadius() const;
	double getStandardRadius() const;
	DiameterSet getDiameterSet() const;

protected:
	double m_standardRadius;
	double m_forcedRadius;
	double m_detectedRadius;
	DiameterSet m_diameterSet;
	SafePtr<StandardList> m_standard;
};

#endif // !STANDARD_RADIUS_DATA_H_ 
