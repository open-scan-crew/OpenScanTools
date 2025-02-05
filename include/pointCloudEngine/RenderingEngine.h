#ifndef RENDERING_ENGINE_H
#define RENDERING_ENGINE_H

#include "pointCloudEngine/IRenderingEngine.h"
#include "gui/IDataDispatcher.h"
#include "gui/IPanel.h"
#include "gui/UnitUsage.h"

#include "pointCloudEngine/ShowTypes.h"
#include "models/3d/DisplayParameters.h"

#include "vulkan/TlFramebuffer.h"
#include "vulkan/ImageTransferEvent.h"
#include "vulkan/Renderers/Renderer.h"
#include "vulkan/Renderers/PostRenderer.h"
#include "vulkan/Renderers/MeasureRenderer.h"
#include "vulkan/Renderers/MarkerRenderer.h"
#include "vulkan/Renderers/ManipulatorRenderer.h"
#include "vulkan/Renderers/SimpleObjectRenderer.h"
#include "vulkan/Compute/ComputePrograms.h"

#include "models/graph/GraphManager.h"

#include "io/ImageTypes.h"
#include "io/exports/CSVWriter.hxx"

#include <atomic>
#include <thread>
#include <set>

class VulkanViewport;
class ObjectNodeVisitor;
class CameraNode;
class ViewPointNode;


class RenderingEngine : public IRenderingEngine, public IPanel
{
public:
    RenderingEngine(GraphManager& graphManager, IDataDispatcher& dataDispatcher, float guiScale);
    ~RenderingEngine();

    // Use start() and stop() ideally one time at the start and stop of the application.
    void start() override;
    void stop() override;

    // Register a viewport in the rendering engine to allow a clean refresh in the main loop
    void registerViewport(VulkanViewport* viewport) override;
    void unregisterViewport(VulkanViewport* viewport) override;

protected:
    void informData(IGuiData* guiData) override;

private:
    void run();
    void update();
    void updateHD();
    void resetDrawBuffers(TlFramebuffer fb);
    bool updateFramebuffer(VulkanViewport& viewport);
    bool renderVirtualViewport(TlFramebuffer framebuffer, const CameraNode& camera, glm::vec2 screenOffset, double& sleepedTime, ImageTransferEvent& transferEvent);

    typedef void (RenderingEngine::*GuiDataFunction)(IGuiData*);
    inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
    {
        m_dataDispatcher.registerObserverOnKey(this, type);
        m_functions.insert({ type, fct });
    };
    void onScreenshot(IGuiData* data);
    void onScreenshotFormat(IGuiData* data);
    void onManipulatorSize(IGuiData* data);
    void onManipulatorModeLocGlob(IGuiData* data);
    //void initializeManipulators();
    void onStartHDRender(IGuiData* data);
    void onPrepareHDImage(IGuiData* data);
    void onQuitEvent(IGuiData* data);
    void onRecordPerformance(IGuiData* data);
    void onGuizmoParameters(IGuiData* data);
    void onRenderExamineTarget(IGuiData* data);
    void onProcessSignalCancel(IGuiData* data);

    void onActiveCamera(IGuiData* data);

    void drawSelectionRect(VkCommandBuffer, const VulkanViewport& viewport);
    void drawOverlay(VkCommandBuffer, const CameraNode& camera, const VkExtent2D& extend);
    void drawOverlayHD(VkCommandBuffer, const CameraNode& camera, const VkExtent2D& extend, const glm::vec2& screenOffset);
    void timeRendering(bool newFrameSubmited);

    void startImGuiContext();
    void shutdownImGui();
    void initVulkanImGui(const uint8_t& viewportNumber);
    void restartVulkanImGui();
    //void drawImGuiStats();

    void updateCompute();

private:
    IDataDispatcher& m_dataDispatcher;
    std::unordered_map<guiDType, GuiDataFunction> m_functions;

    // Engine state
    std::atomic<bool> m_stopEngine;
    std::atomic<bool> m_engineStarted;
    std::thread m_renderThread;
    uint64_t m_renderingCount = 0;
    // --- IMPORTANT ---
    // * This is the swap index for the rendering. It must not be mistaken with the frame swap index.
    // * For example, we can render simultaneously the same objects and their uniforms in multiple Framebuffers which are desynchronized in their swap index.
    uint32_t m_renderSwapIndex = 0;

    // viewports
    std::mutex m_mutexViewports;
    std::set<VulkanViewport*> m_viewports;

    GraphManager& m_graph;

    // Specific Renderers
    Renderer					m_pcRenderer;
    PostRenderer				m_postRenderer;
    MeasureRenderer				m_measureRenderer;
    MarkerRenderer				m_markerRenderer;
    ManipulatorRenderer			m_manipulatorRenderer;
    SimpleObjectRenderer		m_simpleObjectRenderer;

    // Rendering parameters
    SafePtr<CameraNode> m_activeCamera;
    glm::vec3 m_gizmoParameters = { -0.85, 0.9, 0.25 };
    float m_guiScale = 1.f; 
    bool m_showExamineTarget = true;

    // Manipulator rendering parameters
    double m_manipSize = 1.0;

    struct MouseData
    {
        glm::ivec2 position;
    } m_mouseData;

    // *** Dynamic decimation *** //
    std::chrono::steady_clock::time_point m_startRendering;
    float m_renderTime_ms;
    bool m_previousFrameSkipped;

    // *** HD Render *** //
    std::atomic<bool> m_doHDRender = false;
    std::atomic<bool> m_processSignalCancel = false;
    bool m_notEnoughMemoryForHD = false;
    SafePtr<ViewPointNode> m_hdViewPoint;
    ImageFormat m_hdFormat;
    glm::ivec2 m_hdExtent;
    int m_hdMultisampling;
    std::filesystem::path m_hdImageFilepath;
    bool m_showHDFrame;
    double m_hdFrameRatio;
    ImageHDMetadata m_imageMetadata;
    bool m_showProgressBar;
    uint32_t m_hdtilesize;

    // *** Compute *** //
    std::atomic<bool> m_computeRender = false;
    ComputePrograms m_computePrograms;

    //Screenshot
    std::filesystem::path   m_screenshotFilename;
    ImageFormat             m_screenshotFormat;

    //Pref record
    CSVWriter               m_perfRecord;

};

#endif