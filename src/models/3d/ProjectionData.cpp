#include "models/3d/ProjectionData.h"
#include "models/3d/OpticalFunctions.h"

ProjectionData::ProjectionData()
	: m_projectionMode(ProjectionMode::Perspective)
	, m_projectionFrustum{ -0.8, 0.8, -0.5, 0.5, 0.125, 1024.0 }
{
	setFovy(1.047f);
}

ProjectionData::ProjectionData(const ProjectionMode& mode, const ProjectionFrustum& box)
	: m_projectionMode(mode)
	, m_projectionFrustum(box)
{}

ProjectionData::~ProjectionData()
{}

ProjectionMode ProjectionData::getProjectionMode() const
{
	return m_projectionMode;
}

ProjectionFrustum ProjectionData::getProjectionFrustum() const
{
	return m_projectionFrustum;
}

glm::dvec4 ProjectionData::getEyeCoord(double xf, double yf, double depth, double width, double height) const
{
	double xd = (2 * xf / width) - 1.0;
	double yd = (2 * yf / height) - 1.0;

	glm::dvec4 eyeCoord;
	const ProjectionFrustum& p = m_projectionFrustum;

	if (m_projectionMode == ProjectionMode::Perspective)
	{
		double m00 = 2 * p.n / (p.r - p.l);
		double m11 = 2 * p.n / (p.t - p.b);

		//eyeCoord.z = farZ * nearZ / (farZ - (farZ - nearZ) * depth);
		eyeCoord.z = p.n / (1.0 - (1.0 - p.n / p.f) * depth);
		eyeCoord.x = xd * eyeCoord.z / m00;
		eyeCoord.y = yd * eyeCoord.z / m11;
		eyeCoord.w = 1.0;
	}
	else
	{
		eyeCoord.x = (xd * (p.r - p.l) + p.l + p.r) / 2;
		eyeCoord.y = (yd * (p.t - p.b) + p.t + p.b) / 2;
		eyeCoord.z = depth * (p.f - p.n) + p.n;
		eyeCoord.w = 1.0;
	}

	return (eyeCoord);
}

double ProjectionData::getFovx() const
{
	const ProjectionFrustum& p = m_projectionFrustum;
	if (m_projectionMode == ProjectionMode::Perspective)
		return (atan(p.r / p.n) + atan(-p.l / p.n));
	else
		return (0.0);
}

double ProjectionData::getFovy() const
{
	const ProjectionFrustum& p = m_projectionFrustum;
	if (m_projectionMode == ProjectionMode::Perspective)
		return (atan(p.t / p.n) + atan(-p.b / p.n));
	else
		return (0.0);
}

// Note: the pixel size is given at a distance of 1 meter.
glm::vec2 ProjectionData::getPixelSize1m(uint32_t screenWidth, uint32_t screenHeight) const
{
	const ProjectionFrustum& p = m_projectionFrustum;
	if (m_projectionMode == ProjectionMode::Perspective)
		return glm::vec2((p.r - p.l) / (p.n * screenWidth), (p.t - p.b) / (p.n * screenHeight));
	else
		return glm::vec2((p.r - p.l) / (float)screenWidth, (p.t - p.b) / (float)screenHeight);
}

double ProjectionData::getHeightAt1m() const
{
	const ProjectionFrustum& p = m_projectionFrustum;
	if (m_projectionMode == ProjectionMode::Perspective)
		return ((p.t - p.b) / p.n);
	else
		return (p.t - p.b);
}

double ProjectionData::getWidthAt1m() const
{
	const ProjectionFrustum& p = m_projectionFrustum;
	if (m_projectionMode == ProjectionMode::Perspective)
		return ((p.r - p.l) / p.n);
	else
		return (p.r - p.l);
}

double ProjectionData::getFar() const
{
	return m_projectionFrustum.f;
}

double ProjectionData::getNear() const
{
	return m_projectionFrustum.n;
}

double ProjectionData::getRatioW_H() const
{
	const ProjectionFrustum& p = m_projectionFrustum;
	return ((p.r - p.l) / (p.t - p.b));
}

ProjectionData ProjectionData::getTruncatedProjection(double _ratio, double _margin) const
{
	ProjectionData result = *this;
	const ProjectionFrustum& srcP = m_projectionFrustum;
	ProjectionFrustum& dstP = result.m_projectionFrustum;

	double margin_tb;
	double margin_rl;
	double src_tb = srcP.t - srcP.b;
	double src_rl = srcP.r - srcP.l;
	if (src_rl / src_tb > _ratio)
	{
		margin_tb = src_tb * _margin;
		margin_rl = (src_rl - (src_tb - margin_tb * 2.0) * _ratio) / 2.0;
	}
	else
	{
		margin_rl = src_rl * _margin;
		margin_tb = (src_tb  - (src_rl - margin_rl * 2.0) / _ratio) / 2.0;
	}

	dstP.t -= margin_tb;
	dstP.b += margin_tb;
	dstP.r -= margin_rl;
	dstP.l += margin_rl;

	return result;
}

