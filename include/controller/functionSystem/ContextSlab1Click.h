#ifndef CONTEXT_SLAB_1_CLICK_H
#define CONTEXT_SLAB_1_CLICK_H

#include "controller/functionSystem/ARayTracingContext.h"
#include <glm/glm.hpp>


class ContextSlab1Click : public ARayTracingContext
{
public:
	ContextSlab1Click(const ContextId& id);
	~ContextSlab1Click();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;

	uint64_t m_extend;
};

#endif // !CONTEXT_SLAB_1_CLICK_H

