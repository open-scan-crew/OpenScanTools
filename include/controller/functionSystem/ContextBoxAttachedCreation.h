#ifndef CONTEXT_BOX_ATTACHED_CREATION_H
#define CONTEXT_BOX_ATTACHED_CREATION_H

#include "controller/functionSystem/AContextCreateAttached.h"

#include <glm/glm.hpp>

class ContextCreateBoxAttached3Points : public AContextCreateAttached
{
public:
	ContextCreateBoxAttached3Points(const ContextId& id);
	~ContextCreateBoxAttached3Points();

	ContextType getType() const override;
	bool canAutoRelaunch() const override;


private:
	void createObject(Controller& controller, TransformationModule& transfo);

};

class ContextCreateBoxAttached2Points : public ARayTracingContext
{
public:
	ContextCreateBoxAttached2Points(const ContextId& id);
	~ContextCreateBoxAttached2Points();
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;

	ContextType getType() const override;
	bool canAutoRelaunch() const override;

};

#endif