// Return a perspective projection matrix with the Vulkan convention
// The ProjectionData is looking toward the +z axis
// top = +y ; bottom = -y
// right = +x ; left = -x
glm::dmat4 ProjectionData::getProjMatrix() const
{
	const ProjectionFrustum& p = m_projectionFrustum;
	const double& r = p.r;
	const double& l = p.l;
	const double& t = p.t;
	const double& b = p.b;
	const double& n = p.n;
	const double& f = p.f;
	if (m_projectionMode == ProjectionMode::Perspective)
	{
		return { 2 * n / (r - l), 0, 0, 0,
				 0, 2 * n / (t - b), 0, 0,
				 (l + r) / (l - r), (b + t) / (b - t), f / (f - n), 1,
				 0, 0, -f * n / (f - n), 0 };
	}
	else
	{
		return { 2 / (r - l), 0, 0, 0,
				 0, 2 / (t - b), 0, 0,
				 0, 0, 1 / (f - n), 0,
				 (l + r) / (l - r), (b + t) / (b - t), n / (n - f), 1 };
	}
}

void ProjectionData::setProjectionData(const ProjectionData& ref)
{
	m_projectionMode = ref.m_projectionMode;
	m_projectionFrustum = ref.m_projectionFrustum;
}

void ProjectionData::setFovy(float _fovy)
{
	ProjectionFrustum& p = m_projectionFrustum;
	double ratio = ((p.r - p.l) / (p.t - p.b));
	p.t = tan(_fovy / 2.0) * p.n;
	p.b = -p.t;
	p.r = p.t * ratio;
	p.l = -p.r;
}

void ProjectionData::setOrthoHeight(double _height)
{
	ProjectionFrustum& p = m_projectionFrustum;
	double ratio = ((p.r - p.l) / (p.t - p.b));
	p.t = _height / 2.0;
	p.b = -p.t;
	p.r = p.t * ratio;
	p.l = -p.r;
}

void ProjectionData::setPerspectiveZBounds(PerspectiveZBounds zBounds)
{
	double near = tls::getNearValue(zBounds);
	ProjectionFrustum& p = m_projectionFrustum;
	if (m_projectionMode == ProjectionMode::Perspective)
	{
		p.t = p.t / p.n * near;
		p.b = -p.t;
		p.r = p.r / p.n * near;
		p.l = -p.r;
	}

	p.n = near;
	p.f = tls::getFarValue(zBounds);
}

void ProjectionData::setOrthographicZBounds(OrthographicZBounds zBounds)
{
	m_projectionFrustum.f = tls::getOrthographicZBoundsValue(zBounds);
	m_projectionFrustum.n = -m_projectionFrustum.f;
}

void ProjectionData::setRatioW_H(double _ratio)
{
	ProjectionFrustum& p = m_projectionFrustum;
	p.r = p.t * _ratio;
	p.l = -p.r;
}

void ProjectionData::initPerspective(double _fovy, double _nearZ, double _farZ, double _ratioWH)
{
	m_projectionMode = ProjectionMode::Perspective;

	ProjectionFrustum& p = m_projectionFrustum;
	p.n = _nearZ;
	p.f = _farZ;
	p.t = tan(_fovy / 2.0) * _nearZ;
	p.b = -p.t;
	p.r = p.t * _ratioWH;
	p.l = -p.r;
}

void ProjectionData::initOrthographic(double _realH, double _nearZ, double _farZ, double _ratioWH)
{
	m_projectionMode = ProjectionMode::Orthographic;

	ProjectionFrustum& p = m_projectionFrustum;
	p.n = _nearZ;
	p.f = _farZ;
	p.t = _realH / 2.0;
	p.b = -_realH / 2.0;
	p.r = p.t * _ratioWH;
	p.l = -p.r;
}


//
//void CameraParametersSetter::setScreenRatio(int _screenW, int _screenH)
//{
//	if (_screenW <= 0 || _screenH <= 0)
//		// invalid parameters
//		return;
//
//	m_screenRatio = (double)_screenW / (double)_screenH;
//
//	// Maintain the screenRatio for both projection mode
//	for (size_t m = 0; m < (size_t)ProjectionMode::MAX_ENUM; ++m)
//	{
//		ProjectionFrustum& p = m_projectionBox;
//		p.r = p.t * m_screenRatio;
//		p.l = -p.r;
//	}
//}
//
