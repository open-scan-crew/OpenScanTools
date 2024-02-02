#ifndef CLICS_SPHERE4_H
#define CLICS_SPHERE4_H

#include "controller/functionSystem/ARayTracingContext.h"

#include <glm/glm.hpp>

class Context4ClicsSphere : public ARayTracingContext
{
public:
	Context4ClicsSphere(const ContextId& id);
	~Context4ClicsSphere();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;

	bool canAutoRelaunch() const;
	ContextType getType() const override;
};

#endif