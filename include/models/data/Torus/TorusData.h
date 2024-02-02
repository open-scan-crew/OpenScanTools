#ifndef TORUSDATA_H_
#define TORUSDATA_H_

#include <glm/glm.hpp>
#include "models/data/Piping/InsulationData.h"

class TorusData : public InsulationData
{
public:
	TorusData(const double& mainAngle, const double& mainRadius, const double& tubeRadius);
	~TorusData();

	void copyTorusData(const TorusData& torus);

	void setMainRadius(const double& radius);
	void setMainAngle(const double& angle);
	void setTubeRadius(const double& radius);

	const double& getMainRadius() const;
	const double& getMainAngle() const;
	const double& getTubeRadius() const;

	double getAdjustedTubeRadius() const;

	glm::vec3 calculateOffset() const;

protected:
	double	m_mainRadius;
	double	m_mainAngle;
	double	m_tubeRadius;
};


#endif // !SETTERTORUSDATA_H_
