#ifndef CONTEXT_POINTS_MEASURE_H_
#define CONTEXT_POINTS_MEASURE_H_

#include "controller/functionSystem/ARayTracingContext.h"
#include "models/pointCloud/TLS.h"
#include "models/3d/Measures.h"

#include <glm/glm.hpp>
#include <queue>

class PolylineMeasureNode;

struct PolyLineOptions;

class ContextPointsMeasure : public ARayTracingContext
{
public:
	ContextPointsMeasure(const ContextId& id);
	~ContextPointsMeasure();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	virtual ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;

private:
	void applyPolylineOption(const glm::dvec3& origin, glm::dvec3& next, const PolyLineOptions& options);

protected:
	//uint32_t					m_pointsCount;
	//std::deque<glm::dvec3> m_points; // Use ARayTracingContext::m_clickResults
	//std::deque<Measure>			m_measures;
	//NOTE (Aurélien) POC Undo/Redo context
	//std::deque<Measure>			m_measures_history;
	std::vector<glm::dvec3> m_points;
	uint64_t	           m_lastPointInd;
private:
	SafePtr<PolylineMeasureNode> 	m_polylineToEdit;
	bool    m_isPolylineCreated;
};

#endif // !CONTEXT_POINTS_MEASURE_H_
