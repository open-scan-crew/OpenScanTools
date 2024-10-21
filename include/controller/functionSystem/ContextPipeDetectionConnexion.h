#ifndef CONTEXT_PIPE_DETECTION_CONNEXION_H_
#define CONTEXT_PIPE_DETECTION_CONNEXION_H_

#include "controller/functionSystem/ARayTracingContext.h"
#include "controller/functionSystem/PipeDetectionOptions.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "models/application/List.h"
#include <glm/glm.hpp>
#include <vector>

class CylinderNode;
class TorusNode;

class ContextPipeDetectionConnexion : public ARayTracingContext
{
public:
	ContextPipeDetectionConnexion(const ContextId& id);
	~ContextPipeDetectionConnexion();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	ContextState abort(Controller& controller) override;
	ContextState validate(Controller& controller) override;
	ContextState finishLine(Controller& controller);
	bool canAutoRelaunch() const;


	ContextType getType() const override;
private:
	void resetClickUsages(Controller& controller);

private:
	std::vector<glm::dvec3> m_cylinderDirections;
	std::vector<glm::dvec3> m_cylinderCenters;
	std::vector<double> m_cylinderRadii;
	std::vector<double> m_standardRadii;
	std::vector<SafePtr<StandardList>> m_standards;
	std::vector<double> m_cylinderLengths;
	std::vector<glm::dvec3> m_cylinderDirectionsModif;
	std::vector<glm::dvec3> m_cylinderCentersModif;
	std::vector<double> m_cylinderLengthsModif;
	std::vector<SafePtr<CylinderNode>> m_cylinders;
	std::vector<LineConnectionType> m_connexionType;
	double m_RonDext;
	bool m_angleConstraints;
	PipeDetectionOptions m_options;
	bool m_keepDiameter;
	std::vector<SafePtr<TorusNode>> m_tempElbows;
	std::vector<bool> m_isElbow;
	std::vector<int> m_elbowIndex;
	std::vector<SafePtr<AGraphNode>> m_dataToIgnore;
	std::vector<SafePtr<CylinderNode>> m_GuidModifiedCylinder;
};

#endif // !CONTEXT_PIPE_DETECTION_CONNEXION_H_
