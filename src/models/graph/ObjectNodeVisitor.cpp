#include "models/graph/ObjectNodeVisitor.h"

#include "models/graph/ClusterNode.h"
#include "models/graph/BoxNode.h"
#include "models/graph/MeshObjectNode.h"
#include "models/graph/ManipulatorNode.h"
#include "models/graph/CameraNode.h"
#include "models/graph/CylinderNode.h"
#include "models/graph/SphereNode.h"
#include "models/graph/TorusNode.h"
#include "models/graph/PointCloudNode.h"
#include "models/graph/TagNode.h"
#include "models/graph/PointNode.h"
#include "models/graph/AMeasureNode.h"
#include "models/graph/ColumnTiltMeasureNode.h"
#include "models/graph/BeamBendingMeasureNode.h"

#include "models/application/Author.h"
#include "models/3d/NodeFunctions.h"

#include "models/3d/UniformClippingData.h"

#include "gui/UnitConverter.h"
#include "gui/viewport/VulkanViewport.h"

#include "pointCloudEngine/TlScanOverseer.h"
#include "pointCloudEngine/PCE_graphics.h"
#include "pointCloudEngine/RenderingEcoSystem.h"

#include "vulkan/VulkanManager.h"
#include "models/graph/GraphManager.h"
#include "vulkan/Renderers/Renderer.h"
#include "vulkan/Renderers/MeasureRenderer.h"
#include "vulkan/Renderers/MarkerRenderer.h"
#include "vulkan/Renderers/ManipulatorRenderer.h"
#include "vulkan/Renderers/SimpleObjectRenderer.h"

#include "models/3d/TemperatureScaleData.h"

#include "utils/Utils.h"
#include "utils/Logger.h"

#include <algorithm>
#include <cmath>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_vulkan.h"
#include "impl/imgui_impl_qt.h"
#include "utils/ImGuiUtils.h"

#include <fmt/format.h>

#include <qwindow.h>

#include "magic_enum/magic_enum.hpp"

static const double pi2 = (glm::pi<double>() / 2.0);
static const float fpi2 = (glm::pi<float>() / 2.0f);
#define SELECTION_COLOR glm::vec3(1.0f, 1.0f, 0.8f)


ObjectNodeVisitor::ObjectNodeVisitor(GraphManager& graph, VkExtent2D fbExtent, const float& guiScale, const CameraNode& camera)
    : m_graph(graph)
    , m_guiScale(guiScale)
    , m_fbExtent(fbExtent)
    , m_camera(camera)
    , m_searchForClick(false)
    , m_textHoveredId(INVALID_PICKING_ID)
{
    setCamera(camera);
    initTextsFormat();
    m_tan_half_fovy = (float)m_screenHeightAt1m / 2.f;
    m_tan_half_fovx = (float)m_fbExtent.width / (float)m_fbExtent.height * m_tan_half_fovy;
}

ObjectNodeVisitor::~ObjectNodeVisitor()
{
    if (m_logOctree)
    {
        Logger::log(LoggerMode::VKLog) << "Drawn octree cells : normal " << m_totalNonClippedCells << " | clipped " << m_totalClippedCells << Logger::endl;
    }
}

uint32_t ObjectNodeVisitor::getTextHoveredId() const
{
    return m_textHoveredId;
}

uint64_t ObjectNodeVisitor::getFrameHash() const
{
    return HashFrame::hashRenderingData(m_fbExtent, m_viewProjMatrix, m_clippingAssembly, m_bakedPointCloud, m_displayParameters);
}

void ObjectNodeVisitor::setCamera(const CameraNode& camera)
{
    m_viewMatrix = camera.getViewMatrix();
    m_cameraPosition = camera.getCenter();
    m_projMatrix = camera.getProjMatrix();
    m_screenHeightAt1m = camera.getHeightAt1m();
    m_projMode = camera.getProjectionMode();

    m_viewProjMatrix = m_projMatrix * m_viewMatrix;

    m_panoramicScan = camera.getPanoramicScan();
    m_displayParameters = camera.getDisplayParameters();
}

//*** Get the text according to the display filter ***/
// TODO - Passer par une interface pour obtenir les textes directement des objets
//  * Les AGraphNode/AGraphObject renvoient leurs chaines de caractères pour chaque pôle.
//  * Le filtre global est donné en paramètre à l'objet pour éviter des générations inutiles.
//  * Le visitor/text renderer fait la mise en forme finale (index.name.id...).
void ObjectNodeVisitor::getObjectMarkerText(const SafePtr<AGraphNode>& object, std::string& text) const
{
    std::array<std::string, 9> parameters;
    TextFilter objectAvailableParametersDisplay = TEXT_SHOW_STANDARD_BIT;
    
    TextFilter filter;
    ElementType type;
    SafePtr<Author> author;

    {
        ReadPtr<AGraphNode> rObj = object.cget();
        if (!rObj)
            return;

        type = rObj->getType();

        switch (type) {
            case ElementType::Cylinder:
                objectAvailableParametersDisplay |= TEXT_SHOW_LENGTH_BIT;
                objectAvailableParametersDisplay |= TEXT_SHOW_DIAMETER_BIT;
                break;
            case ElementType::Scan:
                objectAvailableParametersDisplay = TEXT_SHOW_COORD_BIT | TEXT_SHOW_NAME_BIT;
                break;
            case ElementType::Point:
            case ElementType::Tag:
                objectAvailableParametersDisplay |= TEXT_SHOW_COORD_BIT;
                break;
            case ElementType::Target:
                objectAvailableParametersDisplay = 0;
                break;
            case ElementType::SimpleMeasure:
            case ElementType::PolylineMeasure:
            case ElementType::PipeToPipeMeasure:
            case ElementType::PipeToPlaneMeasure:
            case ElementType::PointToPipeMeasure:
            case ElementType::PointToPlaneMeasure:
            case ElementType::Cluster:
                objectAvailableParametersDisplay = 0;
                break;
        }
        filter = objectAvailableParametersDisplay & m_displayParameters.m_textOptions.m_filter;
        author = rObj->getAuthor();

        if (filter & TEXT_SHOW_INDEX_BIT)
        {
            // Complete with zeros
            parameters[0] = fmt::format(fmt::runtime(m_index_format), rObj->getUserIndex());
        }
        if (filter & TEXT_SHOW_IDENTIFIER_BIT)
            parameters[2] = Utils::to_utf8(rObj->getIdentifier());
        if (filter & TEXT_SHOW_NAME_BIT)
            parameters[3] = Utils::to_utf8(rObj->getName());
        if (filter & TEXT_SHOW_DISCIPLINE_BIT)
            parameters[4] = Utils::to_utf8(rObj->getDiscipline());
        if (filter & TEXT_SHOW_PHASE_BIT)
            parameters[5] = Utils::to_utf8(rObj->getPhase());

        // NOTE(robin) - Mise en forme optimisée pour prendre moins de temps à l’éxécution.
        // PERF - La mise en forme des coordonnées est malgré tout relativement longue. On pourrait penser à stocker la mise en forme entre chaque frame.
        if (filter & TEXT_SHOW_COORD_BIT)
        {
            glm::dvec3 pos = UnitConverter::meterToX(rObj->getCenter(), m_displayParameters.m_unitUsage.distanceUnit);

            // Format the coordinates values
            parameters[8] = fmt::format(fmt::runtime(m_coord_format), pos.x, pos.y, pos.z);
        }
    }

    if (filter & TEXT_SHOW_AUTHOR_BIT)
    {
        ReadPtr<Author> rAuth = author.cget();
        if (rAuth)
            parameters[1] = Utils::to_utf8(rAuth->getName());
    }

    if (type == ElementType::Cylinder)
    {
        ReadPtr<CylinderNode> rCylinder = static_pointer_cast<CylinderNode>(object).cget();

        if (rCylinder && (filter & TEXT_SHOW_DIAMETER_BIT))
        {
            double r = UnitConverter::meterToX(2.0 * rCylinder->getRadius(), m_displayParameters.m_unitUsage.diameterUnit);
            parameters[6] = fmt::format(fmt::runtime("diam=" + m_diameter_format), r);
        }
        if (filter & TEXT_SHOW_LENGTH_BIT)
        {
            double l = UnitConverter::meterToX(rCylinder->getLength(), m_displayParameters.m_unitUsage.distanceUnit);
            parameters[7] = fmt::format(fmt::runtime("len=" + m_length_format), l);
        }
    }

    if (nodeFunctions::isMissingFile(object))
        text = "?" + text;

    // NOTE(robin) - Le layout indique de façon efficace ou placer les 'parameters' avec les séparateurs '.' et les sauts de ligne '\n'.
    // - Fonctionne jusqu’à 10 parametres (actuellement 9 utilisés), au délà il faudra faire plus d’efforts :)
    // - Il manque certaines sécurités si le layout est mal écrit.
    char layout[] = "0.1.2.3\n4.5\n6.7\n8";
    bool empty_line = true;
    bool add_sep = false;
    for (int i = 0; i < sizeof(layout); ++i)
    {
        char c = layout[i];
        switch (c) {
        case '.':
            add_sep = !empty_line;
            break;
        case '\n':
            if (empty_line)
                break;
            text += c;
            empty_line = true;
            add_sep = false;
            break;
        case '\0':
            break;
        default: // works for positional in [0;9]
            int p = int(c) - 48;
            if (parameters[p].size() > 0)
            {
                if (add_sep)
                    text += '.';
                text += parameters[p];
                empty_line = false;
            }
            break;
        }
    }
}

