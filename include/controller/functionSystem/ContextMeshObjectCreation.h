#ifndef CONTEXT_WAVEFRONT_CREATION_H_
#define CONTEXT_WAVEFRONT_CREATION_H_

#include "controller/functionSystem/ARayTracingContext.h"
#include "controller/messages/IMessage.h"
#include "io/FileInputData.h"

class ClusterNode;

enum class Selection;

class ContextMeshObjectCreation : public ARayTracingContext
{
public:
    ContextMeshObjectCreation(const ContextId& id);
    ~ContextMeshObjectCreation();
    ContextState start(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    ContextState launch(Controller& controller) override;
    ContextState abort(Controller& controller) override;

    bool canAutoRelaunch() const;
    ContextType getType() const override;

private:
    glm::dquat getRotation() const;

private:
    FileInputData input_data_;
    SafePtr<ClusterNode>	m_rootCluster;
    uint32_t				m_count;
};


#endif // !CONTEXT_WAVEFRONT_CREATION_H_
