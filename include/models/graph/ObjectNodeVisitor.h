#ifndef OBJECT_NODE_VISITOR_H
#define OBJECT_NODE_VISITOR_H

#include "models/3d/MarkerDrawData.h"
#include "models/3d/MeshDrawData.h"
#include "models/3d/PointCloudDrawData.h"
#include "models/3d/SegmentDrawData.h"
#include "pointCloudEngine/RenderingTypes.h"
#include "models/3d/DisplayParameters.h"
#include "models/data/clipping/ClippingGeometry.h"
#include "models/graph/TransformationModule.h"

#include "utils/safe_ptr.h"

#include "vulkan/VkUniform.h"

#include <string>

class GraphManager;
class ManipulatorNode;

class AGraphNode;
class AMeasureNode;
class CameraNode;
class VulkanViewport;
class Renderer;
class ManipulatorRenderer;
class SimpleObjectRenderer;
class MeasureRenderer;
class MarkerRenderer;
class MeshObjectNode;
class FilterSystem;
class GizmoRenderer;

struct Measure;
struct SMesh;
struct TlProjectionInfo;

enum class NavigationMode;

class ObjectNodeVisitor
{
private:
    struct ManipDrawData
    {
        //glm::dmat4 transfo;
        TransformationModule transfoMod;
        //glm::dvec3 scale;
        //glm::dvec3 translation;
        //glm::dquat rotation;
        SafePtr<ManipulatorNode> node;
        //ManipulationMode mode;
        //Selection select;
    };

    struct BakedText
    {
        std::string text;
        float wx;
        float wy;
        bool selected;
        bool hovered;
        uint32_t graphicId;
    };

public:
    ObjectNodeVisitor(GraphManager& graph, VkExtent2D fbExtent, const float& guiScale, const CameraNode& camera);
    ~ObjectNodeVisitor();

    uint32_t getTextHoveredId() const;
    uint64_t getFrameHash() const;

    // ***** Setters ***** //
    void setCamera(const CameraNode& camera);
    void setLastMousePosition(const glm::ivec2& mousePos);
    void setDecimationRatio(float ratio);
    void setComplementaryRenderParameters(uint32_t swapIndex, VkFormat pointRenderFormat);
    void activateLogPerWorkload(bool octree, bool objects);

    // ***** Bake Graph ***** //
    void bakeGraph(SafePtr<AGraphNode> root); // bake all graphic datas from the graph
private:
    //void nextGeoNode(const SafePtr<AGraphNode>& node, const glm::dmat4& parent_transfo);
    //void bakeGraphics(const SafePtr<AGraphNode>& node, const glm::dmat4& gTransfo);
    //void bakeClipping(const SafePtr<AGraphNode>& node, const glm::dmat4& gTransfo);
    void nextGeoNode(const SafePtr<AGraphNode>& node, const TransformationModule& parent_transfo);
    void bakeGraphics(const SafePtr<AGraphNode>& node, const TransformationModule& gTransfo);
    void bakeClipping(const SafePtr<AGraphNode>& node, const TransformationModule& gTransfo);

public:
    // ***** Draw commands for each object type ***** //
    void drawImGuiBegin(SafePtr<AGraphNode> startNode, VkCommandBuffer cmdBuffer);
    bool drawRampOverlay();
    void drawImGuiStats(VulkanViewport& viewport);
    void drawImGuiEnd(VkCommandBuffer cmdBuffer);

    void draw_baked_pointClouds(VkCommandBuffer cmdBuffer, Renderer& renderer);
    void draw_baked_manipulators(VkCommandBuffer cmdBuffer, ManipulatorRenderer& renderer, double manipSize);
    void drawGizmo(VkCommandBuffer cmdBuffer, const glm::dvec3& gizmoParameters, std::shared_ptr<MeshBuffer> mesh, ManipulatorRenderer& renderer);
    void draw_baked_markers(VkCommandBuffer cmdBuf, MarkerRenderer& renderer, VkDescriptorSet inputAttachDescSet, SimpleBuffer& markerBuffer);
    void draw_baked_measures(VkCommandBuffer cmdBuffer, MeasureRenderer& renderer, SimpleBuffer& segmentBuffer);
    void draw_baked_meshes(VkCommandBuffer cmdBuffer, SimpleObjectRenderer& renderer, bool transparentPass);
    void blendTransparentPass(VkCommandBuffer cmdBuffer, SimpleObjectRenderer& renderer, VkDescriptorSet inputTransparentLayerDescSet);

    // ***** Miscellaneous ***** //
    bool isPointCloudMissingPart() const;
    