bool ObjectNodeVisitor::drawManipulatorText()
{
    ReadPtr<ManipulatorNode> rManipNode = m_graph.getManipulatorNode().cget();
    if (!rManipNode)
        return false;

    if (!rManipNode->isDisplayed() || rManipNode->getCurrentSelection() == Selection::None)
        return false;

    std::string text(magic_enum::enum_name(rManipNode->getCurrentSelection()));
    switch (rManipNode->getManipulationMode())
    {
    case ManipulationMode::Extrusion:
    case ManipulationMode::Scale:
    case ManipulationMode::Translation:
        text += " : " + fmt::format(fmt::runtime(m_length_format), UnitConverter::meterToX(rManipNode->getDistanceToDisplay(), m_camera.m_unitUsage.distanceUnit));
        break;
    case ManipulationMode::Rotation:
        text += " : " + fmt::format(fmt::runtime(m_simple_format), rManipNode->getDistanceToDisplay(), "Â°");
        break;
    }

    ImUtilsText toDraw;
    toDraw.hovered = false;
    toDraw.selected = false;
    toDraw.text = text;
    toDraw.wx = m_lastMI_X + 15.f;
    toDraw.wy = m_lastMI_Y + 15.f;

    ImVec2 dump, upLeft, botRight;
    ImGuiUtils::calcTextRect(m_displayParameters, m_guiScale, toDraw.text, toDraw.wx, toDraw.wy, dump, upLeft, botRight);

    toDraw.wx += (botRight.x - upLeft.x) * 0.5f;
    toDraw.wy += (botRight.y - upLeft.y) * 0.5f;

    if (!ImGuiUtils::drawText(m_displayParameters, m_guiScale, toDraw))
        return false;
    return true;
}

void ObjectNodeVisitor::drawObjectTexts()
{
    for (const BakedText& text : m_bakedTexts)
    {
        drawBakedText(text);
    }
}

bool ObjectNodeVisitor::bakeTextPosition(const glm::dmat4& transfo, float& _wx, float& _wy, bool clip_max_z) const
{
    // FIXME - Si cette fonction est géré par l'objet, alors on peut ajouter une transfo locale propre à l'objet
    glm::dvec4 csPos = m_viewProjMatrix * glm::dvec4(transfo[3][0], transfo[3][1], transfo[3][2], 1.0);
    // NOTE - On peut désactiver le clipping en z avec un booléen.
    if (csPos.z < 0.0 || (clip_max_z && csPos.z > m_cs_marker_max_z))
        return false;

    glm::dvec4 ndcPos = csPos / csPos.w;
    if (abs(ndcPos.x) >= 1.1 || abs(ndcPos.y) >= 1.1)
        return false;

    _wx = ((float)ndcPos.x + 1.f) * 0.5f * float(m_fbExtent.width);
    _wy = ((float)ndcPos.y + 1.f) * 0.5f * float(m_fbExtent.height);
    if (glm::isnan(_wx) || glm::isnan(_wy))
        return false;

    return true;
}

void ObjectNodeVisitor::drawBakedText(const BakedText& bakedText)
{
    ImUtilsText toDraw;
    toDraw.hovered = bakedText.hovered;
    toDraw.selected = bakedText.selected;
    toDraw.text = bakedText.text;
    toDraw.wx = bakedText.wx;
    toDraw.wy = bakedText.wy;

    ImVec2 dump, upLeft, botRight;
    ImGuiUtils::calcTextRect(m_displayParameters, m_guiScale, toDraw.text, toDraw.wx, toDraw.wy, dump, upLeft, botRight);

    if (!ImGuiUtils::drawText(m_displayParameters, m_guiScale, toDraw))
        return;

    if (upLeft.x <= m_lastMI_X && botRight.x >= m_lastMI_X && upLeft.y <= m_lastMI_Y && botRight.y >= m_lastMI_Y)
        m_textHoveredId = bakedText.graphicId;
}

void ObjectNodeVisitor::initTextsFormat()
{
    // Prepare text formating for the different texts displayed with each objects (coordinates, radius, length).
    // This simple optimisation can save some precious microseconds when there is a lot of labels to draw.
    // Any further options for parameters display (traduction, precision, unit, etc) should be implemented here.
    {
        // NOTE - The unit converter already include a space before the unit.
        std::string diam_unit = UnitConverter::getUnitText(m_displayParameters.m_unitUsage.diameterUnit).toStdString();
        std::string dist_unit = UnitConverter::getUnitText(m_displayParameters.m_unitUsage.distanceUnit).toStdString();
        uint32_t digits = m_displayParameters.m_unitUsage.displayedDigits; // just a shorter name :)

        // See https://fmt.dev/latest/syntax.html#syntax for more infos about the fmt formating options. 
        m_index_format = fmt::format("{{:0{}d}}", 3);
        m_diameter_format = fmt::format("{{:.{0}f}}{1}", digits, diam_unit);
        m_length_format = fmt::format("{{:.{0}f}}{1}", digits, dist_unit);
        m_simple_format = fmt::format("{{:.{0}f}}{{}}", digits);
        m_coord_format = fmt::format("({{:.{0}f}}{1}, {{:.{0}f}}{1}, {{:.{0}f}}{1})", digits, dist_unit);
    }
}

bool ObjectNodeVisitor::drawCameraText()
{
    // Draw the text for the camera
    //  - The camera text should only be seen in perspective mode
    //  - The camera text is only seen by other camera
    if (m_camera.getProjectionMode() == ProjectionMode::Orthographic)
        return false;

    glm::dvec4 wsPos(m_camera.AGraphNode::getTranslation(true), 1.0);

    if ((m_viewMatrix * wsPos).z > m_displayParameters.m_markerOptions.maximumDisplayDistance)
        return false;

    glm::dvec4 csPos = m_viewProjMatrix * wsPos;

    glm::dvec4 ndcPos = csPos / csPos.w;
    float wx(((float)ndcPos.x + 1.f) * 0.5f * float(m_fbExtent.width));
    float wy(((float)ndcPos.y + 1.f) * 0.5f * float(m_fbExtent.height));

    if (csPos.z < 0.0f)
        return false;

    std::string text = Utils::to_utf8(m_camera.Data::getName());
    // Compute the text position and the text background


    ImUtilsText toDraw;
    toDraw.hovered = false;
    toDraw.selected = false;
    toDraw.text = text;
    toDraw.wx = wx;
    toDraw.wy = wy;

    if (!ImGuiUtils::drawText(m_displayParameters, m_guiScale, toDraw))
        return false;

    /*if (text != "") {


        ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
        ImVec2 textPos(ImVec2(wx - textSize.x * 0.5f, wy - textSize.y * 0.5f));
        ImVec2 margin(4.f * m_guiScale, 2.f * m_guiScale);
        ImVec2 a(textPos.x - margin.x, textPos.y - margin.y);
        ImVec2 b(textPos.x + textSize.x + margin.x, textPos.y + textSize.y + margin.y);
        ImU32 textFillColor, textTextColor;
        ImGuiUtils::giveTheme(m_displayParameters, textFillColor, textTextColor);

        dl->AddRectFilled(a, b, textFillColor, 2.f);
        dl->AddText(textPos, textTextColor, text.c_str());
    }
    */

    return true;
}

void ObjectNodeVisitor::drawMeasureTexts()
{
    if (!(m_displayParameters.m_measureMask & SHOW_VALUES))
        return;

    for (const SegmentDrawData& segmentData : m_segmentDrawData)
    {
        drawImGuiMeasureText(segmentData);
    }
}

void ObjectNodeVisitor::drawImGuiMeasureText(const SegmentDrawData segment)
{
    MeasureShowMask mask = m_displayParameters.m_measureMask & segment.showMask;

    // World Space coordinates
    glm::dvec4 wsPos_lowZ;
    glm::dvec4 wsPos_highZ;
    glm::dvec4 originPos(segment.pointA[0], segment.pointA[1], segment.pointA[2], 1.0);
    glm::dvec4 finalPos(segment.pointB[0], segment.pointB[1], segment.pointB[2], 1.0);
    if (originPos.z < finalPos.z)
    {
        wsPos_lowZ = originPos;
        wsPos_highZ = finalPos;
    }
    else
    {
        wsPos_lowZ = finalPos;
        wsPos_highZ = originPos;
    }
    glm::dvec4 wsPos_ortho(wsPos_highZ.x, wsPos_highZ.y, wsPos_lowZ.z, 1.0);

    // Clip Space coordinates
    glm::dvec4 csPos_lowZ = m_viewProjMatrix * wsPos_lowZ;
    glm::dvec4 csPos_highZ = m_viewProjMatrix * wsPos_highZ;
    if ((csPos_lowZ.z + csPos_highZ.z) / 2.0 > m_cs_marker_max_z)
        return;
    glm::dvec4 csPos_ortho = m_viewProjMatrix * wsPos_ortho;

    glm::dvec4 csPos_mid[3] = {
        (csPos_lowZ + csPos_highZ) / 2.0,
        (csPos_lowZ + csPos_ortho) / 2.0,
        (csPos_highZ + csPos_ortho) / 2.0
    };

    double length[3] = {
        glm::distance(wsPos_lowZ, wsPos_highZ),
        glm::distance(wsPos_lowZ, wsPos_ortho),
        glm::distance(wsPos_highZ, wsPos_ortho)
    };

    bool showSegment[3] = {
        (mask & SHOW_MAIN_SEGMENT) == SHOW_MAIN_SEGMENT,
        (mask & SHOW_HORIZONTAL_SEGMENT) == SHOW_HORIZONTAL_SEGMENT,
        (mask & SHOW_VERTICAL_SEGMENT) == SHOW_VERTICAL_SEGMENT
    };

    float fontSize = m_displayParameters.m_textOptions.m_textFontSize;
    for (int i = 0; i < 3; ++i)
    {
        if (!showSegment[i])
            continue;

        glm::dvec4 ndcPos = csPos_mid[i] / csPos_mid[i].w;

        float wx((ndcPos.x + 1.f) * 0.5f * (float)m_fbExtent.width);
        float wy((ndcPos.y + 1.f) * 0.5f * (float)m_fbExtent.height);

        if (csPos_mid[i].z < 0.0f)
            continue;

        std::string number = fmt::format(fmt::runtime(m_length_format), UnitConverter::meterToX(length[i], m_displayParameters.m_unitUsage.distanceUnit));

        ImUtilsText toDraw;
        //TO DO : Use segments hovered/selected data
        toDraw.hovered = false;
        toDraw.selected = false;
        toDraw.text = number;
        toDraw.wx = wx;
        toDraw.wy = wy;

        ImVec2 dump, upLeft, botRight;
        ImGuiUtils::calcTextRect(m_displayParameters, m_guiScale, toDraw.text, toDraw.wx, toDraw.wy, dump, upLeft, botRight);

        if (!ImGuiUtils::drawText(m_displayParameters, m_guiScale, toDraw))
            return;

        if (upLeft.x <= m_lastMI_X && botRight.x >= m_lastMI_X && upLeft.y <= m_lastMI_Y && botRight.y >= m_lastMI_Y)
            m_textHoveredId = segment.index;

        /*
        // Compute the text position and the text background
        ImVec2 textSize = ImGui::CalcTextSize(number.c_str());
        float defaultFontSize = ImGui::CalcTextSize("").y;
        textSize.x *= fontSize / defaultFontSize;
        textSize.y *= fontSize / defaultFontSize;

        ImVec2 textPos(wx - textSize.x * 0.5f, wy - textSize.y * 0.5f);
        ImVec2 margin(4.f * m_guiScale, 2.f * m_guiScale);
        ImVec2 a(textPos.x - margin.x, textPos.y - margin.y);
        ImVec2 b(textPos.x + textSize.x + margin.x, textPos.y + textSize.y + margin.y);
        ImU32 textFillColor, textTextColor;
        // NOTE(robin) - try to use a special coloration for the measure texts
        ImGuiUtils::giveMeasureTheme(m_displayParameters, i, textFillColor, textTextColor);
        dl->AddRectFilled(a, b, textFillColor, 2.f);
        dl->AddText(NULL, fontSize, textPos, textTextColor, number.c_str());
        if (a.x <= m_lastMI_X && b.x >= m_lastMI_X && a.y <= m_lastMI_Y && b.y >= m_lastMI_Y)
            m_textHoveredId = segment.index;
           */
    }
}

