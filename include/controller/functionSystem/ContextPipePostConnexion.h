#ifndef CONTEXT_PIPE_POST_CONNEXION_H_
#define CONTEXT_PIPE_POST_CONNEXION_H_

#include "controller/functionSystem/AContext.h"
#include <glm/glm.hpp>
#include <vector>

class ContextPipePostConnexion : public AContext
{
public:
	ContextPipePostConnexion(const ContextId& id);
	~ContextPipePostConnexion();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;
	ContextState abort(Controller& controller) override;
	ContextState validate(Controller& controller) override;

	bool noisy;
	bool manualExtend;
	bool autoExtend;
	bool optimized;

private:

	double m_RonDext;
	bool m_angleConstraints;
	bool m_keepDiameter;
	std::vector<double> m_standardRadii;
};

#endif // !CONTEXT_PIPE_POST_CONNEXION_H_
