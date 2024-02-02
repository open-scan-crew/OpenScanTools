#ifndef CONTEXT_TRAJECTORY_H
#define CONTEXT_TRAJECTORY_H

#include "controller/functionSystem/ARayTracingContext.h"
#include <glm/glm.hpp>

class ContextTrajectory : public ARayTracingContext
{
public:
	ContextTrajectory(const ContextId& id);
	~ContextTrajectory();
	void resetClickUsages();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;
};

#endif //CONTEXT_TRAJECTORY_H