// inputs :
//   * VP size & scale
//   * Ramp color scheme (future)
//   * Ramp steps count
//   * Unit system
//   * font size (implicit with ImGui)
void ObjectNodeVisitor::drawRampOverlay()
{
    // TODO - Afficher l'echelle pour le mode distance
    //m_displayParameters.m_mode != UiRenderMode::Distance_Ramp &&
    //m_displayParameters.m_mode != UiRenderMode::Flat_Distance_Ramp &&
    if (m_displayParameters.m_rampScale.showTemperatureScale)
    {
        TemperatureScaleData temperatureScale = m_graph.getTemperatureScaleData();
        if (!temperatureScale.isValid || temperatureScale.entries.empty())
            return;

        // Constants
        ImVec2 margin(10.f, 10.f);
        ImVec2 internMargin(10.f, 10.f);
        constexpr float blank = 5.f;
        ImVec2 smallDashSize(6.f, 1.f);
        ImVec2 bigDashSize(10.f, 2.f);
        constexpr float scale_width = 25.f;
        int graduation = m_displayParameters.m_rampScale.graduationCount;

        const float fontSize = m_displayParameters.m_textOptions.m_textFontSize;

        const double entryFirst = temperatureScale.entries.front().temperature;
        const double entryLast = temperatureScale.entries.back().temperature;
        const double vmin = std::min(entryFirst, entryLast);
        const double vmax = std::max(entryFirst, entryLast);
        const int steps = static_cast<int>(temperatureScale.entries.size());
        if (steps == 0)
            return;
        const bool isAscending = entryFirst <= entryLast;

        ImVec2 textSizeVMin = ImGui::CalcTextSize(formatTemperature(vmin).c_str());
        ImVec2 textSizeVMax = ImGui::CalcTextSize(formatTemperature(vmax).c_str());
        ImVec2 textSize(std::max(textSizeVMin.x, textSizeVMax.x), std::max(textSizeVMin.y, textSizeVMax.y));
        float defaultFontSize = ImGui::CalcTextSize("").y;
        textSize.x *= fontSize / defaultFontSize;
        textSize.y *= fontSize / defaultFontSize;

        ImVec2 wndSize(internMargin.x * 2 + textSize.x + blank + bigDashSize.x + scale_width, (float)m_fbExtent.height - margin.y * 2);
        float internSizeY = wndSize.y - internMargin.y * 2;

        if (wndSize.x > m_fbExtent.width || internSizeY < 10.f)
            return;

        ImVec2 wndPos((float)m_fbExtent.width - wndSize.x - margin.x, margin.y);
        float text_x = wndPos.x + internMargin.x;
        float big_dash_x = text_x + textSize.x + blank;
        float small_dash_x = big_dash_x + (bigDashSize.x - smallDashSize.x);
        float scale_x = big_dash_x + bigDashSize.x;

        ImGuiWindowFlags windowFlags = 0;
        windowFlags |= ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoResize;
        windowFlags |= ImGuiWindowFlags_NoCollapse;
        windowFlags |= ImGuiWindowFlags_NoTitleBar;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
        windowFlags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        ImGui::SetNextWindowPos(wndPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(wndSize, ImGuiCond_Always);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(48, 48, 48, 192));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);

        ImGui::Begin("Temperature scale overlay", NULL, windowFlags);
        ImDrawList* dl = ImGui::GetWindowDrawList();

        float dyGrad = (internSizeY - textSize.y) / graduation;
        float lastTextY = -std::numeric_limits<float>::infinity();
        for (int i = 0; i < graduation + 1; ++i)
        {
            float text_y = wndPos.y + internMargin.y + dyGrad * i;
            float dash_y = text_y + textSize.y / 2.f;
            if (text_y < lastTextY + textSize.y * 1.5f)
            {
                ImVec2 dashPos = ImVec2(small_dash_x, dash_y - smallDashSize.y / 2.f);
                dl->AddRectFilled(dashPos, dashPos + smallDashSize, IM_COL32_WHITE);
            }
            else
            {
                int entryIndex = static_cast<int>(std::round(static_cast<double>(i) * (steps - 1) / graduation));
                entryIndex = std::clamp(entryIndex, 0, steps - 1);
                if (isAscending)
                    entryIndex = steps - 1 - entryIndex;
                const double v = temperatureScale.entries[entryIndex].temperature;
                std::string text = formatTemperature(v);
                dl->AddText(NULL, fontSize, ImVec2(text_x, text_y), IM_COL32_WHITE, text.c_str());
                lastTextY = text_y;
                ImVec2 dashPos = ImVec2(big_dash_x, dash_y - bigDashSize.y / 2.f);
                dl->AddRectFilled(dashPos, dashPos + bigDashSize, IM_COL32_WHITE);
            }
        }

        float dY = (internSizeY - textSize.y) / steps;
        for (int i = 0; i < steps; ++i)
        {
            int entryIndex = isAscending ? (steps - 1 - i) : i;
            const TemperatureScaleEntry& entry = temperatureScale.entries[entryIndex];
            ImVec2 rect = ImVec2(scale_x, wndPos.y + internMargin.y + textSize.y / 2.f + dY * i);
            dl->AddRectFilled(rect, rect + ImVec2(scale_width, dY), IM_COL32(entry.r, entry.g, entry.b, 255));
        }

        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        return;
    }

    if (!m_displayParameters.m_rampScale.showScale)
        return;

    bool rampSelected = false;
    std::shared_ptr<IClippingGeometry> rampToOverlay;
    for (const std::shared_ptr<IClippingGeometry>& ramp : m_clippingAssembly.rampActives)
    {
        if (!ramp->isSelected)
            continue;

        if(!rampSelected)
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

    if (rampToOverlay == nullptr)
        return;

    // Constants
    ImVec2 margin(10.f, 10.f);
    ImVec2 internMargin(10.f, 10.f);
    constexpr float blank = 5.f;  // let a gap between the text and the dash
    ImVec2 smallDashSize(6.f, 1.f);
    ImVec2 bigDashSize(10.f, 2.f);
    constexpr float scale_width = 25.f;
    int graduation = m_displayParameters.m_rampScale.graduationCount;

    // Adjuts to the text font and content
    const float fontSize = m_displayParameters.m_textOptions.m_textFontSize;

    // TODO - get this parameters with a virtual method
    float vmin = 0.f;
    float vmax = 0.f;
    glm::vec4 params = rampToOverlay->params;
    switch (rampToOverlay->getShape())
    {
    case ClippingShape::box:
        if (m_displayParameters.m_rampScale.centerBoxScale)
        {
            vmin = -params[2];
            vmax = params[2];
        }
        else
        {
            vmin = 0.f;
            vmax = params[2] * 2;
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

    const int steps = rampToOverlay->rampSteps;
    if (steps == 0)
        return;

    ImVec2 textSizeVMin = ImGui::CalcTextSize(formatNumber(vmin).c_str());
    ImVec2 textSizeVMax = ImGui::CalcTextSize(formatNumber(vmax).c_str());
    ImVec2 textSize(std::max(textSizeVMin.x, textSizeVMax.x), std::max(textSizeVMin.y, textSizeVMax.y));
    // Compute the text size with the font size
    float defaultFontSize = ImGui::CalcTextSize("").y;
    textSize.x *= fontSize / defaultFontSize;
    textSize.y *= fontSize / defaultFontSize;

    ImVec2 wndSize(internMargin.x * 2 + textSize.x + blank + bigDashSize.x + scale_width, (float)m_fbExtent.height - margin.y * 2);
    float internSizeY = wndSize.y - internMargin.y * 2;

    // Exit if not enough space to draw
    if (wndSize.x > m_fbExtent.width || internSizeY < 10.f)
        return;

    ImVec2 wndPos((float)m_fbExtent.width - wndSize.x - margin.x, margin.y);
    float text_x = wndPos.x + internMargin.x;
    float big_dash_x = text_x + textSize.x + blank;
    float small_dash_x = big_dash_x + (bigDashSize.x - smallDashSize.x);
    float scale_x = big_dash_x + bigDashSize.x;

    ImGuiWindowFlags windowFlags = 0;
    windowFlags |= ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoResize;
    windowFlags |= ImGuiWindowFlags_NoCollapse;
    windowFlags |= ImGuiWindowFlags_NoTitleBar;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    windowFlags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::SetNextWindowPos(wndPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(wndSize, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(48, 48, 48, 192));
    //ImGui::PushStyleColor(ImGuiCol_Border, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);

    ImGui::Begin("Ramp overlay", NULL, windowFlags);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    float dyGrad = (internSizeY - textSize.y) / graduation;
    float lastTextY = -std::numeric_limits<float>::infinity();
    for (int i = 0; i < graduation + 1; ++i)
    {
        float text_y = wndPos.y + internMargin.y + dyGrad * i;
        float dash_y = text_y + textSize.y / 2.f;
        if (text_y < lastTextY + textSize.y * 1.5f)
        {
            // no text, small graduation
            ImVec2 dashPos = ImVec2(small_dash_x, dash_y - smallDashSize.y / 2.f);
            dl->AddRectFilled(dashPos, dashPos + smallDashSize, IM_COL32_WHITE);
        }
        else
        {
            // text, big graduation
            float v = vmax + (vmin - vmax) * i / graduation;
            std::string text = formatNumber(v);
            // text
            dl->AddText(NULL, fontSize, ImVec2(text_x, text_y), IM_COL32_WHITE, text.c_str());
            lastTextY = text_y;
            // dash
            ImVec2 dashPos = ImVec2(big_dash_x, dash_y - bigDashSize.y / 2.f);
            dl->AddRectFilled(dashPos, dashPos + bigDashSize, IM_COL32_WHITE);
        }

    }

    // Color Scale
    std::vector<ImU32> rampScale;
    ImGuiUtils::computeRampScale(steps, rampScale);
    float dY = (internSizeY - textSize.y) / steps;
    for (int i = 0; i < steps; ++i)
    {
        ImVec2 rect = ImVec2(scale_x, wndPos.y + internMargin.y + textSize.y / 2.f + dY * i);
        dl->AddRectFilled(rect, rect + ImVec2(scale_width, dY), rampScale[i]);
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    //ImGui::PopStyleColor();
}

float getCumulValue(void const* data, int i, int j) {
    float value = 0.0f;
    switch (j)
    {
    case 2:
        value += static_cast<FrameStats const*>(data)[i].prepareScansTime;
    case 1:
        value += static_cast<FrameStats const*>(data)[i].prepareObjectsTime;
    case 0:
        value += static_cast<FrameStats const*>(data)[i].bakeGraph;
        break;
    case 3:
        return static_cast<FrameStats const*>(data)[i].renderTime;
    default:
        return 0.0f;
    }
    return value;
}

void ObjectNodeVisitor::drawImGuiStats(VulkanViewport& viewport)
{
    if (!viewport.m_displayRenderStats)
        return;
    uint32_t frameStackIndex = viewport.m_frameStackIndex;
    std::array<FrameStats, 120>& frameStack = viewport.m_frameStack;

    ImGuiWindowFlags windowFlags = 0;
    windowFlags |= ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoCollapse;
    windowFlags |= ImGuiWindowFlags_AlwaysAutoResize;
    windowFlags |= ImGuiWindowFlags_NoScrollbar;
    windowFlags |= ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::SetNextWindowPos(ImVec2(viewport.width(), 0), ImGuiCond_Always, ImVec2(1.0, 0.0));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);

    ImGui::Begin("Rendering Statistics [F3] CSV [F4]", NULL, windowFlags);
    ImGui::PushItemWidth(ImGui::GetFontSize() * 10);

    ImGuiUtils::label_big_number("Total cell drawn", frameStack[frameStackIndex].cellCount);
    ImGuiUtils::label_big_number("Total point drawn", frameStack[frameStackIndex].pointCount);
    ImGui::LabelText("Decimation", "x%.2f", frameStack[frameStackIndex].decimation);
    int prevFSI = (frameStackIndex - 2) % frameStack.size();
    ImGui::LabelText("Last frame (ms)", "%.3f", frameStack[prevFSI].renderTime);

    ImGui::PopItemWidth();

    struct Funcs
    {
        static float showRenderTime(void* data, int i) {
            return static_cast<FrameStats*>(data)[i].renderTime;
        }
        static float showPrepareScanTime(void* data, int i) {
            return static_cast<FrameStats*>(data)[i].prepareScansTime;
        }
        static float showPrepareObjectTime(void* data, int i) {
            return static_cast<FrameStats*>(data)[i].prepareObjectsTime;
        }
        static float showBakeGraphTime(void* data, int i) {
            return static_cast<FrameStats*>(data)[i].bakeGraph;
        }
    };

    FrameStats mean_frame = { 0, 0, 0.f, 0.f, 0.f, 0.f, 1.f };
    for (const FrameStats& stats : frameStack)
    {
        mean_frame.bakeGraph += stats.bakeGraph;
        mean_frame.prepareScansTime += stats.prepareScansTime;
        mean_frame.prepareObjectsTime += stats.prepareObjectsTime;
        mean_frame.renderTime += stats.renderTime;
    }
    mean_frame.bakeGraph /= frameStack.size();
    mean_frame.prepareScansTime /= frameStack.size();
    mean_frame.prepareObjectsTime /= frameStack.size();
    mean_frame.renderTime /= frameStack.size();

    int display_count = (int)frameStack.size();
    float wSize = 3 * display_count + 8;

    char title_bake[40];
    snprintf(title_bake, 40, "Graph baking ~%.1f ms", mean_frame.bakeGraph);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.9f, 0.2f, 0.75f, 1.f));
    ImGui::PlotHistogram("30 ms", Funcs::showBakeGraphTime, frameStack.data(), display_count, 0, title_bake, 0.0f, 30.0f, ImVec2(wSize, 100.f));
    ImGui::PopStyleColor();

    char title_objects[40];
    snprintf(title_objects, 40, "Common Objects render ~%.1f ms", mean_frame.prepareObjectsTime);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.5f, 0.8f, 0.3f, 1.f));
    ImGui::PlotHistogram("30 ms", Funcs::showPrepareObjectTime, frameStack.data(), display_count, 0, title_objects, 0.0f, 30.0f, ImVec2(wSize, 100.f));
    ImGui::PopStyleColor();

    char title_scans[40];
    snprintf(title_scans, 40, "Scans render ~%.1f ms", mean_frame.prepareScansTime);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.1f, 0.7f, 0.9f, 1.f));
    ImGui::PlotHistogram("80 ms", Funcs::showPrepareScanTime, frameStack.data(), display_count, 0, title_scans, 0.0f, 80.0f, ImVec2(wSize, 100));
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.0f, 0.2f, 1.f));
    ImGui::PlotHistogram("80 ms", Funcs::showRenderTime, frameStack.data(), display_count, 0, "Render time [GPU] (max 80ms)", 0.0f, 80.0f, ImVec2(wSize, 100));
    ImGui::PopStyleColor();


    ImGuiUtils::plotMultiHistogram(frameStack);
    //ImGui::SetWindowPos(ImVec2(width() - ImGui::GetCurrentWindow()->ContentSize.x, 0), ImGuiCond_Always);
    
    ImGui::End();
    ImGui::PopStyleVar();

}

