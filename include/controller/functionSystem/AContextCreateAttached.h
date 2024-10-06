#ifndef ACONTEXT_CREATE_ATTACHED_H_
#define ACONTEXT_CREATE_ATTACHED_H_

#include "controller/functionSystem/AContextAlignView.h"

#include "models/graph/TransformationModule.h"

class AContextCreateAttached : public AContextAlignView
{
public:
	AContextCreateAttached(const ContextId& id);
	~AContextCreateAttached();
    ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	virtual ContextType getType() const = 0;

protected:
	virtual void createObject(Controller& controller, TransformationModule& transfo) = 0;
	void CalculateCenterAndScale(const glm::dvec3& normal, const glm::dquat& rot, glm::dvec3& center, glm::dvec3& scale);

};

#endif // !ACONTEXT_CREATE_ATTACHED_H_
