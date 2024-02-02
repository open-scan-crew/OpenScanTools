#ifndef CONTEXT_SIMPLE_MEASURE_H_
#define CONTEXT_SIMPLE_MEASURE_H_

#include "controller/functionSystem/ContextPointsMeasure.h"

class ContextSimpleMeasure : public ContextPointsMeasure
{
public:
	ContextSimpleMeasure(const ContextId& id);
	~ContextSimpleMeasure();
	ContextState launch(Controller& controller);
	ContextType getType() const override;
};


#endif //! CONTEXT_SIMPLE_MEASURE_H_