std::string ObjectNodeVisitor::formatNumber(double num)
{
    double d = UnitConverter::meterToX(num, m_displayParameters.m_unitUsage.distanceUnit);
    return fmt::format(fmt::runtime(m_length_format), d);
}

std::string ObjectNodeVisitor::formatTemperature(double value) const
{
    return fmt::format(fmt::runtime(m_simple_format), value, " Celsius");
}

void ObjectNodeVisitor::setLastMousePosition(const glm::ivec2& mousePos)
{
    m_lastMI_X = mousePos.x;
    m_lastMI_Y = mousePos.y;
}

void ObjectNodeVisitor::setDecimationRatio(float ratio)
{
    m_decimationRatio = ratio;
}

void ObjectNodeVisitor::setComplementaryRenderParameters(uint32_t swapIndex, VkFormat pointRenderFormat)
{
    m_uniformSwapIndex = swapIndex;
    m_viewProjUniform = m_camera.getViewProjUniform(m_uniformSwapIndex);
    m_clipUniform = m_camera.getClippingUniform(m_uniformSwapIndex);
    m_pointRenderFormat = pointRenderFormat;
}

void ObjectNodeVisitor::activateLogPerWorkload(bool logOctree, bool logObjects)
{
    m_logOctree = logOctree;
    m_logObjects = logObjects;
}

bool ObjectNodeVisitor::isPointCloudMissingPart() const
{
    return (m_drawHasMissingScanPart);
}