    // Statistics
    void getDrawCount(uint64_t& pointsDrawn, uint64_t& cellsDrawn);

private:
    void initTextsFormat();
    bool drawCameraText();
    void drawManipulator(VkCommandBuffer cmdBuffer, ManipulatorRenderer& renderer, const TransformationModule& transfo, const SafePtr<ManipulatorNode>& manip, double manipSize);
    bool drawManipulatorText();
    void getObjectMarkerText(const SafePtr<AGraphNode>& object, std::string& text) const;
    void drawObjectTexts();
    bool bakeTextPosition(const glm::dmat4& transfo, float& wx, float& wy, bool clip_max_z) const;
    void drawBakedText(const BakedText& text);
    void drawMeasureTexts();
    void drawImGuiMeasureText(const SegmentDrawData segment);

    void clipAndDrawPointCloud(VkCommandBuffer _cmdBuffer, Renderer& renderer, const PointCloudDrawData& bakedPC, TlProjectionInfo& projInfo, const ClippingAssembly& clippingAssembly);

    void sortMarkersByDepth(const std::vector<MarkerDrawData>& markers, std::vector<MarkerDrawData>& markersSorted, double maxDistance);

    std::string formatNumber(double num);
    std::string formatTemperature(double value) const;

private:
    // Scene geometric parameters
    glm::dmat4 m_viewMatrix;
    glm::dvec3 m_cameraPosition;
    glm::dmat4 m_projMatrix;
    glm::dmat4 m_viewProjMatrix;
    double m_screenHeightAt1m;
    ProjectionMode m_projMode;

    // Corresponding Uniform from the Camera
    VkUniformOffset m_viewProjUniform;
    VkUniformOffset m_clipUniform;

    // baked resources
    std::vector<BakedText>          m_bakedTexts;
    std::vector<MarkerDrawData>     m_markerDrawData;
    std::vector<MarkerDrawData>     target_draw_data_;
    // TODO - bake the targets from the graph
    // std::vector<MarkerDrawData>  m_targetDrawData;
    std::vector<SegmentDrawData>    m_segmentDrawData;
    std::vector<PointCloudDrawData> m_bakedPointCloud;
    // NOTE - Pour le futur clipping par phase on aura besoin de plusieurs clipping assembly
    ClippingAssembly m_clippingAssembly;
    std::vector<ManipDrawData>      m_manipDrawData;
    std::vector<MeshDrawData>       m_meshesDrawData;

    // Rendering parameters
    DisplayParameters               m_displayParameters;

    // Complementary Draw States
    bool                            m_drawHasMissingScanPart;
    const float                     m_guiScale;
    float                           m_decimationRatio;
    uint32_t                        m_uniformSwapIndex;
    VkFormat                        m_pointRenderFormat = VkFormat::VK_FORMAT_UNDEFINED;

    // Viewport size
    int								m_lastMI_X;
    int								m_lastMI_Y;
    float							m_tan_half_fovy;
    float							m_tan_half_fovx;
    VkExtent2D						m_fbExtent;

    // Handles
    GraphManager&			m_graph;
    const CameraNode&				m_camera;
    SafePtr<AGraphNode>             m_panoramicScan; // On utilise un AGraphNode pour pouvoir comparer le SafePtr avec les autres nodes

    bool							m_searchForClick;
    uint32_t                        m_textHoveredId;

    // Statistics
    uint64_t m_totalPointsDrawn = 0;
    uint64_t m_totalNonClippedCells = 0;
    uint64_t m_totalClippedCells = 0;
    bool m_logOctree = false;
    bool m_logObjects = false;

    // Performance variables
    std::string m_index_format;
    std::string m_coord_format;  // format des coordonnées (ImGui)
    std::string m_diameter_format;
    std::string m_length_format;
    std::string m_simple_format;

    double m_cs_marker_max_z;  // used to quicky reject markers in clip space (cs)
};

// Une Unique Interface pour les appeler tous et dans le rendu les lier.
class IRender
{
    // NOTE sur les textes :
    // - Pour les textes on fait déjà un clipping de la position avant l'édition des texts.
    // - Ce mécanisme est important pour préserver les performances.
    // - On peut d'ailleurs penser à le transposer pour d'autres affichage (segment, mesh, ect)
    // - Les textes des mesures devrait afficher comme les autres textes.
    // - Leur style peut varier (couleur) cependant.
    void getBakedTexts(const glm::dmat4& transfo, std::vector<std::string>& texts, void (*clippingFct)(int));
    void getSegmentDrawData(const glm::dmat4& transfo, std::vector<SegmentDrawData>& segments);
    void getMeshDrawData(const glm::dmat4& transfo, std::vector<MeshDrawData>& meshData);
    void getMarkerDrawData(const glm::dmat4& transfo, std::vector<MarkerDrawData>& marker_baked);
    void getPointCloudDrawData(const glm::dmat4& transfo, std::vector<PointCloudDrawData>& pc_baked);

    // Special
    bool isRenderImpacted(int renderPass);
};


#endif
