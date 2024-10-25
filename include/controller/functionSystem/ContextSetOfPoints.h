#ifndef CONTEXT_SET_OF_POINTS_H_
#define CONTEXT_SET_OF_POINTS_H_

#include "controller/functionSystem/ARayTracingContext.h"
#include "controller/functionSystem/PlaneDetectionOptions.h"


class ContextSetOfPoints : public ARayTracingContext
{
public:
	ContextSetOfPoints(const ContextId& id);
	~ContextSetOfPoints();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;

	SetOfPointsOptions m_options;
	bool m_userAxes, m_createMeasures, m_fromTop, m_horizontal;
	double m_threshold, m_step;

private:
	void  resetClickUsages();
};


#endif // !CONTEXT_SET_OF_POINTS_H_