std::array<glm::vec3, (size_t)Selection::MAX_ENUM> g_selectionColor =
{
    glm::vec3(0.5, 0.5, 0.5), // None
    glm::vec3(1.0, 0.0, 0.0), // X
    glm::vec3(1.0, 0.0, 0.0), // _X
    glm::vec3(0.0, 1.0, 0.0), // Y
    glm::vec3(0.0, 1.0, 0.0), // _Y
    glm::vec3(0.0, 0.0, 1.0), // Z
    glm::vec3(0.0, 0.0, 1.0), // _Z
    glm::vec3(1.0, 1.0, 0.0), // XY
    glm::vec3(1.0, 1.0, 0.0), // _XY
    glm::vec3(1.0, 1.0, 0.0), // X_Y
    glm::vec3(1.0, 1.0, 0.0), // _X_Y
    glm::vec3(1.0, 0.0, 1.0), // XZ
    glm::vec3(1.0, 0.0, 1.0), // _XZ
    glm::vec3(1.0, 0.0, 1.0), // X_Z
    glm::vec3(1.0, 0.0, 1.0), // _X_Z
    glm::vec3(0.0, 1.0, 1.0), // YZ
    glm::vec3(0.0, 1.0, 1.0), // _YZ
    glm::vec3(0.0, 1.0, 1.0), // Y_Z
    glm::vec3(0.0, 1.0, 1.0), // _Y_Z
};

