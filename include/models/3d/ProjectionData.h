#ifndef PROJECTION_DATA_H
#define PROJECTION_DATA_H

#include "pointCloudEngine/RenderingTypes.h"

#include <glm/glm.hpp>

constexpr double MIN_FOVY = glm::radians(3.0);
constexpr double MAX_FOVY = glm::radians(120.0);

constexpr double MIN_HEIGHT = 0.2;
constexpr double MAX_HEIGHT = 512000.0;


struct ProjectionFrustum
{
	double l;
	double r;
	double b;
	double t;
	double n;
	double f;
};

class ProjectionData
{
public:
	ProjectionData();
	ProjectionData(const ProjectionMode& mode, const ProjectionFrustum& box);
	~ProjectionData();

	ProjectionMode getProjectionMode() const;
	ProjectionFrustum getProjectionFrustum() const;
	glm::dvec4 getEyeCoord(double xf, double yf, double depth, double width, double height) const;

	double getFovx() const;
	double getFovy() const;
	glm::vec2 getPixelSize1m(uint32_t screenWidth, uint32_t screenHeight) const;
	double getHeightAt1m() const;
	double getWidthAt1m() const;
	double getFar() const;
	double getNear() const;
	double getRatioW_H() const;
	ProjectionData getTruncatedProjection(double ratio, double margin) const;

	glm::dmat4 getProjMatrix() const;

	void setProjectionData(const ProjectionData& ref);
	void setFovy(float _fovy);
	void setOrthoHeight(double _height);
	void setPerspectiveZBounds(PerspectiveZBounds zBounds);
	void setOrthographicZBounds(OrthographicZBounds zBounds);
	void setRatioW_H(double ratio);
	void initPerspective(double _fovy, double _nearZ, double _farZ, double _ratioWH);
	void initOrthographic(double _realH, double _nearZ, double _farZ, double _ratioWH);

protected:
	ProjectionMode	m_projectionMode;
	ProjectionFrustum	m_projectionFrustum;
};

#endif // !CAMERA_PARAMETERS_H