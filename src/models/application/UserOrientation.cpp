#include "models/application/UserOrientation.h"
#include "utils/math/trigo.h"

#include "utils/math/glm_extended.h"

UserOrientation::UserOrientation()
{
	m_id = xg::newGuid();
	m_name = "";
	m_order = -1;

	
	m_axisType = UOAxisType::XAxis;
	m_axisPoints = { glm::dvec3({0., 0., 0.}), glm::dvec3({1., 0., 0.}) };
	m_customAxis = { glm::dvec3({0., 0., 0.}), glm::dvec3({1., 0., 0.}) };

	m_newPoint = glm::dvec3(0.0);
	m_oldPoint = glm::dvec3(0.0);
}

UserOrientation::UserOrientation(userOrientationId id)
{
	m_id = id;
	m_name = "";
	m_order = -1;


	m_axisType = UOAxisType::XAxis;
	m_axisPoints = { glm::dvec3({0., 0., 0.}), glm::dvec3({1., 0., 0.}) };
	m_customAxis = { glm::dvec3({0., 0., 0.}), glm::dvec3({1., 0., 0.}) };

	m_newPoint = glm::dvec3(0.0);
	m_oldPoint = glm::dvec3(0.0);
} 

UserOrientation::UserOrientation(const UserOrientation & uo)
{
	m_id = uo.getId();
	m_name = uo.getName();
	m_order = uo.getOrder();

	m_axisType = uo.getAxisType();
	m_axisPoints = uo.getAxisPoints();
	m_customAxis = uo.getCustomAxis();

	m_newPoint = uo.getNewPoint();
	m_oldPoint = uo.getOldPoint();
	
}

UserOrientation::~UserOrientation()
{
}

void UserOrientation::setId(const userOrientationId& id)
{
	m_id = id;
}

void UserOrientation::setName(const QString& newName)
{
	m_name = newName;
}

void UserOrientation::setCustomAxis(const std::array<glm::dvec3, 2>& customAxis)
{
	m_customAxis = customAxis;
}

void UserOrientation::setAxisType(const UOAxisType& type)
{
	m_axisType = type;
}

void UserOrientation::setPoint1(const glm::dvec3& point)
{
	m_axisPoints[0] = point;
}

void UserOrientation::setPoint2(const glm::dvec3& point)
{
	m_axisPoints[1] = point;
}

void UserOrientation::setOrder(uint32_t order)
{
	m_order = order;
}

userOrientationId UserOrientation::getId() const
{
	return m_id;
}

const QString & UserOrientation::getName() const
{
	return m_name;
}

const std::array<glm::dvec3, 2>& UserOrientation::getAxisPoints() const
{
	return m_axisPoints;
}

const std::array<glm::dvec3, 2>& UserOrientation::getCustomAxis() const
{
	return m_customAxis;
}

const UOAxisType& UserOrientation::getAxisType() const
{
	return m_axisType;
}

uint32_t UserOrientation::getOrder() const
{
	return m_order;
}

double UserOrientation::getAngle() const
{
	double dx = (m_axisPoints[1].x - m_axisPoints[0].x);
	double dy = (m_axisPoints[1].y - m_axisPoints[0].y);

	double cdx = (m_customAxis[1].x - m_customAxis[0].x);
	double cdy = (m_customAxis[1].y - m_customAxis[0].y);
	
	std::cout << "Angle : " << (atan2(dy, dx) - atan2(cdy, cdx)) * 180/M_PI << std::endl;
	return atan2(dy, dx) - atan2(cdy, cdx);

}

const glm::dvec3& UserOrientation::getOldPoint() const
{
	return m_oldPoint;
}

const glm::dvec3& UserOrientation::getNewPoint() const
{
	return m_newPoint;
}

void UserOrientation::setOldPoint(const glm::dvec3& oldPoint)
{
	m_oldPoint = oldPoint;
}

void UserOrientation::setNewPoint(const glm::dvec3& newPoint)
{
	m_newPoint = newPoint;
}
