#include "pointCloudEngine/RenderingEngine.h"
#include "vulkan/TlFramebuffer_T.h"
#include "vulkan/VulkanManager.h"

#include "gui/viewport/VulkanViewport.h"

#include "models/graph/ObjectNodeVisitor.h"
#include "models/graph/CameraNode.h"
#include "models/graph/AClippingNode.h"
#include "models/graph/TransformationModule.h"

#include "controller/controls/ControlFunction.h"
#include "controller/controls/ControlSpecial.h"
#include "controller/messages/GeneralMessage.h"

#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "gui/GuiData/GuiDataHD.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ScreenshotTexts.hpp"
#include "gui/UnitConverter.h"
#include "utils/Config.h"

#include "io/ImageWriter.h"
#include "io/exports/CSVWriter.hxx"

#include "utils/Utils.h"
#include "utils/Logger.h"
#include "utils/ImGuiUtils.h"
#include "models/data/Clipping/ClippingGeometry.h"
#include "models/3d/TemperatureScaleData.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"
#include "impl/imgui_impl_qt.h"


#include <cmath>

constexpr double HD_MARGIN = 0.05;

#define TEXT_ORTHO_GRID_SIZE QObject::tr("Grid cell size : %1 %2")

IRenderingEngine* scs::createRenderingEngine(GraphManager& graphManager, IDataDispatcher& dataDispatcher, const float& guiScale)
{
    return new RenderingEngine(graphManager, dataDispatcher, guiScale);
}

RenderingEngine::RenderingEngine(GraphManager& graphManager, IDataDispatcher& dataDispatcher, float guiScale)
    : m_dataDispatcher(dataDispatcher)
    , m_graph(graphManager)
    , m_guiScale(guiScale)
    , m_screenshotFormat(ImageFormat::JPG)
    , m_previousFrameSkipped(false)
    , m_hdMultisampling(1)
    , m_hdtilesize(256)
{
    registerGuiDataFunction(guiDType::screenshot, &RenderingEngine::onScreenshot);
    registerGuiDataFunction(guiDType::renderImagesFormat, &RenderingEngine::onScreenshotFormat);
    registerGuiDataFunction(guiDType::manipulatorLocGlob, &RenderingEngine::onManipulatorModeLocGlob);
    registerGuiDataFunction(guiDType::manipulatorSize, &RenderingEngine::onManipulatorSize);
    registerGuiDataFunction(guiDType::renderRecordPerformance, &RenderingEngine::onRecordPerformance);
    registerGuiDataFunction(guiDType::renderActiveCamera, &RenderingEngine::onActiveCamera);
    registerGuiDataFunction(guiDType::quitEvent, &RenderingEngine::onQuitEvent);
    registerGuiDataFunction(guiDType::hdGenerate, &RenderingEngine::onStartHDRender);
    registerGuiDataFunction(guiDType::hdPrepare, &RenderingEngine::onPrepareHDImage);
    registerGuiDataFunction(guiDType::renderGizmoParameters, &RenderingEngine::onGuizmoParameters);
    registerGuiDataFunction(guiDType::processingSplashScreenEnableCancel, &RenderingEngine::onProcessSignalCancel);

    startImGuiContext();
}

RenderingEngine::~RenderingEngine()
{
    Logger::log(LoggerMode::VKLog) << "Destroying Rendering Engine..." << Logger::endl;
    m_dataDispatcher.unregisterObserver(this);
    stop();

    // - Try to clean the graph in the GraphManager.
    // - Uncomment if any problem during shutdown
    //m_graph.cleanGraph(true);
    shutdownImGui();

    if (m_perfRecord.isOpen())
        m_perfRecord.close();
}

void RenderingEngine::start()
{
    if (m_engineStarted.load() == false)
    {
        m_renderThread = std::thread(&RenderingEngine::run, this);
    }
}

void RenderingEngine::stop()
{
    m_stopEngine.store(true);
    m_renderThread.join();
}

void RenderingEngine::registerViewport(VulkanViewport* viewport)
{
    assert(viewport);
    std::lock_guard<std::mutex> lock(m_mutexViewports);
    if (m_viewports.empty())
        initVulkanImGui();
    if (m_viewports.find(viewport) == m_viewports.end())
    {
        m_viewports.insert(viewport);
        viewport->setCamera(m_graph.createCameraNode(L"Viewport"));
    }
}

void RenderingEngine::unregisterViewport(VulkanViewport* viewport)
{
    assert(viewport);
    std::lock_guard<std::mutex> lock(m_mutexViewports);
    if (m_viewports.find(viewport) != m_viewports.end())
    {
        m_viewports.erase(viewport);
    }
}

void RenderingEngine::informData(IGuiData* data)
{
    if (m_functions.find(data->getType()) != m_functions.end())
    {
        GuiDataFunction fct = m_functions.at(data->getType());
        (this->*fct)(data);
    }
}

void RenderingEngine::onManipulatorModeLocGlob(IGuiData* data)
{
    WritePtr<ManipulatorNode> wManip = m_graph.getManipulatorNode().get();
    if (!wManip)
        return;

    if (static_cast<GuiDataManipulatorLocGlob*>(data)->m_justInvert)
        wManip->setLocalManipulation(!wManip->isLocalManipulation());
    else
        wManip->setLocalManipulation(static_cast<GuiDataManipulatorLocGlob*>(data)->m_isLocal);
}

void RenderingEngine::onManipulatorSize(IGuiData* data)
{
    m_manipSize = ManipulatorNode::getManipSizeFactor(static_cast<GuiDataManipulatorSize*>(data)->m_size);
}

void RenderingEngine::onScreenshotFormat(IGuiData* data)
{
    auto screenshotData = static_cast<GuiDataRenderImagesFormat*>(data);
    m_screenshotFormat = screenshotData->m_format;
    m_screenshotIncludeAlpha = screenshotData->m_includeAlpha;
}

void RenderingEngine::onScreenshot(IGuiData* data)
{
    GuiDataScreenshot* screenshot = static_cast<GuiDataScreenshot*>(data);
    m_screenshotFilename = screenshot->m_filename;
    //ToDo (Aurélien) : add viewport selection, for now select the first one 
    auto viewport((*m_viewports.begin()));
    if (viewport->getNavigationMode() == NavigationMode::Orthographic)
    {
        ReadPtr<CameraNode> rCam = viewport->getCamera().cget();
        if (!rCam)
            return;
        const DisplayParameters& params = rCam->getDisplayParameters();
        auto valueDisplay = [&](double t) { return Utils::roundFloat(UnitConverter::meterToX(t, params.m_unitUsage.distanceUnit)); };
        std::string lengthUnit = UnitConverter::getUnitText(params.m_unitUsage.distanceUnit).toStdString();
        m_screenshotFilename.replace_filename("L" + valueDisplay(rCam->getWidthAt1m()) + lengthUnit + "xH" + valueDisplay(rCam->getHeightAt1m()) + lengthUnit + "_" + m_screenshotFilename.filename().string());
    }
    if (screenshot->m_format != ImageFormat::MAX_ENUM)
    {
        m_screenshotFormat = screenshot->m_format;
        m_screenshotIncludeAlpha = screenshot->m_includeAlpha;
    }
}

void RenderingEngine::onQuitEvent(IGuiData* data)
{
    m_stopEngine.store(true);
}

void RenderingEngine::onRecordPerformance(IGuiData* data)
{
    if (m_perfRecord.isOpen())
        m_perfRecord.close();
    else
    {
        std::filesystem::path guiData(static_cast<GuiDataRenderRecordPerformances*>(data)->m_path);
        m_perfRecord.open(guiData);
        if (m_perfRecord.isOpen())
        {
            m_perfRecord << "cellCount" << "pointCount" << "prepareScansTime" << "prepareObjectsTime" << "renderTime" << "decimation" << CSVWriter::endl;
        }
    }
}

void RenderingEngine::onStartHDRender(IGuiData* data)
{
    // Skip if there is already a HD render
    if (m_doHDRender.load() == true)
        return;

    auto guiData = static_cast<GuiDataGenerateHDImage*>(data);

    m_hdFormat = guiData->m_imageFormat;
    m_hdExtent = guiData->m_imageSize;
    m_hdMultisampling = guiData->m_multisampling;
    m_hdImageFilepath = guiData->m_filepath;
    m_imageMetadata = guiData->m_metadata;
    m_showProgressBar = guiData->m_showProgressBar;
    m_hdtilesize = guiData->m_hdimagetilesize;
    m_hdFullResolutionTraversal = guiData->m_fullResolutionTraversal;

    // FIXME - On peut directement garder la camera sans passer par son viewport
    m_activeCamera = guiData->m_camera;

    // The extent provided by the GuiData must be coherent with the ratio
    assert(abs((double)m_hdExtent.y * m_hdFrameRatio - (double)m_hdExtent.x) <= std::max(m_hdFrameRatio, 1.0));
    m_hdFrameRatio = (double)m_hdExtent.x / (double)m_hdExtent.y;
    m_doHDRender.store(true);
}

