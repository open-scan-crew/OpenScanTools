#ifndef I_RENDERING_ENGINE_H
#define I_RENDERING_ENGINE_H

class IDataDispatcher;
class VulkanViewport;
class GraphManager;

class IRenderingEngine
{
public:
    IRenderingEngine() {};
    virtual ~IRenderingEngine() = 0 {};

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void registerViewport(VulkanViewport* viewport) = 0;
    virtual void unregisterViewport(VulkanViewport* viewport) = 0;
};

namespace scs
{
    IRenderingEngine* createRenderingEngine(GraphManager& graphManager, IDataDispatcher& dataDispatcher, const float& guiScale);
};

#endif //! RENDERING_ENGINE_H