glm::mat4 getManipulatorTransfo(ManipulationMode mode, Selection select, const glm::mat4& model, glm::vec3 manipScale, glm::vec3 displacement)
{
    glm::mat4 result = model;
    switch (mode)
    {
    case ManipulationMode::Translation:
    {
        switch (select)
        {
        case Selection::X:
            //result = model * rotation_m * scale_m;
            result = glm::scale(glm::rotate(model, fpi2, glm::vec3(0.0, 1.0, 0.0)), manipScale);
            break;
        case Selection::Y:
            result = glm::scale(glm::rotate(model, fpi2, glm::vec3(-1.0, 0.0, 0.0)), manipScale);
            break;
        case Selection::Z:
            result = glm::scale(model, manipScale);
            break;
        case Selection::_X:
            result = glm::scale(glm::rotate(model, fpi2, glm::vec3(0.0, -1.0, 0.0)), manipScale);
            break;
        case Selection::_Y:
            result = glm::scale(glm::rotate(model, fpi2, glm::vec3(1.0, 0.0, 0.0)), manipScale);
            break;
        case Selection::_Z:
            result = glm::scale(glm::rotate(model, 2.0f * fpi2, glm::vec3(-1.0, -1.0, 0.0)), manipScale);
            break;
        default:
            break;
        }
        break;
    }
    case ManipulationMode::Rotation:
    {
        switch (select)
        {
        case Selection::X:
            result = glm::scale(glm::rotate(model, fpi2, glm::vec3(0.f, 0.f, 1.f)), manipScale);
            break;
        case Selection::Y:
            result = glm::scale(glm::rotate(model, fpi2, glm::vec3(0.f, 1.f, 0.f)), manipScale);
            break;
        case Selection::Z:
            result = glm::scale(glm::rotate(model, fpi2, glm::vec3(1.f, 0.f, 0.f)), manipScale);
            break;
        default:
            break;
        }
        break;
    }
    case ManipulationMode::Scale:
    case ManipulationMode::Extrusion:
    {
        switch (select)
        {
        case Selection::X:
            result = glm::scale(glm::rotate(glm::translate(model, glm::vec3(displacement.x, 0.f, 0.f)), fpi2, glm::vec3(0.f, 1.f, 0.f)), manipScale);
            break;
        case Selection::_X:
            result = glm::scale(glm::rotate(glm::translate(model, glm::vec3(-displacement.x, 0.f, 0.f)), fpi2, glm::vec3(0.f, -1.f, 0.f)), manipScale);
            break;
        case Selection::Y:
            result = glm::scale(glm::rotate(glm::translate(model, glm::vec3(0.f, displacement.y, 0.f)), fpi2, glm::vec3(-1.f, 0.f, 0.f)), manipScale);
            break;
        case Selection::_Y:
            result = glm::scale(glm::rotate(glm::translate(model, glm::vec3(0.f, -displacement.y, 0.f)), fpi2, glm::vec3(1.f, 0.f, 0.f)), manipScale);
            break;
        case Selection::Z:
            result = glm::scale(glm::translate(model, glm::vec3(0.f, 0.f, displacement.z)), manipScale);
            break;
        case Selection::_Z:
            result = glm::scale(glm::rotate(glm::translate(model, glm::vec3(0.f, 0.f, -displacement.z)), 2.f * fpi2, glm::vec3(-1.f, -1.f, 0.f)), manipScale);
            break;
        case Selection::XY:
            result = glm::scale(glm::rotate(glm::translate(model, glm::vec3(displacement.x,  displacement.y, 0.f)), fpi2, glm::vec3(-1.f, 1.f, 0.f)), manipScale);
            break;
        case Selection::X_Y:
            result = glm::scale(glm::rotate(glm::translate(model, glm::vec3(displacement.x, -displacement.y, 0.f)), fpi2, glm::vec3(1.f, 1.f, 0.f)), manipScale);
            break;
        case Selection::_XY:
            result = glm::scale(glm::rotate(glm::translate(model, glm::vec3(-displacement.x, displacement.y, 0.f)), fpi2, glm::vec3(-1.f, -1.f, 0.f)), manipScale);
            break;
        case Selection::_X_Y:
            result = glm::scale(glm::rotate(glm::translate(model, glm::vec3(-displacement.x, -displacement.y, 0.f)), fpi2, glm::vec3(1.f, -1.f, 0.f)), manipScale);
            break;
        case Selection::YZ:
            result = glm::scale(glm::rotate(glm::translate(model, glm::vec3(0.f, displacement.y, displacement.z)), -fpi2 * 0.5f, glm::vec3(1.f, 0.f, 0.f)), manipScale);
            break;
        case Selection::_YZ:
            result = glm::scale(glm::rotate(glm::translate(model, glm::vec3(0.f, -displacement.y, displacement.z)), fpi2 * 0.5f, glm::vec3(1.f, 0.f, 0.f)), manipScale);
            break;
        case Selection::Y_Z:
            result = glm::scale(glm::rotate(glm::translate(model, glm::vec3(0.f, displacement.y, -displacement.z)), -fpi2 * 1.5f, glm::vec3(1.f, 0.f, 0.f)), manipScale);
            break;
        case Selection::_Y_Z:
            result = glm::scale(glm::rotate(glm::translate(model, glm::vec3(0.f, -displacement.y, -displacement.z)), fpi2 * 1.5f, glm::vec3(1.f, 0.f, 0.f)), manipScale);
            break;
        case Selection::XZ:
            result = glm::scale(glm::rotate(glm::translate(model, glm::vec3(displacement.x, 0.f, displacement.z)), fpi2 * 0.5f, glm::vec3(0.f, 1.f, 0.f)), manipScale);
            break;
        case Selection::_XZ:
            result = glm::scale(glm::rotate(glm::translate(model, glm::vec3(-displacement.x, 0.f, displacement.z)), -fpi2 * 0.5f, glm::vec3(0.f, 1.f, 0.f)), manipScale);
            break;
        case Selection::X_Z:
            result = glm::scale(glm::rotate(glm::translate(model, glm::vec3(displacement.x, 0.f, -displacement.z)), fpi2 * 1.5f, glm::vec3(0.f, 1.f, 0.f)), manipScale);
            break;
        case Selection::_X_Z:
            result = glm::scale(glm::rotate(glm::translate(model, glm::vec3(-displacement.x, 0.f, -displacement.z)), -fpi2 * 1.5f, glm::vec3(0.f, 1.f, 0.f)), manipScale);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    }

    return result;
}

void ObjectNodeVisitor::draw_baked_manipulators(VkCommandBuffer cmdBuffer, ManipulatorRenderer& renderer, double manipSize)
{
    for (const ManipDrawData& manipDrawData : m_manipDrawData)
    {
        drawManipulator(cmdBuffer, renderer, manipDrawData.transfoMod, manipDrawData.node, manipSize);
    }
}

void ObjectNodeVisitor::drawManipulator(VkCommandBuffer cmdBuffer, ManipulatorRenderer& renderer, const TransformationModule& transfo, const SafePtr<ManipulatorNode>& manip, double manipSize)
{
    ReadPtr<ManipulatorNode> rManip = manip.cget();
    if (!rManip)
        return;

    glm::dmat4 customTransfo = transfo.getTransformation();
    glm::dvec3 objectScale = transfo.getScale();
    glm::dquat qrotation = transfo.getOrientation();
    glm::dvec3 translation = transfo.getCenter();
    glm::dvec3 displacement;

    double dist = (m_camera.getProjectionMode() == ProjectionMode::Perspective ? glm::length(m_viewMatrix * glm::dvec4(translation, 1.0)) : m_camera.getHeightAt1m());

    //Note (Aurélien) : c'est ici qu'il faut agir, pour modifier la taille et la distance des manipulateurs.
    // À noter : dist impacte le scale des manipulateur.
    //           scale la distance des manipulateurs par rapport au centre du parent.
    switch (rManip->getManipulationMode())
    {
    case ManipulationMode::Translation:
        dist /= 2.0;
        //displacement = glm::dvec3(dist); // Not used, we can put anything
        break;
    case ManipulationMode::Rotation:
        dist /= 8.0;
        //displacement = glm::dvec3(dist); // Not used, we can put anything
        break;
    case ManipulationMode::Scale:
    case ManipulationMode::Extrusion:
        dist /= 2.0;
        displacement = glm::abs(objectScale); // used to displace 
        displacement *= 1.1;
        break;
    }
    dist *= manipSize;

    if (!rManip->isLocalManipulation())
        // only translation
        customTransfo = glm::translate(glm::dmat4(1.0), translation);
    else
        // translation & rotation
        customTransfo = glm::translate(glm::dmat4(1.0), translation) * glm::mat4_cast(qrotation);

    std::shared_ptr<MeshBuffer> mesh(rManip->getActiveMeshBuffer());
    assert(mesh);
    if (!mesh)
        return;
    const Selection currentSelect(rManip->getCurrentSelection());
    const std::unordered_set<Selection> selectionDrawable(rManip->getAcceptableSelections());
    glm::dvec3 manipScale(dist);

    for (Selection select : selectionDrawable)
    {
        if (currentSelect != Selection::None && currentSelect != select)
            continue;
        glm::mat4 transfo = getManipulatorTransfo(rManip->getManipulationMode(), select, customTransfo, manipScale, displacement);
        glm::vec3 color = (currentSelect == select) ? SELECTION_COLOR : g_selectionColor[(size_t)select];
        renderer.draw(transfo, color, select, cmdBuffer, m_viewProjUniform, mesh);
    }
}

void ObjectNodeVisitor::draw_baked_meshes(VkCommandBuffer cmdBuffer, SimpleObjectRenderer& renderer, bool transparentPass)
{
    TlTopologyFlags topoFlags = TL_TOPOLOGY_NONE;
    topoFlags |= !transparentPass ? TL_TOPOLOGY_LINE : TL_TOPOLOGY_NONE;
    topoFlags |= ((m_displayParameters.m_alphaObject >= 1.f) != transparentPass) ? TL_TOPOLOGY_TRIANGLE : TL_TOPOLOGY_NONE;
    uint32_t subpass = transparentPass ? 1u : 0u;

    renderer.setAlphaValue(cmdBuffer, m_displayParameters.m_alphaObject < 1.f ? m_displayParameters.m_alphaObject / 2.f : 1.f);
    for (const auto& mesh : m_meshesDrawData)
    {
        if (!mesh.meshBuffer)
            continue;

       renderer.setObjectId(cmdBuffer, mesh.graphicId);
       renderer.setColor(cmdBuffer, mesh.color);
       renderer.setTransformationMatrix(cmdBuffer, mesh.transfo);
       renderer.setObjectFlags(cmdBuffer, mesh.isHovered, mesh.isSelected);

       renderer.drawMesh(cmdBuffer, m_viewProjUniform, mesh.meshBuffer, subpass, topoFlags);
    }
}

void ObjectNodeVisitor::blendTransparentPass(VkCommandBuffer cmdBuffer, SimpleObjectRenderer& renderer, VkDescriptorSet inputTransparentLayerDescSet)
{
    renderer.setAlphaBlendValue(cmdBuffer, m_displayParameters.m_alphaObject);
    renderer.blendTransparentImage(cmdBuffer, inputTransparentLayerDescSet);
}

//******************************************//
//             GRAPH TRAVERSAL              //
//******************************************//

void ObjectNodeVisitor::bakeGraph(SafePtr<AGraphNode> root)
{
    m_markerDrawData.clear();
    m_manipDrawData.clear();

    m_cs_marker_max_z = (m_projMatrix * glm::dvec4(0.0, 0.0, m_displayParameters.m_markerOptions.maximumDisplayDistance, 1.0)).z;

    TransformationModule origin_transfo;
    origin_transfo.addGlobalTranslation(m_camera.getLargeCoordinatesCorrection());

    nextGeoNode(root, origin_transfo);

    // Generate the Clipping Data for the shaders
    std::vector<UniformClippingData> UniformClippingData;
    //m_clippingAssembly.clearMatrix();
    generateUniformData(m_clippingAssembly, UniformClippingData);
    m_camera.uploadClippingUniform(UniformClippingData, m_uniformSwapIndex);
}

void ObjectNodeVisitor::nextGeoNode(const SafePtr<AGraphNode>& node, const TransformationModule& parent_transfo)
{
    TransformationModule global_transfo = parent_transfo;
    {
        ReadPtr<AGraphNode> rPtr = node.cget();
        if (!rPtr)
            return;
        // NOTE - To see fun effects, replace by "addTransfoLeft()"
        global_transfo.compose_right(*static_cast<const TransformationModule*>(&rPtr));
    }

    ElementType elemType = ElementType::None;
    bool visible = false;
    {
        ReadPtr<AGraphNode> rNode = node.cget();
        // Pratique de tester le isDisplayed() une et une seule fois pour tout les types
        if (!rNode)
            return;
        elemType = rNode->getType();
        visible = rNode->isVisible();
    }

    bakeGraphics(node, global_transfo);
    switch (elemType)
    {
    case ElementType::Box:
    case ElementType::Cylinder:
    case ElementType::Torus:
    case ElementType::Point:
    case ElementType::Tag:
    case ElementType::SimpleMeasure:
    case ElementType::PolylineMeasure:
    case ElementType::Sphere:
    {
        bakeClipping(node, global_transfo);
        break;
    }
    default:
        break;
    }

    std::unordered_set<SafePtr<AGraphNode>> children = AGraphNode::getGeometricChildren(node);
    for (const SafePtr<AGraphNode>& child : children) // PERF - const &
    {
        nextGeoNode(child, global_transfo);
    }
}

// TODO:
// [x] Passer la transfo globale en paramètre pour générer les MarkerDrawData
// [x] Afficher les mesures
// [x] Afficher les nuages de point
// [x] Afficher les meshes
// [x] Afficher les textes
// [x] Générer les ClippingAssembly
// [ ] Trouver comment écrire une interface dans AGraphNode pour que chaque spécialisation
//    spécifie elle-même ses objets graphique.
void ObjectNodeVisitor::bakeGraphics(const SafePtr<AGraphNode>& node, const TransformationModule& gTransfo)
{
    ElementType elemType = ElementType::None;
    AGraphNode::Type graphType = AGraphNode::Type::Default;
    {
        ReadPtr<AGraphNode> rNode = node.cget();
        // Pratique de tester le isDisplayed() une et une seule fois pour tout les types
        if (!rNode || !rNode->isDisplayed())
            return;
        elemType = rNode->getType();
        graphType = rNode->getGraphType();
    }

    glm::dmat4 transfoMat = gTransfo.getTransformation();

    // Marker //
    {
        bool show_marker = false;
        ReadPtr<AGraphNode> rObj = node.cget();
        if (!rObj)
            return;
        switch (elemType)
        {
        case ElementType::Scan:
            show_marker = (m_displayParameters.m_markerMask & SHOW_SCAN_MARKER) != 0;
            break;
        case ElementType::Tag:
            show_marker = (m_displayParameters.m_markerMask & SHOW_TAG_MARKER) != 0;
            break;
        case ElementType::Target:
            target_draw_data_.emplace_back(MarkerRenderer::getMarkerDrawData(transfoMat, *&rObj));
            show_marker = false;
            break;
        case ElementType::Point:
        case ElementType::BeamBendingMeasure:
        case ElementType::ColumnTiltMeasure:
        case ElementType::ViewPoint:
            show_marker = true;
            break;
        }

        if (show_marker)
            m_markerDrawData.emplace_back(MarkerRenderer::getMarkerDrawData(transfoMat, *&rObj));
    }

    switch (elemType)
    {
    case ElementType::Scan:
    {
        WritePtr<PointCloudNode> wScan = static_write_cast<PointCloudNode>(node);
        if (!wScan)
            break;

        // Point Cloud
        if (m_panoramicScan && (node != m_panoramicScan))
            break;
        Color32 color = wScan->getColor();
        if (m_displayParameters.m_mode == UiRenderMode::Clusters_Color)
        {
            SafePtr<AGraphNode> parent = wScan->getOwningObjectParent();
            ElementType type = ElementType::None;
            {
                ReadPtr<AGraphNode> rParent = parent.cget();
                if (rParent)
                    type = rParent->getType();
            }

            if (type == ElementType::Cluster)
            {
                ReadPtr<ClusterNode> rCluster = static_pointer_cast<ClusterNode>(parent).cget();
                if (rCluster->getDefaultTreeType() == TreeType::Scan)
                    color = rCluster->getColor();
            }
        }

        wScan->uploadUniform(transfoMat, m_uniformSwapIndex);
        if(wScan->getScanGuid().isValid())
            m_bakedPointCloud.push_back({ transfoMat, wScan->getScanGuid(), color, wScan->getUniform(m_uniformSwapIndex) , wScan->getClippable(), wScan->getPhase(), false });
        break;
    }
    case ElementType::PCO:
    {
        WritePtr<PointCloudNode> pco = static_write_cast<PointCloudNode>(node);
        if (!pco)
            break;
        Color32 color = pco->getColor();
        pco->uploadUniform(transfoMat, m_uniformSwapIndex);
        if (pco->getScanGuid().isValid())
            m_bakedPointCloud.push_back({ transfoMat, pco->getScanGuid(), color, pco->getUniform(m_uniformSwapIndex) , pco->getClippable(), pco->getPhase(), true });
        break;
    }
    // Mesh
    case ElementType::Box:
    {
        WritePtr<BoxNode> wBox = static_write_cast<BoxNode>(node);
        if (!wBox)
            break;
        m_meshesDrawData.emplace_back(wBox->getMeshDrawData(transfoMat));
        m_meshesDrawData.emplace_back(wBox->getGridMeshDrawData(transfoMat));

    }
    break;
    case ElementType::Cylinder:
    {
        ReadPtr<CylinderNode> rCyl = static_read_cast<CylinderNode>(node);
        if (!rCyl)
            break;
        m_meshesDrawData.emplace_back(rCyl->getMeshDrawData(transfoMat));
    }
    break;
    case ElementType::Sphere:
    {
        ReadPtr<SphereNode> rSph = static_read_cast<SphereNode>(node);
        if (!rSph)
            break;
        m_meshesDrawData.emplace_back(rSph->getMeshDrawData(transfoMat));
    }
    break;
    case ElementType::Torus:
    {
        ReadPtr<TorusNode> rTor = static_read_cast<TorusNode>(node);
        if (!rTor)
            break;
        m_meshesDrawData.emplace_back(rTor->getMeshDrawData(transfoMat));
    }
    break;
    case ElementType::MeshObject:
    {
        ReadPtr<MeshObjectNode> rMesh = static_read_cast<MeshObjectNode>(node);
        if (!rMesh)
            break;
        m_meshesDrawData.emplace_back(rMesh->getMeshDrawData(transfoMat));
    }
    break;
    case ElementType::PolylineMeasure:
    case ElementType::PointToPlaneMeasure:
    case ElementType::PointToPipeMeasure:
    case ElementType::PipeToPlaneMeasure:
    case ElementType::PipeToPipeMeasure:
    case ElementType::SimpleMeasure:
    {
        // Probably the same cast for all the measures
        ReadPtr<AMeasureNode> rMeasure = static_read_cast<AMeasureNode>(node);
        if (rMeasure)
        {
            rMeasure->getSegmentDrawData(transfoMat, m_segmentDrawData);
            // TODO - Ajouter les flags (selected, hovered) aux datas du segment
            //      - CliggingActive et ClippingMode ne sont pas utilisés
            //renderer.setObjectFlags(cmdBuffer, rMeasure->isHovered(), rMeasure->isSelected(), rMeasure->isClippingActive(), rMeasure->getClippingMode() == ClippingMode::showExterior);
        }
        break;
    }
    case ElementType::Cluster:
    {
        break;
    }
    break;
    default:
        break;
    }

    bool drawImguitext = m_displayParameters.m_displayAllMarkersTexts;
    drawImguitext &= !(elemType == ElementType::None);
    drawImguitext &= !((elemType == ElementType::Scan) && (m_displayParameters.m_markerMask & SHOW_SCAN_MARKER) == 0);
    switch (graphType)
    {
    case AGraphNode::Type::Default:
    {
        if (!drawImguitext)
            break;

        bool selected = false;
        bool hovered = false;
        uint32_t graphicId = INVALID_PICKING_ID;
        {
            ReadPtr<AGraphNode> rObj = node.cget();
            if (!rObj)
                break;
            selected = rObj->isSelected();
            hovered = rObj->isHovered();
            graphicId = rObj->getGraphicId();
        }
        float wx, wy;
        if (!bakeTextPosition(transfoMat, wx, wy, !selected && !hovered))
            break;

        std::string text;
        getObjectMarkerText(node, text);
        if (text == "")
            break;
        m_bakedTexts.push_back(BakedText{ text, wx, wy, selected, hovered, graphicId });
        break;
    }
    case AGraphNode::Type::Manipulator:
    {
        SafePtr<ManipulatorNode> manip = static_pointer_cast<ManipulatorNode>(node);
        // Il faut un WritePtr pour updateTransfo()
        WritePtr<ManipulatorNode> wManip = manip.get();
        if (!wManip)
            break;
        wManip->updateTransfo();
        m_manipDrawData.push_back({ gTransfo, manip });
        // TODO
        // m_texts.push_back(wManip->getText());
        break;
    }
    // Currently the camera is not linked in the graph
    case AGraphNode::Type::Camera:
    {
        SafePtr<CameraNode> cam = static_pointer_cast<CameraNode>(node);
        ReadPtr<CameraNode> rCam = cam.cget();
        if (rCam->isExamineActive() && rCam->m_showExamineTarget)
            target_draw_data_.emplace_back(MarkerRenderer::getExamineDrawData(rCam->getExamineTargetPosition()));
        break;
    }
    default:
        break;
    }

    // Draw the pictogramme for the examine target
    if (m_camera.isExamineActive() && m_camera.m_showExamineTarget)
    {
        target_draw_data_.emplace_back(MarkerRenderer::getExamineDrawData(m_camera.getExamineTargetPosition()));
    }
}

void ObjectNodeVisitor::bakeClipping(const SafePtr<AGraphNode>& node, const TransformationModule& gTransfo)
{
    ReadPtr<AClippingNode> rClipping = static_read_cast<AClippingNode>(node);
    if (!rClipping)
        return;

    if (rClipping->isClippingActive())
    {
        rClipping->pushClippingGeometries(m_clippingAssembly, gTransfo);
    }

    if (rClipping->isRampActive())
    {
        rClipping->pushRampGeometries(m_clippingAssembly.rampActives, gTransfo);
    }
}

void ObjectNodeVisitor::drawImGuiBegin(SafePtr<AGraphNode> startNode, VkCommandBuffer cmdBuffer)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_Impl_NewFrame((float)m_fbExtent.width, (float)m_fbExtent.height);
    ImGui::NewFrame();

    // Empty window with minimum style. Used to draw custom texts.
    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoBringToFrontOnFocus
        | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
        | ImGuiWindowFlags_NoBackground;
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2((float)m_fbExtent.width, (float)m_fbExtent.height), ImGuiCond_Always);
    //ImGui::SetNextWindowContentSize(ImVec2((float)m_fbExtent.width, (float)m_fbExtent.height));
    //ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.6, 0.5, 0.0, 1.0));
    //ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0, 0.0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);

    ImGui::Begin("Objects labels", NULL, windowFlags);

    // Draw des textes déjà obtenus pendant le bake du graph
    drawMeasureTexts();
    drawObjectTexts();
    drawManipulatorText();

    ImGui::End();
    ImGui::PopStyleVar(2);
}