void RenderingEngine::onPrepareHDImage(IGuiData* data)
{
    GuiDataPrepareHDImage* guiData = static_cast<GuiDataPrepareHDImage*>(data);
    m_showHDFrame = guiData->m_showFrame;
    m_showHDFrameGrid = guiData->m_showGrid;
    m_hdFrameRatio = guiData->m_frameRatio;
}

void RenderingEngine::onActiveCamera(IGuiData* data)
{
    auto gui = static_cast<GuiDataActiveCamera*>(data);
    if(gui->m_camera)
        m_activeCamera = gui->m_camera;
}

void RenderingEngine::onGuizmoParameters(IGuiData* data)
{
    m_gizmoParameters = static_cast<GuiDataGizmoParameters*>(data)->m_paramters;
}

void RenderingEngine::onProcessSignalCancel(IGuiData* data)
{
    m_processSignalCancel.store(true);
}

void RenderingEngine::run()
{
    // Allow only one update() running 
    bool vFalse = false;
    if (m_engineStarted.compare_exchange_strong(vFalse, true) == false)
        return;

    m_stopEngine.store(false);

    while (m_stopEngine.load() == false)
    {
        // Introduit une pause possible dans le rendu pour faire autre chose (image HD)
        if (m_doHDRender.load() == true)
            updateHD();
        else if (m_computeRender.load() == true)
            updateCompute();
        else
            update();
    }

    m_engineStarted.store(false);
}

VkExtent2D getViewportExtent(VulkanViewport& viewport)
{
    if (viewport.getFramebuffer() != TL_NULL_HANDLE)
        return viewport.getFramebuffer()->extent;
    else
        return { 0, 0 };
}

void RenderingEngine::update()
{
    //initializeManipulators();
    // Refresh the Scene
    m_renderSwapIndex = ++m_renderSwapIndex % 2;
    m_graph.refreshScene();

    m_graph.resetObjectsHovered();
    m_mutexViewports.lock();

    std::vector<TlFramebuffer> m_framebuffersToRender;
    // Instruct the drawing for each viewport (if necessary)
    VulkanManager& vkm = VulkanManager::getInstance();
    vkm.startNextFrame();
    vkm.waitForRenderFence();
    // Refresh the Inputs for each viewport.
    // Also apply the viewport inputs on the graph before updating the scene.

    // ******* Inputs *******
    // On gère les inputs survenus dans tout les viewports.
    // Les inputs peuvent entrainer une modification du modèle et donc de l'affichage.
    // Un input dans un viewport peut notament avoir des effets visible dans un autre viewport.
    // Les input qui modifie le modèle sont :
    //  * Sélection d'un objet (Click)
    //  * Hover d'un objet
    //  * Manipulation d'un ou plusieurs objets
    for (VulkanViewport* viewport : m_viewports)
    {
        WritePtr<CameraNode> wCamera = viewport->getCamera().get();
        if (!wCamera)
            continue;

        // Single & Multi hover
        m_graph.setObjectsHovered(viewport->getHoveredIds());
        SafePtr<AGraphNode> hoverNode = m_graph.getSingleHoverObject();

        // Single & Multi selection
        // NOTE(robin) - the control will not reach in time the graph for this frame. So the items will be selected for the next frame.
        std::unordered_set<uint32_t> objects_selected = viewport->getSelectedIds();
        if (!objects_selected.empty())
            m_dataDispatcher.sendControl(new control::special::MultiSelect(m_graph.getNodesFromGraphicIds(objects_selected), true));

        viewport->updateInputs(wCamera, m_graph.getManipulatorNode(), hoverNode);
    }

    // ******* Affichage *******
    for (VulkanViewport* viewport : m_viewports)
    {
        if (!viewport->isInitialized())
            continue;

        m_mouseData.position = viewport->getMousePos(); // ?? why not in the updateFramebuffer ??

        if (updateFramebuffer(*viewport))
            m_framebuffersToRender.push_back(viewport->getFramebuffer());
    }
    m_mutexViewports.unlock();
    VulkanManager::getInstance().submitMultipleFramebuffer(m_framebuffersToRender);
    processPendingScreenshots();
    timeRendering(m_framebuffersToRender.size() > 0);

    // NOTE(robin) - Un sleep de 16ms dure en pratique entre 16 et 35ms
    if (m_framebuffersToRender.size() == 0)
        std::this_thread::sleep_for(std::chrono::duration<double, std::ratio<1, 1000>>(16));
}

