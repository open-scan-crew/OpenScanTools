#ifndef GRIDDATA_H_
#define GRIDDATA_H_

#include <glm/glm.hpp>

enum class GridType
{
	ByStep,
	ByMultiple
};

class GridData
{
public:
	GridData();
	~GridData();

	void copyGridData(const GridData& grid);

	void setGridType(const GridType& type);
	void setGridDivision(const glm::vec3& division);

	const glm::vec3& getGridDivision() const;
	GridType getGridType() const;

protected:
	GridType	m_gridType;
	glm::vec3	m_gridDivision;
};
#endif // !SETTERGRIDDATA_H_
