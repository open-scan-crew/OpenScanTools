#include "models/data/Grid/GridData.h"

GridData::GridData()
	: m_gridDivision(glm::vec3(1.0f))
	, m_gridType(GridType::ByMultiple)
{}

GridData::~GridData()
{}

void GridData::copyGridData(const GridData& grid)
{
	m_gridType = grid.getGridType();
	m_gridDivision = grid.getGridDivision();
}

void GridData::setGridType(const GridType& type)
{
	m_gridType = type;
}

void GridData::setGridDivision(const glm::vec3& division)
{
	m_gridDivision = division;
}

const glm::vec3& GridData::getGridDivision() const
{
	return m_gridDivision;
}

GridType GridData::getGridType() const
{
	return m_gridType;
}