#ifndef CONTEXT_ALIGN_VIEW2P_H_
#define CONTEXT_ALIGN_VIEW2P_H_

#include "controller/functionSystem/AContextAlignView.h"

#include <glm/glm.hpp>
#include <deque>

class ContextAlignView2P : public AContextAlignView
{
public:
	ContextAlignView2P(const ContextId& id);
	~ContextAlignView2P();
	ContextState launch(Controller& controller) override;
	ContextType getType() const override;
};

#endif // !CONTEXT_ALIGN_VIEW_H_