void RenderingEngine::updateHD()
{
    m_processSignalCancel.store(false);
    m_notEnoughMemoryForHD = false;

    // Refresh the Scene
    m_renderSwapIndex = 0;
    m_graph.refreshScene();

    // Should be input parameters
    // NOTE: Gap filling relies on a 3x3 kernel. A base 2px border keeps a one-pixel safety zone
    // after the image transfer cropping and avoids visible seams between tiles when the texel
    // threshold is low (e.g., 2). Post-processing passes like edge-aware blur and depth lining can
    // require a larger footprint, so the border is expanded accordingly.
    auto computeTileBorder = [](const DisplayParameters& display) {
        constexpr uint32_t baseBorder = 2;
        uint32_t border = baseBorder;

        if (display.m_edgeAwareBlur.enabled)
        {
            const float resolutionScale = std::max(display.m_edgeAwareBlur.resolutionScale, 0.1f);
            const uint32_t blurFootprint = static_cast<uint32_t>(std::ceil(display.m_edgeAwareBlur.radius / resolutionScale));
            border = std::max(border, blurFootprint + 1u); // +1 to keep a safety band after cropping
        }

        if (display.m_depthLining.enabled)
            border = std::max<uint32_t>(border, 3u);

        if (display.m_postRenderingAmbientOcclusion.enabled)
        {
            const uint32_t aoFootprint = static_cast<uint32_t>(std::ceil(std::max(0.0f, display.m_postRenderingAmbientOcclusion.radius)));
            border = std::max(border, aoFootprint + 1u);
        }

        return border;
    };

    DisplayParameters hdDisplayParams = {};
    if (ReadPtr<CameraNode> rCam = m_activeCamera.cget())
        hdDisplayParams = rCam->getDisplayParameters();

    const uint32_t border = computeTileBorder(hdDisplayParams);
    constexpr uint32_t minTileSize = 64;
    const int sampleCount = std::max(1, m_hdMultisampling);
    VulkanManager& vkm = VulkanManager::getInstance();
    const bool useAccumulation = sampleCount > 1;
    const size_t bytesPerPixel = m_hdFormat == ImageFormat::PNG16 ? 8ull : 4ull;

    auto buildSampleOffsets = [](int count) {
        switch (count)
        {
        case 2:
            return std::vector<glm::vec2>{ {-0.25f, -0.25f}, {0.25f, 0.25f} };
        case 4:
            return std::vector<glm::vec2>{ {-0.25f, -0.25f}, {0.25f, -0.25f}, {-0.25f, 0.25f}, {0.25f, 0.25f} };
        case 8:
            return std::vector<glm::vec2>{
                {-0.25f, -0.125f}, {0.25f, -0.125f}, {-0.25f, 0.125f}, {0.25f, 0.125f},
                {-0.125f, -0.25f}, {0.125f, -0.25f}, {-0.125f, 0.25f}, {0.125f, 0.25f}
            };
        default:
            return std::vector<glm::vec2>(std::max(count, 1), glm::vec2(0.f));
        }
    };

    const std::vector<glm::vec2> sampleOffsets = buildSampleOffsets(sampleCount);

    auto clampTileSize = [&](uint32_t size) {
        const uint32_t maxImageSize = std::max(2u, vkm.getPhysicalDeviceLimits().maxImageDimension2D);
        const uint32_t maxAllowed = maxImageSize > border * 2 ? maxImageSize - border * 2 : minTileSize;
        return std::max(minTileSize, std::min(size, maxAllowed));
        };

    uint32_t frameW = clampTileSize(m_hdtilesize);
    uint32_t frameH = frameW;

    if (m_hdExtent.x == 0 || m_hdExtent.y == 0)
        return;

    auto computeTileCount = [&](uint32_t tileW, uint32_t tileH) {
        const uint32_t tileCountX = (m_hdExtent.x - 1) / tileW + 1;
        const uint32_t tileCountY = (m_hdExtent.y - 1) / tileH + 1;
        return std::pair<uint32_t, uint32_t>(tileCountX, tileCountY);
        };

    // If the requested tile size leads to too many tiles, scale it up to reduce per-tile overhead.
    constexpr uint32_t targetTileBudget = 64;
    auto [tileCountX, tileCountY] = computeTileCount(frameW, frameH);
    uint32_t tileTotal = tileCountX * tileCountY;
    if (tileTotal > targetTileBudget)
    {
        const double scale = std::sqrt(static_cast<double>(tileTotal) / static_cast<double>(targetTileBudget));
        frameW = clampTileSize(static_cast<uint32_t>(std::ceil(frameW * scale)));
        frameH = frameW;
    }

    // Create a virtual viewport, falling back to smaller tiles if necessary (low VRAM cases)
    TlFramebuffer virtualViewport = TL_NULL_HANDLE;
    bool viewportReady = false;
    while (frameW >= minTileSize && viewportReady == false)
    {        
        const uint32_t viewportW = frameW + border * 2;
        const uint32_t viewportH = frameH + border * 2;
        if (vkm.createVirtualViewport(viewportW, viewportH, sampleCount, virtualViewport))
        {
            viewportReady = true;
            break;
        }
        frameW = clampTileSize(frameW / 2);
        frameH = frameW;
    }

    if (!viewportReady)
        return;

    // Clear the 2-frame safety buffer once before tiling instead of per tile to reduce overhead.
    vkm.startNextFrame();
    vkm.startNextFrame();
    vkm.startNextFrame();

    ImageWriter imgWriter;
    OrthoGridOverlay orthoGridOverlay;
    RampScaleOverlay rampScaleOverlay;
    if (imgWriter.startCapture(m_hdFormat, m_hdExtent.x, m_hdExtent.y, m_imageMetadata.includeAlpha) == false)
        return;

    // Projection parameters
    std::tie(tileCountX, tileCountY) = computeTileCount(frameW, frameH);
    uint32_t count = 0;
    double sleepedTime = 0.0;
    SafePtr<CameraNode> cameraHD = m_graph.duplicateCamera(m_activeCamera);
    {
        WritePtr<CameraNode> wCameraHD = cameraHD.get();
        if (!wCameraHD)
        {
            assert(false);
            return;
        }

        // Update internal matrixes
        wCameraHD->refresh();
        orthoGridOverlay.active = wCameraHD->getProjectionMode() == ProjectionMode::Orthographic && wCameraHD->m_orthoGridActive;
        if (orthoGridOverlay.active)
        {
            orthoGridOverlay.step = wCameraHD->m_orthoGridStep;
            orthoGridOverlay.lineWidth = wCameraHD->m_orthoGridLineWidth;
            orthoGridOverlay.color = wCameraHD->m_orthoGridColor;
            orthoGridOverlay.distanceUnit = wCameraHD->m_unitUsage.distanceUnit;
        }

        if(m_showProgressBar)
            m_dataDispatcher.updateInformation(new GuiDataProcessingSplashScreenStart((uint64_t)tileCountX * tileCountY, TEXT_SCREENSHOT_START, TEXT_SCREENSHOT_PROCESSING.arg(count).arg(tileCountX * tileCountY)));

        ProjectionData proj = wCameraHD->getTruncatedProjection(m_hdFrameRatio, m_showHDFrame ? HD_MARGIN : 0.0);
        ProjectionFrustum frustum = proj.getProjectionFrustum();
        m_imageMetadata.hasBottomZ = false;
        m_imageMetadata.hasVerticalCorners = false;
        if (m_imageMetadata.ortho)
        {
            const glm::dvec3 viewDir = glm::normalize(wCameraHD->getViewAxis());
            const double verticalComponent = std::abs(viewDir.z);
            constexpr double alignmentThreshold = 1e-3;
            const glm::dmat4 modelMatrix = wCameraHD->getModelMatrix();
            auto toWorld = [&](double x, double y) {
                glm::dvec4 pos = modelMatrix * glm::dvec4(x, y, 0.0, 1.0);
                return glm::dvec3(pos);
            };

            if (verticalComponent >= (1.0 - alignmentThreshold))
            {
                const glm::dvec3 bottomLeftWorld = toWorld(frustum.l, frustum.t);
                const glm::dvec3 topRightWorld = toWorld(frustum.r, frustum.b);
                m_imageMetadata.hasVerticalCorners = true;
                m_imageMetadata.imageBottomLeft = glm::dvec2(bottomLeftWorld.x, bottomLeftWorld.y);
                m_imageMetadata.imageTopRight = glm::dvec2(topRightWorld.x, topRightWorld.y);
            }
            else if (verticalComponent <= alignmentThreshold)
            {
                const glm::dvec3 bottomLeftWorld = toWorld(frustum.l, frustum.t);
                const glm::dvec3 bottomRightWorld = toWorld(frustum.r, frustum.t);
                m_imageMetadata.hasBottomZ = true;
                m_imageMetadata.imageBottomZ = (bottomLeftWorld.z + bottomRightWorld.z) / 2.0;
            }
        }
        std::vector<uint64_t> tileAccumulation;
        std::vector<char> tileBuffer;
        const bool preciseColor = m_hdFormat == ImageFormat::PNG16;
        for (uint32_t tileX = 0; tileX < tileCountX; tileX++)
        {
            for (uint32_t tileY = 0; tileY < tileCountY; tileY++)
            {
                // Test si on veut interrompre le rendu
                if (m_processSignalCancel.exchange(false))
                {
                    if (m_notEnoughMemoryForHD)
                        m_dataDispatcher.updateInformation(new GuiDataProcessingSplashScreenLogUpdate(QString(TEXT_HD_FAILED_NOT_ENOUGH_MEMORY)));
                    tileX = tileCountX;
                    break;
                }

                count++;
                const uint32_t tileOffsetW = tileX * frameW;
                const uint32_t tileOffsetH = tileY * frameH;
                const uint32_t tileWidth = std::min<uint32_t>(frameW, m_hdExtent.x - tileOffsetW);
                const uint32_t tileHeight = std::min<uint32_t>(frameH, m_hdExtent.y - tileOffsetH);
                if (tileWidth == 0 || tileHeight == 0)
                    continue;

                if (useAccumulation)
                {
                    tileAccumulation.assign(static_cast<size_t>(tileWidth) * tileHeight * 4ull, 0ull);
                    tileBuffer.resize(static_cast<size_t>(tileWidth) * tileHeight * bytesPerPixel);
                }
                int accumulatedSamples = 0;

                for (int sampleIdx = 0; sampleIdx < sampleCount; ++sampleIdx)
                {
                    const glm::vec2 sampleJitter = sampleOffsets[static_cast<size_t>(sampleIdx % static_cast<int>(sampleOffsets.size()))];

                    // Set (x, y, z, left, right, top, bottom, near, far);
                    double minW = double(tileX) * frameW - border + sampleJitter.x;
                    double maxW = (double(tileX) + 1) * frameW + border + sampleJitter.x;
                    double dl = frustum.l * (1.0 - minW / m_hdExtent.x) + frustum.r * minW / m_hdExtent.x;
                    double dr = frustum.l * (1.0 - maxW / m_hdExtent.x) + frustum.r * maxW / m_hdExtent.x;
                    double minH = double(tileY) * frameH - border + sampleJitter.y;
                    double maxH = (double(tileY) + 1) * frameH + border + sampleJitter.y;
                    double db = frustum.b * (1.0 - minH / m_hdExtent.y) + frustum.t * minH / m_hdExtent.y;
                    double dt = frustum.b * (1.0 - maxH / m_hdExtent.y) + frustum.t * maxH / m_hdExtent.y;
                    wCameraHD->setProjectionFrustum(dl, dr, db, dt, frustum.n, frustum.f);
                    wCameraHD->refresh();
                    wCameraHD->prepareUniforms(m_renderSwapIndex);

                    glm::vec2 screenOffset((2 * tileX * (float)frameW - m_hdExtent.x) / 2.f + sampleJitter.x,
                        (2 * tileY * (float)frameH - m_hdExtent.y) / 2.f + sampleJitter.y);

                    ImageTransferEvent ite;
                    renderVirtualViewport(virtualViewport, *&wCameraHD, screenOffset, sleepedTime, ite, m_hdFullResolutionTraversal);

                    if (useAccumulation)
                    {
                        if (vkm.doImageTransfer(ite, tileWidth, tileHeight, tileBuffer.data(), tileBuffer.size(), 0u, 0u, border, preciseColor))
                        {
                            if (preciseColor)
                            {
                                const uint16_t* srcRow = reinterpret_cast<const uint16_t*>(tileBuffer.data());
                                for (size_t idx = 0; idx < static_cast<size_t>(tileWidth) * tileHeight * 4ull; ++idx)
                                    tileAccumulation[idx] += srcRow[idx];
                            }
                            else
                            {
                                const uint8_t* srcRow = reinterpret_cast<const uint8_t*>(tileBuffer.data());
                                for (size_t idx = 0; idx < static_cast<size_t>(tileWidth) * tileHeight * 4ull; ++idx)
                                    tileAccumulation[idx] += srcRow[idx];
                            }
                            ++accumulatedSamples;
                        }
                    }
                    else
                    {
                        imgWriter.transferImageTile(ite, tileOffsetW, tileOffsetH, border);
                    }
                }

                if (useAccumulation)
                {
                    const double divisor = static_cast<double>(std::max(1, accumulatedSamples));
                    if (preciseColor)
                    {
                        uint16_t* dst = reinterpret_cast<uint16_t*>(tileBuffer.data());
                        for (size_t idx = 0; idx < static_cast<size_t>(tileWidth) * tileHeight * 4ull; ++idx)
                        {
                            dst[idx] = static_cast<uint16_t>(std::round(static_cast<double>(tileAccumulation[idx]) / divisor));
                        }
                    }
                    else
                    {
                        uint8_t* dst = reinterpret_cast<uint8_t*>(tileBuffer.data());
                        for (size_t idx = 0; idx < static_cast<size_t>(tileWidth) * tileHeight * 4ull; ++idx)
                        {
                            dst[idx] = static_cast<uint8_t>(std::round(static_cast<double>(tileAccumulation[idx]) / divisor));
                        }
                    }
                    imgWriter.writeTile(tileBuffer.data(), tileWidth, tileHeight, tileOffsetW, tileOffsetH);
                }

                // Copy du rendu dans l'image de destination
                if (m_showProgressBar)
                    m_dataDispatcher.updateInformation(new GuiDataProcessingSplashScreenProgressBarUpdate(TEXT_SCREENSHOT_PROCESSING.arg(count).arg(tileCountX * tileCountY), count));
            }
        }
    }

    if (m_showProgressBar)
        m_dataDispatcher.updateInformation(new GuiDataProcessingSplashScreenProgressBarUpdate(TEXT_SCREENSHOT_PROCESSING.arg(count).arg(tileCountX * tileCountY), count));
    Logger::log(LoggerMode::VKLog) << "Sleeped time during rendering " << sleepedTime << " seconds !" << Logger::endl;

    cameraHD.destroy();
    vkm.destroyFramebuffer(virtualViewport);

    if (orthoGridOverlay.active)
        imgWriter.applyOrthoGridOverlay(m_imageMetadata, orthoGridOverlay);

    {
        rampScaleOverlay.unitUsage = hdDisplayParams.m_unitUsage;
        rampScaleOverlay.graduation = hdDisplayParams.m_rampScale.graduationCount;

        if (hdDisplayParams.m_rampScale.showTemperatureScale)
        {
            TemperatureScaleData temperatureScale = m_graph.getTemperatureScaleData();
            if (temperatureScale.isValid && !temperatureScale.entries.empty())
            {
                const double entryFirst = temperatureScale.entries.front().temperature;
                const double entryLast = temperatureScale.entries.back().temperature;
                rampScaleOverlay.active = true;
                rampScaleOverlay.isTemperature = true;
                rampScaleOverlay.vmin = std::min(entryFirst, entryLast);
                rampScaleOverlay.vmax = std::max(entryFirst, entryLast);
                rampScaleOverlay.temperatureAscending = entryFirst <= entryLast;
                rampScaleOverlay.temperatureEntries = temperatureScale.entries;
                rampScaleOverlay.steps = static_cast<int>(temperatureScale.entries.size());
            }
        }
        else if (hdDisplayParams.m_rampScale.showScale)
        {
            ClippingAssembly rampAssembly;
            std::unordered_set<SafePtr<AClippingNode>> ramps = m_graph.getRampObjects(true, false);
            for (const SafePtr<AClippingNode>& ramp : ramps)
            {
                ReadPtr<AClippingNode> rRamp = ramp.cget();
                if (!rRamp)
                    continue;
                rRamp->pushRampGeometries(rampAssembly.rampActives, TransformationModule(*&rRamp));
            }

            bool rampSelected = false;
            std::shared_ptr<IClippingGeometry> rampToOverlay;
            for (const std::shared_ptr<IClippingGeometry>& ramp : rampAssembly.rampActives)
            {
                if (!ramp->isSelected)
                    continue;

                if (!rampSelected)
                {
                    rampSelected = true;
                    rampToOverlay = ramp;
                }
                else
                {
                    rampToOverlay = nullptr;
                    break;
                }
            }

            if (rampToOverlay && rampToOverlay->rampSteps > 0)
            {
                float vmin = 0.f;
                float vmax = 0.f;
                const glm::vec4 params = rampToOverlay->params;
                switch (rampToOverlay->getShape())
                {
                case ClippingShape::box:
                    if (hdDisplayParams.m_rampScale.centerBoxScale)
                    {
                        vmin = -params[2];
                        vmax = params[2];
                    }
                    else
                    {
                        vmin = 0.f;
                        vmax = params[2] * 2.f;
                    }
                    break;
                case ClippingShape::cylinder:
                case ClippingShape::sphere:
                    vmin = params[0] - params[3];
                    vmax = params[1] - params[3];
                    break;
                case ClippingShape::torus:
                    vmin = params[2];
                    vmax = params[3];
                    break;
                }

                rampScaleOverlay.active = true;
                rampScaleOverlay.isTemperature = false;
                rampScaleOverlay.vmin = vmin;
                rampScaleOverlay.vmax = vmax;
                rampScaleOverlay.steps = rampToOverlay->rampSteps;
            }
        }
    }

    if (rampScaleOverlay.active)
        imgWriter.applyRampScaleOverlay(rampScaleOverlay);

    if (imgWriter.save(m_hdImageFilepath, m_imageMetadata))
    {
        if (m_showProgressBar)
        {
            m_dataDispatcher.updateInformation(new GuiDataProcessingSplashScreenLogUpdate(TEXT_SCREENSHOT_DONE.arg(m_hdImageFilepath.generic_wstring())));
            m_dataDispatcher.updateInformation(new GuiDataProcessingSplashScreenEnd(TEXT_SCREENSHOT_PROCESSING_DONE));
            m_dataDispatcher.updateInformation(new GuiDataTmpMessage(TEXT_SCREENSHOT_DONE.arg(m_hdImageFilepath.generic_wstring())));
        }
    }
    else
    {
        if (m_showProgressBar)
            m_dataDispatcher.updateInformation(new GuiDataProcessingSplashScreenEnd(TEXT_SCREENSHOT_FAILED));
        IOLOG << "Failed to write " << m_hdImageFilepath << LOGENDL;
    }

    m_dataDispatcher.sendControl(new control::function::ForwardMessage(new GeneralMessage(GeneralInfo::IMAGEEND)));

    m_hdImageFilepath.clear();
    m_doHDRender.store(false);
}

