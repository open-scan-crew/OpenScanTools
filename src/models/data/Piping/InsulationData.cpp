#include "models/data/Piping/InsulationData.h"

InsulationData::InsulationData()
	: m_insulationRadius(0.)
{
}

InsulationData::~InsulationData()
{
}

void InsulationData::setInsulationRadius(const double& radius)
{
	m_insulationRadius = radius;
}

double InsulationData::getInsulationRadius() const
{
	return m_insulationRadius;
}
