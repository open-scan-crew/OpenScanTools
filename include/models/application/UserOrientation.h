#ifndef USERORIENTATION_LIST_H_
#define USERORIENTATION_LIST_H_

#include <QtWidgets/qwidget.h>
#include <set>
#include <glm/glm.hpp>

#include "crossguid/guid.hpp"

typedef xg::Guid userOrientationId;
enum class UOAxisType {XAxis, YAxis, Custom};

class UserOrientation
{
public:
	UserOrientation();
	UserOrientation(userOrientationId id);
	UserOrientation(const UserOrientation& uo);
	~UserOrientation();

	userOrientationId getId() const;
	const QString& getName() const;
	uint32_t getOrder() const;

	void setId(const userOrientationId& id);
	void setName(const QString& newName);
	void setOrder(uint32_t order);

	//Orientation :
	const std::array<glm::dvec3, 2>& getAxisPoints() const;
	const std::array<glm::dvec3, 2>& getCustomAxis() const;
	const UOAxisType& getAxisType() const;
	//Angle around Z axis
	double getAngle() const;

	void setCustomAxis(const std::array<glm::dvec3, 2>& customAxis);
	void setAxisType(const UOAxisType& type);
	void setPoint1(const glm::dvec3& point);
	void setPoint2(const glm::dvec3& point);

	//Translation :
	const glm::dvec3& getOldPoint() const;
	const glm::dvec3& getNewPoint() const;

	void setOldPoint(const glm::dvec3& oldPoint);
	void setNewPoint(const glm::dvec3& newPoint);

private:
	QString m_name;
	userOrientationId m_id;
	uint32_t m_order;

	//Orientation Data
	std::array<glm::dvec3, 2> m_axisPoints;
	std::array<glm::dvec3, 2> m_customAxis;
	UOAxisType m_axisType;

	//Translation Data
	glm::dvec3 m_oldPoint;
	glm::dvec3 m_newPoint;
};

#endif // !USERORIENTATION_LIST_H_