void ObjectNodeVisitor::drawImGuiEnd(VkCommandBuffer cmdBuffer)
{
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuffer);
}

void ObjectNodeVisitor::draw_baked_pointClouds(VkCommandBuffer cmdBuffer, Renderer& renderer)
{
    m_totalPointsDrawn = 0;
    m_totalNonClippedCells = 0;
    m_totalClippedCells = 0;

    renderer.setViewportAndScissor(0, 0, m_fbExtent.width, m_fbExtent.height, cmdBuffer);
    renderer.setConstantPointSize(m_displayParameters.m_pointSize, cmdBuffer);
    renderer.setConstantContrastBrightness((float)m_displayParameters.m_contrast, (float)m_displayParameters.m_brightness, cmdBuffer);
    renderer.setConstantSaturationLuminance((float)m_displayParameters.m_saturation, (float)m_displayParameters.m_luminance, cmdBuffer);
    renderer.setConstantBlending((float)m_displayParameters.m_hue, cmdBuffer);

    if (m_displayParameters.m_mode == UiRenderMode::Distance_Ramp ||
        m_displayParameters.m_mode == UiRenderMode::Flat_Distance_Ramp)
        renderer.setConstantRampDistance(m_displayParameters.m_distRampMin, m_displayParameters.m_distRampMax, m_displayParameters.m_distRampSteps, cmdBuffer);

    // *** Traverse the scan using the frustum matrix as a clipping box ***
    // The projection info are common for all the scans
    // The Model matrix will be edited by each drawScan()
    TlProjectionInfo projInfoNode{ glm::dmat4(), m_viewProjMatrix, m_fbExtent.width, m_fbExtent.height, m_displayParameters.m_pointSize, m_decimationRatio, m_displayParameters.m_deltaFilling };

    m_drawHasMissingScanPart = false;
    ClippingAssembly emptyAssembly;

    for (const PointCloudDrawData& bakedPC : m_bakedPointCloud)
    {
        clipAndDrawPointCloud(cmdBuffer, renderer, bakedPC, projInfoNode, bakedPC.clippable ? m_clippingAssembly : emptyAssembly);
    }
}

void ObjectNodeVisitor::clipAndDrawPointCloud(VkCommandBuffer _cmdBuffer, Renderer& renderer, const PointCloudDrawData& bakedPC, TlProjectionInfo& projInfo, const ClippingAssembly& _clippingAssembly)
{
    TlScanOverseer& overseer = TlScanOverseer::getInstance();
    TlScanDrawInfo drawInfo = TlScanDrawInfo();
    bool needStreaming(false); // catch the result from the scan view
    projInfo.modelMat = bakedPC.transfo;

    drawInfo.modelUni = bakedPC.uniform;
    drawInfo.color = bakedPC.color.toVector();

    const ClippingAssembly* assemblyToUse = &_clippingAssembly;
    ClippingAssembly resolvedAssembly;
    if (_clippingAssembly.hasPhaseClipping())
    {
        resolvedAssembly = _clippingAssembly.resolveByPhase(bakedPC.phase);
        assemblyToUse = &resolvedAssembly;

        std::vector<UniformClippingData> uniformData;
        generateUniformData(resolvedAssembly, uniformData);
        m_camera.uploadClippingUniform(uniformData, m_uniformSwapIndex);
    }

    if (((m_displayParameters.m_mode == UiRenderMode::Flat) ||
        (m_displayParameters.m_mode == UiRenderMode::Grey_Colored)) &&
        !bakedPC.isObject)
        renderer.setConstantPtColor(m_displayParameters.m_flatColor, _cmdBuffer);
    else
        renderer.setConstantPtColor(drawInfo.color, _cmdBuffer);

    // Clip the octree and get the resulting parts to show on screen
    if (overseer.getScanView(bakedPC.scanGuid, projInfo, *assemblyToUse, drawInfo, needStreaming) == false)
        return;

    m_drawHasMissingScanPart |= needStreaming;

    // Draw points that do not need a clipping test
    if (!drawInfo.cellDrawInfo.empty())
    {
        renderer.drawPoints(drawInfo, m_viewProjUniform, correspUiRenderMode.at(m_displayParameters.m_mode), _cmdBuffer, m_displayParameters.m_blendMode, m_pointRenderFormat);
    }

    // Draw points with a clipping restriction
    if (!drawInfo.cellDrawInfoCB.empty())
    {
        renderer.drawPointsClipping(drawInfo, m_viewProjUniform, m_clipUniform, correspUiRenderMode.at(m_displayParameters.m_mode), _cmdBuffer, m_displayParameters.m_blendMode, m_pointRenderFormat);
    }

    // Stats
    for (const TlCellDrawInfo& cellDraw : drawInfo.cellDrawInfo)
    {
        m_totalPointsDrawn += cellDraw.vertexCount;
    }
    for (const TlCellDrawInfo_multiCB& cellDrawCB : drawInfo.cellDrawInfoCB)
    {
        m_totalPointsDrawn += cellDrawCB.vertexCount;
    }
    m_totalNonClippedCells += drawInfo.cellDrawInfo.size();
    m_totalClippedCells += drawInfo.cellDrawInfoCB.size();
}

