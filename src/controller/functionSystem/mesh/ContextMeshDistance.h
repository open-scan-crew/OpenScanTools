#ifndef CONTEXT_MESH_DISTANCE_H
#define CONTEXT_MESH_DISTANCE_H

#include "controller/functionSystem/ARayTracingContext.h"
#include "models/OpenScanToolsModelEssentials.h"
#include "models/Types.hpp"
#include "models/3d/MeshBuffer.h"

#include <unordered_set>

class ContextMeshDistance : public ARayTracingContext
{
public:
    ContextMeshDistance(const ContextId& id);
    ~ContextMeshDistance();
    ContextState start(Controller& controller) override;
    virtual ContextState feedMessage(IMessage* message, Controller& controller) override;
    virtual ContextState launch(Controller& controller) override;
    bool canAutoRelaunch() const;

    virtual ContextType getType() const override;

protected:
    xg::Guid m_meshSelected;
    bool m_meshReady;
    //std::mutex m_meshMutex;
    //std::vector<Triangle> m_triangles;
    //glm::dvec3 m_rayOrigin;
    //glm::dvec3 m_rayDir;
};

#endif