void RenderingEngine::resetDrawBuffers(TlFramebuffer _fb)
{
    SimpleBuffer& markerBuffer = _fb->drawMarkerBuffers[_fb->currentImage];
    VulkanManager::getInstance().freeAllocation(markerBuffer);
    SimpleBuffer& measureBuffer = _fb->drawMeasureBuffers[_fb->currentImage];
    VulkanManager::getInstance().freeAllocation(measureBuffer);
}

VkClearColorValue getVkClearColor(BlendMode mode, Color32 color32)
{
    const VkClearColorValue background{ color32.Red_f(), color32.Green_f(), color32.Blue_f(), 0.f };
    return (mode == BlendMode::Opaque) ? background : VkClearColorValue{ 0.f, 0.f, 0.f, 0.f };
};

bool RenderingEngine::updateFramebuffer(VulkanViewport& viewport)
{
    VulkanManager& vkm = VulkanManager::getInstance();
    // We must have a valid framebuffer
    TlFramebuffer framebuffer = viewport.getFramebuffer();
    if (framebuffer == TL_NULL_HANDLE)
        return false;

    // The frame rendering can begin
    if (vkm.acquireNextImage(framebuffer) == false)
        return false;

    WritePtr<CameraNode> wCamera = viewport.getCamera().get();
    if (!wCamera)
    {
        Logger::log(LoggerMode::VKLog) << "Failed to get camera on write mode." << Logger::endl;
        // We must recreate because we already have acquire the next image.
        // Another solution would be to NOT acquire the image on the next update. But this solution seems simpler.
        framebuffer->mustRecreateSwapchain.store(true);
        return false;
    }

    if (!viewport.isInitialized())
    {
        Logger::log(LoggerMode::VKLog) << "Viewport not initialized." << Logger::endl;
        return false;
    }
    FrameStats stats = { 0, 0, 0.f, 0.f, 0.f, 0.f, 1.f };

    // Compute the clippings active and their matrices
    std::chrono::steady_clock::time_point tp_graph = std::chrono::steady_clock::now();

    //prepareGeneralUniforms(wCamera);
    wCamera->prepareUniforms(m_renderSwapIndex);
    wCamera->setScreenRatio(framebuffer->extent.width, framebuffer->extent.height);
    resetDrawBuffers(framebuffer);

    // Visitor
    ObjectNodeVisitor visitor(m_graph, framebuffer->extent, m_guiScale, *&wCamera);
    visitor.setComplementaryRenderParameters(m_renderSwapIndex, framebuffer->pcFormat);

    // Visit all the graph and store graphic resources for the final draw
    // TODO - Set the panoramic scan
    //        SafePtr<PointCloudNode> panoScan = wCamera->getPanoramicScan();
    visitor.bakeGraph(m_graph.getRoot());
    bool refreshPCRender = viewport.compareFrameHash(visitor.getFrameHash());

    // The rendering necissities must be validated
    viewport.updateRenderingNecessityState(wCamera, refreshPCRender);
    float decimationRatio = viewport.decimatePointSize(refreshPCRender);
    if (decimationRatio > 0.f)
        decimationRatio *= viewport.getOctreePrecisionMultiplier();
    visitor.setDecimationRatio(decimationRatio);

    std::chrono::steady_clock::time_point tp_scans = std::chrono::steady_clock::now();

    VkCommandBuffer cmdBuffer = vkm.getGraphicsCmdBuffer(framebuffer);
    const DisplayParameters& display = wCamera->getDisplayParameters();
    vkm.resetCommandBuffer(cmdBuffer);
    vkm.beginCommandBuffer(cmdBuffer);
    if (refreshPCRender)
    {
        vkm.beginScanRenderPass(framebuffer, getVkClearColor(display.m_blendMode, display.m_backgroundColor));

        visitor.draw_baked_pointClouds(cmdBuffer, m_pcRenderer);

        viewport.setMissingScanPart(visitor.isPointCloudMissingPart());
        vkm.endScanRenderPass(framebuffer);

        // First compute shader (Gap Filling / Depth correction)
        vkm.beginPostTreatmentFilling(framebuffer);
        m_postRenderer.setConstantZRange(wCamera->getNear(), wCamera->getFar(), cmdBuffer);
        m_postRenderer.setConstantScreenSize(framebuffer->extent, wCamera->getPixelSize1m(framebuffer->extent.width, framebuffer->extent.height), cmdBuffer);
        m_postRenderer.setConstantProjMode(wCamera->getProjectionMode() == ProjectionMode::Perspective, cmdBuffer);
        m_postRenderer.setConstantTexelThreshold(display.m_texelThreshold, cmdBuffer);
        m_postRenderer.processPointFilling(cmdBuffer, framebuffer->descSetSamplers, framebuffer->descSetCorrectedDepth, framebuffer->extent);

        // Second compute shader (Normals)
        if (display.m_postRenderingNormals.show && display.m_blendMode == BlendMode::Opaque)
        {
            vkm.beginPostTreatmentNormal(framebuffer);
            m_postRenderer.setConstantScreenOffset(-glm::vec2((float)framebuffer->extent.width / 2.f, (float)framebuffer->extent.height / 2.f), cmdBuffer);
            m_postRenderer.setConstantLighting(display.m_postRenderingNormals, cmdBuffer);
            if (display.m_mode == UiRenderMode::Normals_Colored)
                m_postRenderer.processNormalColored(cmdBuffer, wCamera->getInversedViewUniform(m_renderSwapIndex), framebuffer->descSetSamplers, framebuffer->descSetCorrectedDepth, framebuffer->extent);
            else
                m_postRenderer.processNormalShading(cmdBuffer, wCamera->getInversedViewUniform(m_renderSwapIndex), framebuffer->descSetSamplers, framebuffer->descSetCorrectedDepth, framebuffer->extent);
        }

        if (display.m_postRenderingAmbientOcclusion.enabled && display.m_blendMode == BlendMode::Opaque)
        {
            vkm.beginPostTreatmentAmbientOcclusion(framebuffer);
            m_postRenderer.processAmbientOcclusion(cmdBuffer, display.m_postRenderingAmbientOcclusion, wCamera->getNear(), wCamera->getFar(), wCamera->getProjectionMode() == ProjectionMode::Perspective, framebuffer->descSetSamplers, framebuffer->descSetCorrectedDepth, framebuffer->extent);
        }

        if (display.m_depthLining.enabled)
        {
            vkm.beginPostTreatmentDepthLining(framebuffer);
            m_postRenderer.processDepthLining(cmdBuffer, display.m_depthLining, framebuffer->descSetSamplers, framebuffer->descSetCorrectedDepth, framebuffer->extent);
        }

        if (display.m_edgeAwareBlur.enabled)
        {
            vkm.beginPostTreatmentEdgeAwareBlur(framebuffer);
            m_postRenderer.processEdgeAwareBlur(cmdBuffer, display.m_edgeAwareBlur, framebuffer->descSetSamplers, framebuffer->descSetCorrectedDepth, framebuffer->extent);
        }

        if (display.m_blendMode != BlendMode::Opaque && display.m_transparency > 0.f)
        {
            vkm.beginPostTreatmentTransparency(framebuffer);
            m_postRenderer.setConstantHDR(display.m_transparency, display.m_negativeEffect, display.m_reduceFlash, display.m_flashAdvanced, display.m_flashControl, framebuffer->extent, display.m_backgroundColor, cmdBuffer);
            m_postRenderer.processTransparencyHDR(cmdBuffer, framebuffer->descSetSamplers, framebuffer->extent);
        }
    }

    std::chrono::steady_clock::time_point tp_objects = std::chrono::steady_clock::now();

    // ----- Objects Render Pass -----
    // (subpass 0) - draw opaque objects
    vkm.beginObjectRenderPass(framebuffer);
    visitor.draw_baked_measures(cmdBuffer, m_measureRenderer, framebuffer->drawMeasureBuffers[framebuffer->currentImage]);
    visitor.draw_baked_meshes(cmdBuffer, m_simpleObjectRenderer, false);

    // (subpass 1) - draw transparent objects
    vkm.getDeviceFunctions()->vkCmdNextSubpass(cmdBuffer, VK_SUBPASS_CONTENTS_INLINE);
    visitor.draw_baked_meshes(cmdBuffer, m_simpleObjectRenderer, true);

    // (subpass 2) - blend
    vkm.getDeviceFunctions()->vkCmdNextSubpass(cmdBuffer, VK_SUBPASS_CONTENTS_INLINE);
    visitor.blendTransparentPass(cmdBuffer, m_simpleObjectRenderer, framebuffer->descSetInputTransparentLayer);

    // (subpass 3) - draw markers
    vkm.getDeviceFunctions()->vkCmdNextSubpass(cmdBuffer, VK_SUBPASS_CONTENTS_INLINE);
    SimpleBuffer& markerBuffer = framebuffer->drawMarkerBuffers[framebuffer->currentImage];
    visitor.draw_baked_markers(cmdBuffer, m_markerRenderer, framebuffer->descSetInputDepth, markerBuffer);

    // (subpass 4) - ImGui
    vkm.getDeviceFunctions()->vkCmdNextSubpass(cmdBuffer, VK_SUBPASS_CONTENTS_INLINE);
    visitor.setLastMousePosition(m_mouseData.position);
    visitor.drawImGuiBegin(m_graph.getRoot(), cmdBuffer);
    // Draw 2D Overlay (selection rectangle, ui, etc)
    drawSelectionRect(cmdBuffer, viewport);
    drawOverlay(cmdBuffer, *&wCamera, framebuffer->extent);

    visitor.drawRampOverlay();
    visitor.drawImGuiStats(viewport);
    visitor.drawImGuiEnd(cmdBuffer);

    std::shared_ptr<MeshBuffer> gizmo = MeshManager::getInstance().getManipMesh(ManipulationMode::Translation);
    // (subpass 5) - draw gizmos
    vkm.getDeviceFunctions()->vkCmdNextSubpass(cmdBuffer, VK_SUBPASS_CONTENTS_INLINE);
    visitor.draw_baked_manipulators(cmdBuffer, m_manipulatorRenderer, m_manipSize);
    visitor.drawGizmo(cmdBuffer, m_gizmoParameters, gizmo, m_manipulatorRenderer);

    vkm.endObjectRenderPass(framebuffer);
    // -----------------------------

    if (!m_screenshotFilename.empty())
    {
        // We use an VkEvent for recording the VkImage transfer and get it on the host later
        const bool preciseScreenshot = m_screenshotFormat == ImageFormat::PNG16;
        ImageTransferEvent ite = vkm.transferFramebufferImage(framebuffer, cmdBuffer, preciseScreenshot);
        m_pendingScreenshots.push_back({ m_screenshotFilename, m_screenshotFormat, m_screenshotIncludeAlpha, ite, framebuffer->extent.width, framebuffer->extent.height });
        m_screenshotFilename.clear();
    }

    // -----------------------------
    vkm.applyPicking(framebuffer, viewport.getMousePos(), nullptr);
    vkm.endCommandBuffer(cmdBuffer);

    viewport.getPickingPos(*&wCamera);
    viewport.refreshHoveredId(visitor.getTextHoveredId());

    // All stats
    visitor.getDrawCount(stats.pointCount, stats.cellCount);
    stats.bakeGraph = std::chrono::duration<float, std::milli>(tp_scans - tp_graph).count();
    stats.prepareScansTime = std::chrono::duration<float, std::milli>(tp_objects - tp_scans).count();
    stats.prepareObjectsTime = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - tp_objects).count();
    if (m_perfRecord.isOpen())
    {
        FrameStats& _stats(viewport.m_frameStack[(viewport.m_frameStackIndex - 2) % viewport.m_frameStack.size()]);
        if(_stats.renderTime)
            m_perfRecord << _stats.cellCount << _stats.pointCount << _stats.prepareScansTime << _stats.prepareObjectsTime << _stats.renderTime << _stats.decimation << CSVWriter::endl;
    }
    viewport.recordFrameStats(stats);

    return true;
}