void ObjectNodeVisitor::draw_baked_markers(VkCommandBuffer cmdBuf, MarkerRenderer& renderer, VkDescriptorSet inputAttachDescSet, SimpleBuffer& markerBuffer)
{
    std::vector<MarkerDrawData> markerSortedAndFiltered;
    sortMarkersByDepth(m_markerDrawData, markerSortedAndFiltered, m_displayParameters.m_markerOptions.maximumDisplayDistance);

    if (markerSortedAndFiltered.empty() && target_draw_data_.empty())
        return;

    float nearScale = m_displayParameters.m_markerOptions.nearSize * m_screenHeightAt1m * m_guiScale / (float)m_fbExtent.height;
    float farScale = m_displayParameters.m_markerOptions.farSize * m_screenHeightAt1m * m_guiScale / (float)m_fbExtent.height;
    // Force the farScale for the orthographic view
    nearScale = (m_projMode == ProjectionMode::Orthographic) ? farScale : nearScale;

    // load draw data in memory (the buffer should be reset elsewhere)
    assert(markerBuffer.alloc == nullptr);
    VulkanManager& vkm = VulkanManager::getInstance();
    VkDeviceSize data_size = markerSortedAndFiltered.size() * sizeof(MarkerDrawData);
    VkDeviceSize target_data_size = target_draw_data_.size() * sizeof(MarkerDrawData);
    VkDeviceSize total_data_size = data_size + target_data_size;
    vkm.allocSimpleBuffer(total_data_size, markerBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize offset = 0;

    vkm.loadInSimpleBuffer(markerBuffer, data_size, markerSortedAndFiltered.data(), offset, 4);
    offset += data_size;
    vkm.loadInSimpleBuffer(markerBuffer, target_data_size, target_draw_data_.data(), offset, 4);
    offset += target_data_size;

    // Draw markers
    renderer.setScaleConstants(cmdBuf, nearScale, farScale, m_displayParameters.m_markerOptions.nearLimit, m_displayParameters.m_markerOptions.farLimit);
    // Set border color constants (Neutral, Hover, Select)
    glm::vec4 neutralColor = m_displayParameters.m_markerOptions.improveVisibility ? glm::vec4(0.19f, 0.19f, 0.19f, 0.75f) : glm::vec4(0.f, 0.f, 0.f, 0.f);
    renderer.setBordersColorConstant(cmdBuf, neutralColor, glm::vec4(0.59f, 0.05f, 0.87f, 1.f), glm::vec4(0.95f, 0.84f, 0.1f, 1.f));
    renderer.setDepthRenderConstant(cmdBuf, m_displayParameters.m_markerOptions.improveVisibility);
    renderer.setViewportSizeConstants(cmdBuf, m_fbExtent);
    renderer.drawMarkerData(cmdBuf, markerBuffer, 0u, (uint32_t)markerSortedAndFiltered.size(), m_viewProjUniform, inputAttachDescSet);

    // Draw targets
    float scale = 15.f * m_screenHeightAt1m * m_guiScale / (float)m_fbExtent.height;
    // NOTE - The near and far limits have no effect here because we set the same scale for near and far.
    renderer.setScaleConstants(cmdBuf, scale, scale, m_displayParameters.m_markerOptions.nearLimit, m_displayParameters.m_markerOptions.farLimit);
    renderer.setBordersColorConstant(cmdBuf, glm::vec4(0.f, 0.f, 0.f, 0.f), glm::vec4(0.59f, 0.05f, 0.87f, 1.f), glm::vec4(0.95f, 0.84f, 0.1f, 1.f));
    renderer.setDepthRenderConstant(cmdBuf, false);
    renderer.drawMarkerData(cmdBuf, markerBuffer, (uint32_t)markerSortedAndFiltered.size(), (uint32_t)target_draw_data_.size(), m_viewProjUniform, inputAttachDescSet);
}

// FONCTIONNEMENT(robin)
//  - On utilise une structure 'MeasureBufferData' pour décrire les segments de mesure à afficher.
//  - Chaque segment contient les infos nécessaires aux shaders pour son affichage (position, couleur, iindex)
//  - Les segments sont générer par les objets qui souhaitent afficher une mesure.
//  - Ils sont stokés dans un buffer commun alloué et géré le 'MesureStorage'.
//  - Une fois stoké, un segment de mesure est référencé par un 'MesureStorageId'.
//  - Ce 'MesureStorageId' est utilisé pour draw la donnée stockée sur le buffer via les shaders.
//  - 
//
// REWORK(robin)
//  - 
void ObjectNodeVisitor::draw_baked_measures(VkCommandBuffer cmdBuffer, MeasureRenderer& renderer, SimpleBuffer& segmentBuffer)
{
    if (m_segmentDrawData.empty())
        return;

    // load draw data in memory (the buffer should be reset elsewhere)
    assert(segmentBuffer.alloc == nullptr);
    VkDeviceSize dataSize = m_segmentDrawData.size() * sizeof(SegmentDrawData);
    VulkanManager::getInstance().allocSimpleBuffer(dataSize, segmentBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize offset = 0;

    VulkanManager::getInstance().loadInSimpleBuffer(segmentBuffer, dataSize, m_segmentDrawData.data(), offset, 4);

    renderer.setStripCount(cmdBuffer, 10.f);
    renderer.setScreenParameters(cmdBuffer, m_screenHeightAt1m / (float)m_fbExtent.height, m_camera.getNear(), m_camera.getFar());
    renderer.setPointSize(cmdBuffer, 11.f * m_guiScale);
    // Normal for the light ray (in the camera space)
    renderer.setLightRay(cmdBuffer, glm::vec3(-0.4f, -0.5f, 0.812f));
    renderer.setMeasureShowMask(cmdBuffer, m_displayParameters.m_measureMask);
    renderer.drawMeasures(cmdBuffer, m_viewProjUniform, segmentBuffer.buffer, (uint32_t)m_segmentDrawData.size());
}

void ObjectNodeVisitor::sortMarkersByDepth(const std::vector<MarkerDrawData>& markers, std::vector<MarkerDrawData>& sortedIndexes, double maxDistance)
{
    std::multimap<float, uint32_t> indexesMap;

    // Use the multimap to sort the markers by their Z-view coord
    uint32_t max_i = markers.size() > _UI32_MAX ? _UI32_MAX : (uint32_t)markers.size();
    for (uint32_t i = 0; i < max_i; ++i)
    {
        glm::vec4 wsPos(markers[i].position[0], markers[i].position[1], markers[i].position[2], 1.f);
        glm::vec4 vsPos = m_viewMatrix * wsPos;
        if (vsPos.z > maxDistance)
            continue;
        indexesMap.insert({ vsPos.z, i });
    }

    // Store the instanceId in the vector by decreasing Z order
    for (auto it = indexesMap.rbegin(); it != indexesMap.rend(); ++it)
    {
        sortedIndexes.push_back(markers[it->second]);
    }
}

void ObjectNodeVisitor::getDrawCount(uint64_t& pointsDrawn, uint64_t& cellsDrawn)
{
    pointsDrawn = m_totalPointsDrawn;
    cellsDrawn = m_totalNonClippedCells + m_totalClippedCells;
}

void ObjectNodeVisitor::drawGizmo( VkCommandBuffer cmdBuffer, const glm::dvec3& gizmoParameters, std::shared_ptr<MeshBuffer> mesh, ManipulatorRenderer& renderer)
{
    if (!m_displayParameters.m_displayGizmo || !mesh)
        return;

    auto frustum(m_camera.getProjectionFrustum());
    TransformationModule scaling;
    double ratio = 1.0 / m_camera.getRatioW_H();
    scaling.setScale({1.0, ratio, 1.0});

    glm::dmat4 s_w = m_camera.getModelMatrix() * glm::inverse(m_camera.getProjMatrix()) * scaling.getInverseTransformation();

    TransformationModule positionning;
    if(frustum.n > 0)
        positionning.setPosition({ gizmoParameters.x, gizmoParameters.y * ratio, frustum.n + 0.25 });
    else
        positionning.setPosition({ gizmoParameters.x, gizmoParameters.y * ratio, 0.25 });
    positionning.setRotation(glm::inverse(m_camera.getRotation()));
    positionning.setScale(glm::vec3(gizmoParameters.z));

    glm::dmat4 transfo(s_w * positionning.getTransformation());
    //X
    renderer.draw(glm::rotate(transfo, pi2, glm::dvec3(0.0f, 1.0f, 0.0f)), glm::vec3(1.0f, 0.0f, 0.0f), Selection::None, cmdBuffer, m_viewProjUniform, mesh);
    //Y
    renderer.draw(glm::rotate(transfo, pi2, glm::dvec3(-1.0f, 0.0f, 0.0f)), glm::vec3(0.0f, 1.0f, 0.0f), Selection::None, cmdBuffer, m_viewProjUniform, mesh);
    //Z
    renderer.draw(glm::rotate(transfo, pi2, glm::dvec3(0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, 0.0f, 1.0f), Selection::None, cmdBuffer, m_viewProjUniform, mesh);
}
