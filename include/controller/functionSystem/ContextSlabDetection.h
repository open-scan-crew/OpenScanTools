#ifndef CONTEXT_SLAB_DETECTION_H
#define CONTEXT_SLAB_DETECTION_H

#include "controller/functionSystem/ARayTracingContext.h"
#include <glm/glm.hpp>


class ContextSlabDetection : public ARayTracingContext
{
public:
	ContextSlabDetection(const ContextId& id);
	~ContextSlabDetection();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;

	uint64_t m_extend;
};

#endif // !CONTEXT_SLAB_DETECTION_H
