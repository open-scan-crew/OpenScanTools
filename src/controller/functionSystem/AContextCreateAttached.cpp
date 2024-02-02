#include "controller/functionSystem/AContextCreateAttached.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "utils/math/trigo.h"
#include "gui/texts/RayTracingTexts.hpp"

#include "models/3d/Graph/CameraNode.h"

#include "utils/Logger.h"

AContextCreateAttached::AContextCreateAttached(const ContextId& id)
	: AContextAlignView(id)
{
    // Set the description of the points needed for the processing
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_RAYTRACING_DEFAULT_1 });
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_RAYTRACING_DEFAULT_2 });
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_RAYTRACING_DEFAULT_3 });
}

AContextCreateAttached::~AContextCreateAttached()
{}

void AContextCreateAttached::CalculateCenterAndScale(const glm::dvec3& normale, const glm::dquat& rot, glm::dvec3& center, glm::dvec3& scale)
{
    scale = glm::dvec3(0.0);
    center = glm::dvec3(0.0);
    glm::dmat3 m = glm::transpose(glm::mat3_cast(rot));

    for (int i = 0; i < m_clickResults.size(); i++)
    {
        for (int j = i + 1; j < m_clickResults.size(); j++)
        {

            glm::dvec3 IinvRot = m * m_clickResults[i].position;
            glm::dvec3 JinvRot = m * m_clickResults[j].position;
            glm::dvec3 diff = glm::abs(IinvRot - JinvRot);

            if (diff.x > scale.x)
            {
                scale.x = diff.x;
                center.x = (IinvRot.x + JinvRot.x) / 2.;
            }

            if (diff.y > scale.y)
            {
                scale.y = diff.y;
                center.y = (IinvRot.y + JinvRot.y) / 2.;
            }
        }
    }
    center.z = (m * m_clickResults[0].position).z;
    center = glm::mat3_cast(rot) * center;
}

ContextState AContextCreateAttached::feedMessage(IMessage* message, Controller& controller)
{
    AContextAlignView::feedMessage(message, controller);
    return m_state;
}

ContextState AContextCreateAttached::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
    if (pointMissing())
        return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

    ReadPtr<CameraNode> rCam = m_cameraNode.cget();
    if (!rCam)
        return abort(controller);

	glm::dvec3	A(glm::normalize(m_clickResults[1].position - m_clickResults[0].position)),
				B(glm::normalize(m_clickResults[2].position - m_clickResults[0].position)),
				C(m_clickResults[0].position - rCam->getCenter()),
				N(glm::normalize(glm::cross(A, B)));

	if (glm::dot(N, C) > 0)
		N = -N;

    // X_1 = cross(Y_1, Z_1)
    // Y_1 = cross(Z_0, Z_2)
    // Z_1 = Z_0 = Z(0., 0., 1.)
    glm::dvec3 Y_1 = glm::cross(glm::dvec3(0., 0., 1.), N);
    if (Y_1.length() < 0.001)
        Y_1 = glm::dvec3(0., 1., 0.);
    else
        Y_1 = glm::normalize(Y_1);
    glm::dvec3 X_1 = glm::cross(Y_1, glm::dvec3(0., 0., 1.));

    //     Y_2 = Y_1
    //     Z_2 = N
    // ==> X_2 = glm::cross(Y_2, Z_2) = glm::cross(Y_1, N)
    glm::dvec3 X_2 = glm::cross(Y_1, N);

    glm::dquat q0(glm::dvec3(1., 0., 0.), X_1);
    //glm::dquat q0(tls::math::quat_from_3_vector(glm::dvec3(1., 0., 0.), X_1, glm::dvec3(0., 0., 1.)));
    glm::dquat q1(X_1, X_2);
    //glm::dquat q1(tls::math::quat_from_3_vector(X_1, X_2, Y_1));
    Logger::log(LoggerMode::FunctionLog) << "First quat: " << q0.x << ", " << q0.y << ", " << q0.z << ", " << q0.w << Logger::endl;
    Logger::log(LoggerMode::FunctionLog) << "Second quat: " << q1.x << ", " << q1.y << ", " << q1.z << ", " << q1.w << Logger::endl;
    glm::dquat qt = q1 * q0;
    Logger::log(LoggerMode::FunctionLog) << "Total quat: " << qt.x << ", " << qt.y << ", " << qt.z << ", " << qt.w << Logger::endl;
    //glm::dquat q2(glm::dvec3(0.0, 0.0, 1.0), N);
	
	TransformationModule mod;
    mod.setRotation(qt);
    //mod.setRotation(q2);
    glm::dvec3 center;
    glm::dvec3 scale;
    CalculateCenterAndScale(N, qt, center, scale);

    const ClippingBoxSettings& settings = controller.getContext().CgetClippingSettings();
    scale.z = settings.size.z;
    glm::dvec3 up(glm::dvec3(scale.z * N.x * 0.5, scale.z * N.y * 0.5, scale.z * N.z * 0.5));
	switch (settings.offset)
	{
		case ClippingBoxOffset::Topface:
			mod.setPosition(center - up);
			break;
		case ClippingBoxOffset::BottomFace:
			mod.setPosition(center + up);
			break;
		case ClippingBoxOffset::CenterOnPoint:
			mod.setPosition(center);
			break;
	}
    mod.setScale(scale / 2.);
	createObject(controller, mod);

    m_clickResults.clear();
	return waitForNextPoint(controller);
}