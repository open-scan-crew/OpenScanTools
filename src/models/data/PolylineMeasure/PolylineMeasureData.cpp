#include "models/data/PolylineMeasure/PolylineMeasureData.h"

PolylineMeasureData::PolylineMeasureData()
{}

PolylineMeasureData::~PolylineMeasureData()
{}

void PolylineMeasureData::copyPolylineMeasureData(const PolylineMeasureData& data)
{
	m_measures = data.getMeasures();
}

void PolylineMeasureData::addMeasure(const Measure& mesure)
{
	m_measures.push_back(mesure);
}

void PolylineMeasureData::removeMeasureBack()
{
	m_measures.pop_back();
}

void PolylineMeasureData::setMeasures(const std::vector<Measure>& measures)
{
	m_measures = measures;
}

const Pos3D& PolylineMeasureData::getFirstPos() const
{
	return (m_measures.front().origin);
}

const Pos3D& PolylineMeasureData::getLastPos() const
{
	return (m_measures.back().final);
}

std::vector<Measure> PolylineMeasureData::getMeasures() const
{
	return m_measures;
}

glm::dvec3 PolylineMeasureData::computeAreaOfPolyline() const
{

	std::vector<glm::dvec3> points;
	int size = (int)m_measures.size();
	if (size < 2)			//not enough points to compute area
		return glm::dvec3(0.0, 0.0, 0.0);

	for (int i = 0; i < (int)m_measures.size(); i++)
	{
		points.push_back(m_measures[i].origin);
	}
	points.push_back(m_measures[size - 1].final);


	points.push_back(points[0]);
	double areaX(0), areaY(0), areaZ(0);
	for (int i = 0; i < (int)points.size() - 1; i++)
	{
		areaX += 0.5 * (points[i].y * points[i + 1].z - points[i + 1].y * points[i].z);
		areaY += 0.5 * (points[i].x * points[i + 1].z - points[i + 1].x * points[i].z);
		areaZ += 0.5 * (points[i + 1].x * points[i].y - points[i + 1].y * points[i].x);
	}
	return glm::dvec3(abs(areaX), abs(areaY), abs(areaZ));
}