bool RenderingEngine::renderVirtualViewport(TlFramebuffer framebuffer, const CameraNode& camera, glm::vec2 screenOffset, double& _sleepedTime, ImageTransferEvent& transferEvent, bool fullResolutionTraversal)
{
    VulkanManager& vkm = VulkanManager::getInstance();
    const DisplayParameters& displayParam = camera.getDisplayParameters();

    
    // We must have a valid framebuffer for virtual rendering
    if (framebuffer->pImages[framebuffer->currentImage] == VK_NULL_HANDLE)
        return false;

    // Dynamic rendering variables
    const bool transparencyForcesFullTraversal = (displayParam.m_transparency > 0) && (displayParam.m_blendMode == BlendMode::Transparent);
    float decimationRatio = (transparencyForcesFullTraversal || fullResolutionTraversal) ? 0.f : 1.f;  // no decimation (0.0) with transparency or explicit full traversal
	if (decimationRatio > 0.f)
		decimationRatio *= getOctreePrecisionMultiplier(Config::getOctreePrecision());

    resetDrawBuffers(framebuffer);
    // Visitors
    ObjectNodeVisitor visitor(m_graph, framebuffer->extent, m_guiScale, camera);
    visitor.setDecimationRatio(decimationRatio);
    visitor.setComplementaryRenderParameters(m_renderSwapIndex, framebuffer->pcFormat);
    visitor.activateLogPerWorkload(true, false);

    // NEW method to visit only once all the graph
    visitor.bakeGraph(m_graph.getRoot());

    VkCommandBuffer cmdBuffer = vkm.getGraphicsCmdBuffer(framebuffer);

    // Scan render pass
    // Traverse the octree with full definition -> wait for the streaming to load the PC -> build the draw command with another octree traversal
    vkm.waitForRenderFence();
    vkm.resetCommandBuffer(cmdBuffer);
    vkm.beginCommandBuffer(cmdBuffer);
    vkm.beginScanRenderPass(framebuffer, getVkClearColor(displayParam.m_blendMode, displayParam.m_backgroundColor));
    visitor.draw_baked_pointClouds(cmdBuffer, m_pcRenderer);

    bool allScansReadyToDraw = !visitor.isPointCloudMissingPart();
    if (!allScansReadyToDraw)
    {
        std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();
        vkm.waitForStreamingIdle();
        _sleepedTime += std::chrono::duration<double, std::ratio<1, 1>>(std::chrono::steady_clock::now() - tp).count();

        vkm.resetCommandBuffer(cmdBuffer);
        vkm.beginCommandBuffer(cmdBuffer);
        vkm.beginScanRenderPass(framebuffer, getVkClearColor(displayParam.m_blendMode, displayParam.m_backgroundColor));
        visitor.draw_baked_pointClouds(cmdBuffer, m_pcRenderer);
        if (visitor.isPointCloudMissingPart())
        {
            Logger::log(LoggerMode::VKLog) << "-!- Some scans are still missing after a full download -!- The computer is probably running low on memory -!-" << Logger::endl;
            m_processSignalCancel.store(true);
            m_notEnoughMemoryForHD = true;
        }
    }

    vkm.endScanRenderPass(framebuffer);

    // First compute shader (Gap Filling / Depth correction)
    vkm.beginPostTreatmentFilling(framebuffer);
    m_postRenderer.setConstantZRange(camera.getNear(), camera.getFar(), cmdBuffer);
    m_postRenderer.setConstantScreenSize(framebuffer->extent, camera.getPixelSize1m(framebuffer->extent.width, framebuffer->extent.height), cmdBuffer);
    m_postRenderer.setConstantProjMode(camera.getProjectionMode() == ProjectionMode::Perspective, cmdBuffer);
    m_postRenderer.setConstantTexelThreshold(displayParam.m_texelThreshold, cmdBuffer);
    m_postRenderer.processPointFilling(cmdBuffer, framebuffer->descSetSamplers, framebuffer->descSetCorrectedDepth, framebuffer->extent);

    // Second compute shader (Normals)
    if (displayParam.m_postRenderingNormals.show && displayParam.m_blendMode == BlendMode::Opaque)
    {
        vkm.beginPostTreatmentNormal(framebuffer);
        m_postRenderer.setConstantScreenOffset(screenOffset, cmdBuffer);
        m_postRenderer.setConstantLighting(displayParam.m_postRenderingNormals, cmdBuffer);
        if (displayParam.m_mode  == UiRenderMode::Normals_Colored)
            m_postRenderer.processNormalColored(cmdBuffer, camera.getInversedViewUniform(m_renderSwapIndex), framebuffer->descSetSamplers, framebuffer->descSetCorrectedDepth, framebuffer->extent);
        else
            m_postRenderer.processNormalShading(cmdBuffer, camera.getInversedViewUniform(m_renderSwapIndex), framebuffer->descSetSamplers, framebuffer->descSetCorrectedDepth, framebuffer->extent);
    }

    if (displayParam.m_postRenderingAmbientOcclusion.enabled && displayParam.m_blendMode == BlendMode::Opaque)
    {
        vkm.beginPostTreatmentAmbientOcclusion(framebuffer);
        m_postRenderer.processAmbientOcclusion(cmdBuffer, displayParam.m_postRenderingAmbientOcclusion, camera.getNear(), camera.getFar(), camera.getProjectionMode() == ProjectionMode::Perspective, framebuffer->descSetSamplers, framebuffer->descSetCorrectedDepth, framebuffer->extent);
    }

    if (displayParam.m_depthLining.enabled)
    {
        vkm.beginPostTreatmentDepthLining(framebuffer);
        m_postRenderer.processDepthLining(cmdBuffer, displayParam.m_depthLining, framebuffer->descSetSamplers, framebuffer->descSetCorrectedDepth, framebuffer->extent);
    }

    if (displayParam.m_edgeAwareBlur.enabled)
    {
        vkm.beginPostTreatmentEdgeAwareBlur(framebuffer);
        m_postRenderer.processEdgeAwareBlur(cmdBuffer, displayParam.m_edgeAwareBlur, framebuffer->descSetSamplers, framebuffer->descSetCorrectedDepth, framebuffer->extent);
    }

    if (displayParam.m_blendMode != BlendMode::Opaque && displayParam.m_transparency > 0.f)
    {
        vkm.beginPostTreatmentTransparency(framebuffer);
        m_postRenderer.setConstantHDR(displayParam.m_transparency, displayParam.m_negativeEffect, displayParam.m_reduceFlash, displayParam.m_flashAdvanced, displayParam.m_flashControl, framebuffer->extent, displayParam.m_backgroundColor, cmdBuffer);
        m_postRenderer.processTransparencyHDR(cmdBuffer, framebuffer->descSetSamplers, framebuffer->extent);
    }

    // ----- Objects Render Pass -----
    // (subpass 0) - draw opaque objects
    vkm.beginObjectRenderPass(framebuffer);
    //visitor.drawMeasures(cmdBuffer, m_measureRenderer);
    visitor.draw_baked_meshes(cmdBuffer, m_simpleObjectRenderer, false);

    // (subpass 1) - draw transparent objects
    vkm.getDeviceFunctions()->vkCmdNextSubpass(cmdBuffer, VK_SUBPASS_CONTENTS_INLINE);
    visitor.draw_baked_meshes(cmdBuffer, m_simpleObjectRenderer, true);

    // (subpass 2) - blend
    vkm.getDeviceFunctions()->vkCmdNextSubpass(cmdBuffer, VK_SUBPASS_CONTENTS_INLINE);
    visitor.blendTransparentPass(cmdBuffer, m_simpleObjectRenderer, framebuffer->descSetInputTransparentLayer);

    // (subpass 3) - draw markers
    vkm.getDeviceFunctions()->vkCmdNextSubpass(cmdBuffer, VK_SUBPASS_CONTENTS_INLINE);
    SimpleBuffer& markerBuffer = framebuffer->drawMarkerBuffers[framebuffer->currentImage];
    visitor.draw_baked_markers(cmdBuffer, m_markerRenderer, framebuffer->descSetInputDepth, markerBuffer);

    // (subpass 4) - ImGui
    vkm.getDeviceFunctions()->vkCmdNextSubpass(cmdBuffer, VK_SUBPASS_CONTENTS_INLINE);
    visitor.setLastMousePosition(m_mouseData.position);
    visitor.drawImGuiBegin(m_graph.getRoot(), cmdBuffer);
    // TODO - Draw the selection rectangle with ImGui
    // Draw 2D Overlay (selection rectangle, ui, etc)
    //drawOverlayHD(cmdBuffer, camera, framebuffer->extent, screenOffset);
    visitor.drawImGuiEnd(cmdBuffer);

    // (subpass 5) - draw gizmos
    vkm.getDeviceFunctions()->vkCmdNextSubpass(cmdBuffer, VK_SUBPASS_CONTENTS_INLINE);
    //visitor.drawManipulator(cmdBuffer, &m_graph.getManipulatorManager(), m_manipulatorRenderer);
    //visitor.drawGizmo(cmdBuffer, m_gizmoParameters, m_gizmoRenderer);

    vkm.endObjectRenderPass(framebuffer);
    // -----------------------------
    //vkm.applyPicking(framebuffer, viewport.getMousePos(), nullptr);
    const bool preciseScreenshot = m_hdFormat == ImageFormat::PNG16;
    transferEvent = vkm.transferFramebufferImage(framebuffer, cmdBuffer, preciseScreenshot);
    vkm.endCommandBuffer(cmdBuffer);

    vkm.submitVirtualFramebuffer(framebuffer);

    return true;
}

