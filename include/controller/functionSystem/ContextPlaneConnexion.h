#ifndef CONTEXT_PLANE_CONNEXION_H_
#define CONTEXT_PLANE_CONNEXION_H_

#include "controller/functionSystem/ARayTracingContext.h"
#include "models/pointCloud/TLS.h"
#include "controller/controls/ControlFunctionClipping.h"
#include "pointCloudEngine/OctreeRayTracing.h"
#include "models/graph/BoxNode.h"

#include <glm/glm.hpp>
#include <queue>

class ContextPlaneConnexion : public ARayTracingContext
{
public:
	ContextPlaneConnexion(const ContextId& id);
	~ContextPlaneConnexion();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	virtual ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;

protected:
	std::vector<SafePtr<BoxNode>> m_planes;
	std::vector<RectangularPlane> m_rectPlanes;
	std::vector<glm::dvec3> m_points;
	uint64_t	           m_lastPointInd;
};

#endif // !CONTEXT_PLANE_CONNEXION_H_
