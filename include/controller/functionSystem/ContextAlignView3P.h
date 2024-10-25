#ifndef CONTEXT_ALIGN_VIEW3P_H_
#define CONTEXT_ALIGN_VIEW3P_H_

#include "controller/functionSystem/AContextAlignView.h"

class ContextAlignView3P : public AContextAlignView
{
public:
	ContextAlignView3P(const ContextId& id);
	~ContextAlignView3P();
	ContextState launch(Controller& controller) override;
	ContextType getType() const override;
};

#endif // !CONTEXT_ALIGN_VIEW_H_