void RenderingEngine::drawSelectionRect(VkCommandBuffer, const VulkanViewport& viewport)
{
    ImGuiWindowFlags windowFlags = 0;
    windowFlags |= ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoResize;
    windowFlags |= ImGuiWindowFlags_NoCollapse;
    windowFlags |= ImGuiWindowFlags_NoTitleBar;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    windowFlags |= ImGuiWindowFlags_NoScrollbar;
    windowFlags |= ImGuiWindowFlags_NoScrollWithMouse;

    VkExtent2D extent = viewport.getFramebuffer()->extent;

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2((float)extent.width, (float)extent.height), ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, 0);
    ImGui::PushStyleColor(ImGuiCol_Border, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    ImGui::Begin("SelectRect", NULL, windowFlags);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    // Selection rectangle
    {
        Rect2D rect = viewport.getHoverRect();
        if (rect.c0 != rect.c1)
        {
            dl->AddRectFilled(ImVec2(rect.c0.x, rect.c0.y), ImVec2(rect.c1.x, rect.c1.y), IM_COL32(112, 28, 222, 55));
            dl->AddRect(ImVec2(rect.c0.x, rect.c0.y), ImVec2(rect.c1.x, rect.c1.y), IM_COL32(112, 28, 222, 150), 0.f, 0, 2.f);
        }
    }

    // Polygonal selector preview
    {
        const std::vector<glm::vec2>& preview = viewport.getPolygonalSelectorPreview();
        if (!preview.empty())
        {
            ImU32 selectorColor = IM_COL32(242, 214, 0, 255);
            const float thickness = 2.0f;

            for (size_t i = 1; i < preview.size(); ++i)
            {
                ImVec2 p0(preview[i - 1].x * extent.width, preview[i - 1].y * extent.height);
                ImVec2 p1(preview[i].x * extent.width, preview[i].y * extent.height);
                dl->AddLine(p0, p1, selectorColor, thickness);
            }

            for (const glm::vec2& p : preview)
            {
                dl->AddCircleFilled(ImVec2(p.x * extent.width, p.y * extent.height), 3.0f, selectorColor);
            }

            if (viewport.isPolygonalSelectorPreviewClosed() && preview.size() > 2)
            {
                ImVec2 first(preview.front().x * extent.width, preview.front().y * extent.height);
                ImVec2 last(preview.back().x * extent.width, preview.back().y * extent.height);
                dl->AddLine(last, first, selectorColor, thickness);
            }
        }
    }

    // Polygonal selector persisted polygons (outline-only visual feedback)
    {
        if (viewport.isPolygonalSelectorActive())
        {
            const std::vector<std::vector<glm::vec2>>& polygons = viewport.getPolygonalSelectorPolygons();
            const uint32_t appliedCount = viewport.getPolygonalSelectorAppliedPolygonCount();

            for (size_t i = 0; i < polygons.size(); ++i)
            {
                const std::vector<glm::vec2>& polygon = polygons[i];
                if (polygon.size() < 3)
                    continue;

                std::vector<ImVec2> pts;
                pts.reserve(polygon.size());
                for (const glm::vec2& p : polygon)
                    pts.emplace_back(p.x * extent.width, p.y * extent.height);

                const bool applied = i < appliedCount;
                ImU32 outlineColor = applied ? IM_COL32(242, 214, 0, 230) : IM_COL32(242, 214, 0, 140);
                for (size_t k = 0; k < pts.size(); ++k)
                    dl->AddLine(pts[k], pts[(k + 1) % pts.size()], outlineColor, 1.5f);
            }
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

void RenderingEngine::drawOverlay(VkCommandBuffer cmdBuffer, const CameraNode& camera, const VkExtent2D& extent)
{
    ImGuiWindowFlags windowFlags = 0;
    windowFlags |= ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoResize;
    windowFlags |= ImGuiWindowFlags_NoCollapse;
    windowFlags |= ImGuiWindowFlags_NoTitleBar;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    windowFlags |= ImGuiWindowFlags_NoScrollbar;
    windowFlags |= ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2((float)extent.width, (float)extent.height), ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, 0);
    ImGui::PushStyleColor(ImGuiCol_Border, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    ImGui::Begin("Overlay", NULL, windowFlags);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    ImVec2 upLeft = ImVec2(0.,0.);
    ImVec2 botRight = ImVec2(extent.width, extent.height);

    //HD Frame
    if(m_showHDFrame)
    {
        ProjectionData HDFrame = camera.getTruncatedProjection(m_hdFrameRatio, HD_MARGIN);

        glm::dvec2 frameSize(HDFrame.getWidthAt1m(), HDFrame.getHeightAt1m());
        glm::dvec2 sizeCam(camera.getWidthAt1m(), camera.getHeightAt1m());

        // Compute the text position and the text background
        glm::vec2 margin((sizeCam - frameSize) / (sizeCam * 2.0));
        ImVec2 frameUpLeft = ImVec2(margin.x * extent.width, margin.y * extent.height);
        ImVec2 frameBotRight = ImVec2((1 - margin.x) * extent.width, (1 - margin.y) * extent.height);

        // Shade the viewport outside of the HD frame
        ImColor shadeColor(96, 96, 96, 216);
        dl->AddRectFilled(upLeft, ImVec2(botRight.x, frameUpLeft.y), shadeColor);
        dl->AddRectFilled(ImVec2(upLeft.x, frameBotRight.y), botRight, shadeColor);
        dl->AddRectFilled(ImVec2(upLeft.x, frameUpLeft.y), ImVec2(frameUpLeft.x, frameBotRight.y), shadeColor);
        dl->AddRectFilled(ImVec2(frameBotRight.x, frameUpLeft.y), ImVec2(botRight.x, frameBotRight.y), shadeColor);
        ImColor frameColor(238, 315, 119, 255);
        dl->AddRect(frameUpLeft, frameBotRight, frameColor, 0.f, 0, 2.f);

        if (m_showHDFrameGrid)
        {
            const float lineWidth = 1.0f;
            const float thirdWidth = (frameBotRight.x - frameUpLeft.x) / 3.0f;
            const float thirdHeight = (frameBotRight.y - frameUpLeft.y) / 3.0f;

            for (int i = 1; i <= 2; ++i)
            {
                const float x = frameUpLeft.x + thirdWidth * i;
                const float y = frameUpLeft.y + thirdHeight * i;

                dl->AddLine(ImVec2(x, frameUpLeft.y), ImVec2(x, frameBotRight.y), frameColor, lineWidth);
                dl->AddLine(ImVec2(frameUpLeft.x, y), ImVec2(frameBotRight.x, y), frameColor, lineWidth);
            }

            dl->AddLine(frameUpLeft, frameBotRight, frameColor, lineWidth);
            dl->AddLine(ImVec2(frameBotRight.x, frameUpLeft.y), ImVec2(frameUpLeft.x, frameBotRight.y), frameColor, lineWidth);
        }
    }

    // Ortho Grid
    if (camera.getProjectionMode() == ProjectionMode::Orthographic && camera.m_orthoGridActive)
    {
        float lineWidth = camera.m_orthoGridLineWidth;
        float gridSize = camera.m_orthoGridStep;
        Color32 color32 = camera.m_orthoGridColor;
        ImColor color = ImColor(color32.r, color32.g, color32.b, color32.a);

        float limitGridW = 5.f;

        float gridW = (gridSize * extent.width) / float(camera.getWidthAt1m());
        float gridH = (gridSize * extent.height) / float(camera.getHeightAt1m());

        float totalW = botRight.x - upLeft.x;
        float totalH = botRight.y - upLeft.y;

        float numLineH = std::trunc(totalH / gridH);

        if (gridW > limitGridW)
        {
            float decalH = totalH - gridH * numLineH;

            for (float w = 0.f; w < totalW; w += gridW)
            {
                ImVec2 a(std::roundf(w + upLeft.x), std::roundf(upLeft.y));
                ImVec2 b(std::roundf(w + upLeft.x), std::roundf(botRight.y));
                dl->AddLine(a, b, color, lineWidth);
            }

            for (float h = 0.f; h < totalH; h += gridH)
            {
                ImVec2 a(std::roundf(upLeft.x), std::roundf(h + decalH + upLeft.y));
                ImVec2 b(std::roundf(botRight.x), std::roundf(h + decalH + upLeft.y));
                dl->AddLine(a, b, color, lineWidth);
            }
        }

        ImUtilsText text;
        QString unitText = UnitConverter::getUnitText(camera.m_unitUsage.distanceUnit);
        double gridSizeUI = UnitConverter::meterToX(gridSize, camera.m_unitUsage.distanceUnit);

        text.text = TEXT_ORTHO_GRID_SIZE.arg(gridSizeUI).arg(unitText).toStdString();
        text.hovered = false;
        text.selected = false;

        ImVec2 textUpLeft, textBotRight, dump;

        ImGuiUtils::calcTextRect(camera, m_guiScale, text.text, upLeft.x, botRight.y, dump, textUpLeft, textBotRight);

        text.wx = upLeft.x + (textBotRight.x - textUpLeft.x) / 2;
        text.wy = botRight.y - (textBotRight.y - textUpLeft.y);

        ImGuiUtils::drawText(camera, m_guiScale, text);
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

void RenderingEngine::drawOverlayHD(VkCommandBuffer, const CameraNode& camera, const VkExtent2D& extent, const glm::vec2& screenOffset)
{
    ImGuiWindowFlags windowFlags = 0;
    windowFlags |= ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoResize;
    windowFlags |= ImGuiWindowFlags_NoCollapse;
    windowFlags |= ImGuiWindowFlags_NoTitleBar;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    windowFlags |= ImGuiWindowFlags_NoScrollbar;
    windowFlags |= ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2((float)extent.width, (float)extent.height), ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, 0);
    ImGui::PushStyleColor(ImGuiCol_Border, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    ImGui::Begin("OverlayHD", NULL, windowFlags);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    ImVec2 upLeft = ImVec2(0., 0.);
    ImVec2 botRight = ImVec2(extent.width, extent.height);

    // Ortho Grid
    if (camera.getProjectionMode() == ProjectionMode::Orthographic && camera.m_orthoGridActive)
    {
        float lineWidth = camera.m_orthoGridLineWidth;
        float gridSize = camera.m_orthoGridStep;
        Color32 color32 = camera.m_orthoGridColor;
        ImColor color = ImColor(color32.r, color32.g, color32.b, color32.a);

        float limitGridW = 5.f;

        float gridW = (gridSize * extent.width) / float(camera.getWidthAt1m());
        float gridH = (gridSize * extent.height) / float(camera.getHeightAt1m());

        float totalW = botRight.x - upLeft.x;
        float totalH = botRight.y - upLeft.y;

        float numLineH = std::trunc(totalH / gridH);

        if (gridW > limitGridW)
        {
            float decalH = totalH - gridH * numLineH;

            for (float w = 0.f; w < totalW; w += gridW)
            {
                ImVec2 a(std::roundf(w + upLeft.x), std::roundf(upLeft.y));
                ImVec2 b(std::roundf(w + upLeft.x), std::roundf(botRight.y));
                dl->AddLine(a, b, color, lineWidth);
            }

            for (float h = 0.f; h < totalH; h += gridH)
            {
                ImVec2 a(std::roundf(upLeft.x), std::roundf(h + decalH + upLeft.y));
                ImVec2 b(std::roundf(botRight.x), std::roundf(h + decalH + upLeft.y));
                dl->AddLine(a, b, color, lineWidth);
            }
        }

        ImUtilsText text;
        QString unitText = UnitConverter::getUnitText(camera.m_unitUsage.distanceUnit);
        double gridSizeUI = UnitConverter::meterToX(gridSize, camera.m_unitUsage.distanceUnit);

        text.text = TEXT_ORTHO_GRID_SIZE.arg(gridSizeUI).arg(unitText).toStdString();
        text.hovered = false;
        text.selected = false;

        ImVec2 textUpLeft, textBotRight, dump;

        ImGuiUtils::calcTextRect(camera, m_guiScale, text.text, upLeft.x, botRight.y, dump, textUpLeft, textBotRight);

        text.wx = upLeft.x + (textBotRight.x - textUpLeft.x) / 2;
        text.wy = botRight.y - (textBotRight.y - textUpLeft.y);

        ImGuiUtils::drawText(camera, m_guiScale, text);
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

void RenderingEngine::timeRendering(bool newFrameSubmited)
{
    // Manage if we wake up from a ghost mode
    if (!m_previousFrameSkipped)
    {
        // Render time of the previous frame
        m_renderTime_ms = std::chrono::duration<float, std::ratio<1, 1000>>(std::chrono::steady_clock::now() - m_startRendering).count();
    }
    else
    {
        m_renderTime_ms = 0;
    }
    m_previousFrameSkipped = !newFrameSubmited;

    for (VulkanViewport* viewport : m_viewports)
    {
        viewport->setPreviousRenderTime(m_renderTime_ms);
    }

    if (newFrameSubmited)
    {
        // Start the chrono for the frame submited
        m_startRendering = std::chrono::steady_clock::now();
    }
}

void RenderingEngine::startImGuiContext()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = m_guiScale;
    ImGui::StyleColorsDark();
}

void RenderingEngine::initVulkanImGui()
{
    VulkanManager& vkm = VulkanManager::getInstance();

    ImGui_ImplQt_InitForVulkan();
    ImGui_ImplVulkan_LoadFunctions(VK_API_VERSION_1_0, &VulkanDeviceFunctions::external_loader, vkm.getDeviceFunctions());

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.ApiVersion = VK_API_VERSION_1_0;
    init_info.Instance = vkm.getVkInstance();
    init_info.PhysicalDevice = vkm.getPhysicalDevice();
    init_info.Device = vkm.getDevice();
    init_info.QueueFamily = vkm.getGraphicsQFI();
    init_info.Queue = vkm.getGraphicsQueue();
    init_info.DescriptorPool = vkm.getDescriptorPool();
    init_info.RenderPass = vkm.getObjectRenderPass();
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.Subpass = 4;
    init_info.UseDynamicRendering = false;

    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = vkm.getCheckFunction();

    ImGui_ImplVulkan_Init(&init_info);

    // Upload Fonts
    // NOTE on ImGui function:
    // * Create its own VkDeviceMemory
    // * Don't free the (char*)pixels
    // * Don't check if the font resources have already been created (g_FontImage, g_FontMemory, g_FontView, g_UploadBuffer, g_UploadBufferMemory
    ImGui_ImplVulkan_CreateFontsTexture();
}

void RenderingEngine::shutdownImGui()
{
    // Free ImGui resources
    ImGui_ImplVulkan_Shutdown();
    ImGui::DestroyContext();
}
void RenderingEngine::processPendingScreenshots()
{
    if (m_pendingScreenshots.empty())
        return;

    VulkanManager& vkm = VulkanManager::getInstance();
    vkm.waitIdle();

    for (PendingScreenshot& pending : m_pendingScreenshots)
    {
        ImageWriter imgWriter;
        imgWriter.saveScreenshot(pending.filepath, pending.format, pending.transfer, pending.width, pending.height, pending.includeAlpha);
        m_dataDispatcher.sendControl(new control::function::ForwardMessage(new GeneralMessage(GeneralInfo::IMAGEEND)));
    }

    m_pendingScreenshots.clear();
}

void RenderingEngine::updateCompute()
{
    // Get commandBuffer for compute

    // Get MeshBuffer for input
    // Get VkBuffer for output

    //m_computePrograms.bindDescriptorBuffers();
    //m_computePrograms.processMeshDistance();

    // Submit cmdBuffer

    // Wait queue

    // Get points for buffer results and compute minimum

    // Create simple measure

    // TADA ?
}
