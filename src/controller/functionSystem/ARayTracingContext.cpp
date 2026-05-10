#include "controller/functionSystem/ARayTracingContext.h"
#include "models/3d/Measures.h"
#include "models/graph/AMeasureNode.h"
#include "models/3d/MeshBuffer.h"

#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/texts/RayTracingTexts.hpp"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/IControlListener.h"
#include "controller/controls/ControlMetaControl.h"
#include "models/graph/GraphManager.h"
#include "models/graph/CameraNode.h"
#include "controller/messages/FullClickMessage.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "pointCloudEngine/RayTracingDisplayFilter.h"
#include "pointCloudEngine/PCE_core.h"
#include "vulkan/VulkanManager.h"

#include "gui/texts/ContextTexts.hpp"

#include "utils/Logger.h"
#include "utils/math/glm_extended.h"


namespace
{
RayTracingDisplayFilterSettings buildRayTracingDisplayFilterSettings(const ClickInfo& clickInfo)
{
    RayTracingDisplayFilterSettings settings;
    ReadPtr<CameraNode> rCamera = clickInfo.viewport.cget();
    if (!rCamera)
        return settings;

    const DisplayParameters& display = rCamera->getDisplayParameters();
    settings.enabled = true;
    settings.renderMode = display.m_mode;
    settings.colorimetricFilter = display.m_colorimetricFilter;
    settings.polygonalSelector = display.m_polygonalSelector;
    return settings;
}
}

ARayTracingContext::ARayTracingContext(const ContextId& id)
	: AContext(id)
	, m_panoramic(xg::Guid())
	, m_lastCameraPos(0.0, 0.0, 0.0)
	, m_repeatInput(false)
	, m_isDisplayingClickTarget(true)
{
}

ARayTracingContext::~ARayTracingContext()
{}

ContextState ARayTracingContext::start(Controller& controller)
{
    return waitForNextPoint(controller);
}

ContextState ARayTracingContext::feedMessage(IMessage* message, Controller& controller)
{
    if (message->getType() == IMessage::MessageType::FULL_CLICK)
    {
        controller.updateInfo(new GuiDataChangeCursor(Qt::CursorShape::BusyCursor));

        FullClickMessage* clickMsg = static_cast<FullClickMessage*>(message);
        ClickInfo clickInfo = clickMsg->m_clickInfo;

        m_panoramic = clickInfo.panoramic;
        m_lastCameraPos = clickInfo.rayOrigin;

        controller.getGraphManager().getMeshInfoForClick(clickInfo);

        if (clickInfo.mesh != nullptr &&
            clickInfo.hover != m_meshSelected)
        {
            // If there is no mesh we wait for one (points already ray traced are waiting)
            if (downloadTriangleList(clickInfo.mesh, clickInfo.meshTransfo))
                m_meshSelected = clickInfo.hover;
            else
            {
                m_meshSelected = SafePtr<AGraphNode>();
                Logger::log(LoggerMode::FunctionLog) << "Error: failed to load mesh in context memory." << Logger::endl;
            }
        }

        std::lock_guard<std::mutex> lock_2(m_clickMutex);
        m_waitingClicks.push_back(clickInfo);
		
        m_state = ContextState::ready_for_using;
    }
    return (m_state);
}

bool ARayTracingContext::getNextPosition(Controller& controller)
{
    if (m_usages.empty())
        return false;
    m_state = ContextState::running;

    glm::dvec3 bestPoint = glm::dvec3(NAN);
    glm::dvec3 bestNormal = glm::dvec3(NAN);
    std::string scanName = TEXT_CONTEXT_NO_SCAN_FOUND.toStdString();

    SafePtr<AGraphNode>  objectPtr;
    ClickInfo clickInfo;
    ClickUsage clickUsage;
    {
        std::lock_guard<std::mutex> lock(m_clickMutex);
        // We reset the results if all the click have already been provided.
        // This means the context is reused for another process.
        if (m_clickResults.size() >= m_usages.size() && !m_repeatInput)
        {
            m_clickResults.clear();
        }
        clickUsage = m_usages[m_clickResults.size() % m_usages.size()];

        if (m_waitingClicks.size() > 0)
        {
            clickInfo = m_waitingClicks.back();
            m_waitingClicks.pop_back();
        }
        else
        {
            // Normal, nothing to do
            m_state = ContextState::waiting_for_input;
            return false;
        }
    }
    m_lastClickInfo = clickInfo;

    ElementType objectType;
    bool skipRayTracing = false;
    glm::dvec3 objectCenter;
    auto is3DObjectForExamine = [](ElementType type)
    {
        switch (type)
        {
        case ElementType::Box:
        case ElementType::Sphere:
        case ElementType::Cylinder:
        case ElementType::Torus:
        case ElementType::Piping:
        case ElementType::MeshObject:
        case ElementType::SimpleMeasure:
        case ElementType::PolylineMeasure:
        case ElementType::BeamBendingMeasure:
        case ElementType::ColumnTiltMeasure:
        case ElementType::PointToPlaneMeasure:
        case ElementType::PointToPipeMeasure:
        case ElementType::PipeToPlaneMeasure:
        case ElementType::PipeToPipeMeasure:
            return true;
        default:
            return false;
        }
    };
    {
        ReadPtr<AGraphNode> object = clickInfo.hover.cget();
        if (object)
        {
            objectType = object->getType();
            objectCenter = object->getCenter();
            if (clickUsage.objectTypesAccepted.find(objectType) != clickUsage.objectTypesAccepted.end())
                objectPtr = clickInfo.hover;
            else if (object->getType() == ElementType::Scan)
                skipRayTracing = true;
            else
                FUNCLOG << "[ARayTracingContext] Object type not accepted" << LOGENDL;
        }
    }

        bool useObjectCenter = clickUsage.getObjectCenter;
        if (objectPtr && is3DObjectForExamine(objectType) && !clickInfo.forceObjectCenter)
            useObjectCenter = false;

        if (objectPtr && useObjectCenter) {

                std::vector<Measure> measures;
                switch (objectType) {
			case ElementType::PipeToPipeMeasure:
			case ElementType::PipeToPlaneMeasure:
			case ElementType::PointToPipeMeasure:
			case ElementType::PointToPlaneMeasure:
			case ElementType::SimpleMeasure:
			case ElementType::PolylineMeasure:
            {
                ReadPtr<AMeasureNode> readMeasure = static_pointer_cast<AMeasureNode>(objectPtr).cget();
                if (!readMeasure)
                    break;
                measures = readMeasure->getMeasures();
            }
            break;
            case ElementType::MeshObject:
                // rayTraceMesh()
                // getPlaneNormal()
			default :
				measures = {};
				break;
		}

		if(measures.empty())
			bestPoint = objectCenter;
		else
		{
			float bestAngle = glm_extended::angleBetweenV3(measures[0].origin - clickInfo.rayOrigin, clickInfo.ray);
			bestPoint = measures[0].origin;
			for (Measure measure : measures) {
				float angleToOrigin = glm_extended::angleBetweenV3(measure.origin - clickInfo.rayOrigin, clickInfo.ray);
				if (angleToOrigin < bestAngle) {
					bestAngle = angleToOrigin;
					bestPoint = measure.origin;
				}
				float angleToFinal = glm_extended::angleBetweenV3(measure.final - clickInfo.rayOrigin, clickInfo.ray);
				if (angleToFinal < bestAngle) {
					bestAngle = angleToFinal;
					bestPoint = measure.final;
				}
			}
		}
	}
    else if (!skipRayTracing)
    {
        if (clickInfo.mesh != nullptr)
            rayTraceMesh(clickInfo, bestPoint, bestNormal);
        else
            bestPoint = rayTracePointClouds(controller, clickInfo, scanName);
    }

    if (std::isnan(bestPoint.x) == false)
    {
        ClickResult result;
        result.position = bestPoint;
        result.object = objectPtr;
        result.normal = bestNormal;
        result.scanName = scanName;
        m_clickResults.push_back(result);

		if (m_isDisplayingClickTarget)
			controller.updateInfo(new GuiDataRenderTargetClick(bestPoint));

        return true;
    }

    return false;
}

ContextState ARayTracingContext::abort(Controller& controller)
{
    controller.updateInfo(new GuiDataChangeCursor());
    return AContext::abort(controller);
}

ContextState ARayTracingContext::validate(Controller& controller)
{
    controller.updateInfo(new GuiDataChangeCursor());
    controller.getControlListener()->notifyUIControl(new control::meta::ControlStopMetaControl());
    return AContext::validate(controller);
}

ContextState ARayTracingContext::waitForNextPoint(Controller& controller, QString errorMessage)
{
    QString nextClickMessage;
    if (!errorMessage.isEmpty())
        nextClickMessage += errorMessage + " ";
       
    if ((m_clickResults.size() < m_usages.size()))
    {
        ClickUsage clickUsage = m_usages[m_clickResults.size() % m_usages.size()];
        nextClickMessage += clickUsage.uiMessage;
        controller.updateInfo(new GuiDataChangeCursor(Qt::CursorShape::CrossCursor));
        controller.updateInfo(new GuiDataTmpMessage(nextClickMessage, 0));
        return (m_state = ContextState::waiting_for_input);
    }

    return (m_state = (m_state == ContextState::running) ? ARayTracingContext::validate(controller) : ContextState::ready_for_using);
}

bool ARayTracingContext::pointMissing()
{
    return (m_clickResults.size() < m_usages.size());
}

void ARayTracingContext::nextText(Controller& controller)
{
    controller.updateInfo(new GuiDataTmpMessage(m_usages[m_clickResults.size() % m_usages.size()].uiMessage, 0));
}

bool ARayTracingContext::downloadTriangleList(const std::shared_ptr<MeshBuffer> meshBuffer, glm::dmat4 meshTransfo) //Vu que pour l'export d'Obj/Fbx, j'utilise la même fonction, pourquoi pas l'utiliser via le MeshManager ? A cause du mutex avec le raytracing de mesh ?
{
    if (meshBuffer == nullptr)
        return false;

    std::lock_guard<std::mutex> lock(m_meshMutex);
    m_triangles.clear();

    //controller.updateInfo(new GuiDataChangeCursor(Qt::CursorShape::BusyCursor));
    size_t bufferSize = meshBuffer->getSimpleBuffer().size;
    void* bufferData = new char[bufferSize];
    memset(bufferData, 0, bufferSize);

    std::chrono::steady_clock::time_point tp[3];

    // --- Download the raw data (vertices & indexes) from the mesh buffer ---
    tp[0] = std::chrono::steady_clock::now();
    VulkanManager::getInstance().downloadSimpleBuffer(meshBuffer->getSimpleBuffer(), bufferData, bufferSize);

    // Unpack the triangles (Can take a lot more memory)
    tp[1] = std::chrono::steady_clock::now();
    getTriangleList(bufferData, meshBuffer, meshTransfo, m_triangles);
    delete[] bufferData;

    tp[2] = std::chrono::steady_clock::now();
    float dta = std::chrono::duration<float, std::milli>(tp[1] - tp[0]).count();
    float dtb = std::chrono::duration<float, std::milli>(tp[2] - tp[1]).count();

    Logger::log(LoggerMode::FunctionLog) << "CPU Mesh conversion : " << dta << "[ms] read, " << dtb << "[ms] decode." << Logger::endl;
    return true;
}

bool ARayTracingContext::getTriangleList(void* bufferData, const std::shared_ptr<MeshBuffer> meshBuffer, glm::dmat4 meshTransfo, std::vector<Triangle>& triangles)
{
    // Estimate the triangles count
    uint32_t triangleCount = 0;
    for (auto topoDraw : meshBuffer->getDrawList())
    {
        for (const StandardDraw& stdDraw : topoDraw.second)
        {
            if (topoDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                triangleCount += stdDraw.vertexCount / 3;
            if (topoDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP ||
                topoDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
                triangleCount += stdDraw.vertexCount - 2;
        }
    }

    for (auto topoIDraw : meshBuffer->getIndexedDrawList())
    {
        for (const IndexedDraw& idd : topoIDraw.second)
        {
            if (topoIDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                triangleCount += idd.indexCount / 3;
            if (topoIDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP ||
                topoIDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
                triangleCount += idd.indexCount - 2;
        }
    }
    triangles.reserve(triangleCount);

    glm::vec3* vertices = static_cast<glm::vec3*>(bufferData);
    size_t vCount = meshBuffer->getVertexCount();
    std::vector<glm::vec3> v_g; // vertices global transfo
    v_g.reserve(vCount);
    for (size_t i = 0; i < vCount; ++i)
    {
        v_g.emplace_back(meshTransfo * glm::vec4(vertices[i], 1.f));
    }

    for (auto topoDraw : meshBuffer->getDrawList())
    {
        for (const StandardDraw& stdDraw : topoDraw.second)
        {
            if (topoDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            {
                for (uint32_t i = stdDraw.firstVertex; i < stdDraw.firstVertex + stdDraw.vertexCount - 2; i += 3)
                {
                    triangles.emplace_back(v_g[i], v_g[i + 1], v_g[i + 2]);
                }
            }
            else if (topoDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP)
            {
                for (uint32_t i = stdDraw.firstVertex; i < stdDraw.firstVertex + stdDraw.vertexCount - 2; ++i)
                {
                    triangles.emplace_back(v_g[i], v_g[i + 1], v_g[i + 2]);
                }
            }
            else if (topoDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
            {
                for (uint32_t i = stdDraw.firstVertex; i < stdDraw.firstVertex + stdDraw.vertexCount - 2; ++i)
                {
                    triangles.emplace_back(v_g[stdDraw.firstVertex], v_g[i + 1], v_g[i + 2]);
                }
            }
        }
    }

    uint32_t* indexes = static_cast<uint32_t*>((void*)((char*)bufferData + meshBuffer->getIndexOffset()));

    for (auto topoIDraw : meshBuffer->getIndexedDrawList())
    {
        for (const IndexedDraw& idd : topoIDraw.second)
        {
            if (topoIDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            {
                for (uint32_t i = idd.firstIndex; i < idd.firstIndex + idd.indexCount - 2; i += 3)
                {
                    triangles.emplace_back(v_g[idd.vertexOffset + indexes[i]],
                        v_g[idd.vertexOffset + indexes[i + 1]],
                        v_g[idd.vertexOffset + indexes[i + 2]]);
                }
            }
            else if (topoIDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP)
            {
                for (uint32_t i = idd.firstIndex; i < idd.firstIndex + idd.indexCount - 2; ++i)
                {
                    triangles.emplace_back(v_g[idd.vertexOffset + indexes[i]],
                        v_g[idd.vertexOffset + indexes[i + 1]],
                        v_g[idd.vertexOffset + indexes[i + 2]]);
                }
            }
            else if (topoIDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
            {
                for (uint32_t i = idd.firstIndex; i < idd.firstIndex + idd.indexCount - 2; ++i)
                {
                    triangles.emplace_back(v_g[idd.vertexOffset + indexes[0]],
                        v_g[idd.vertexOffset + indexes[i + 1]],
                        v_g[idd.vertexOffset + indexes[i + 2]]);
                }
            }
        }
    }

    return true;
}

glm::vec3 ARayTracingContext::minimalDistanceOnMesh(glm::vec3 M)
{
    // Distance minimum entre tout les points projetés
    float minDist = std::numeric_limits<float>::max();
    glm::vec3 result = glm::vec3(0.f, 0.f, 0.f);
    for (const Triangle& t : m_triangles)
    {
        // Calcule la distance au triangle
        glm::vec3 P = projectionOnTriangle(M, t);

        // Stocke le point
        float dist = length(M - P);
        if (dist < minDist)
        {
            minDist = dist;
            result = P;
        }
    }
    return result;
}

glm::vec3 ARayTracingContext::projectionOnTriangle(const glm::vec3& M, const Triangle t)
{
    glm::vec3 AB = t.B - t.A;
    glm::vec3 BC = t.C - t.B;
    glm::vec3 CA = t.A - t.C;
    glm::vec3 AM = M - t.A;
    glm::vec3 BM = M - t.B;
    glm::vec3 CM = M - t.C;

    // Calcul de la normale du triangle ABC dans le sens direct
    glm::vec3 n = cross(AB, BC);

    // On distingue trois cas de figure :
    //  (a) P est à l’intèrieur du triangle
    //  (b) P est proche d’un côté
    //  (c) P est proche d’un sommet

    // Valeurs utiles pour savoir si on est à l’extèrieur d’un côté
    float alpha = dot(cross(AB, AM), n);
    float beta = dot(cross(BC, BM), n);
    float omega = dot(cross(CA, CM), n);

    // Valeurs utiles pour la projection sur un côté
    float t_ab = dot(AB, AM) / dot(AB, AB);
    float t_bc = dot(BC, BM) / dot(BC, BC);
    float t_ca = dot(CA, CM) / dot(CA, CA);

    if (t_ab > 0.0)
    {
        if (t_ab < 1.0 && alpha < 0.0)
        {
            return (t.A + AB * t_ab);
        }
        else if (t_bc < 0.0)
        {
            return t.B;
        }
    }
    if (t_bc > 0.0)
    {
        if (t_bc < 1.0 && beta < 0.0)
        {
            return (t.B + BC * t_bc);
        }
        else if (t_ca < 0.0)
        {
            return t.C;
        }
    }
    if (t_ca > 0.0)
    {
        if (t_ca < 1.0 && omega < 0.0)
        {
            return (t.C + CA * t_ca);
        }
        else if (t_ab < 0.0)
        {
            return t.A;
        }
    }
    // On est à l’intèrieur
    return (M - n * dot(AM, n) / dot(n, n));
}

glm::vec3 ARayTracingContext::projectionAlongAxis(const glm::vec3& M, const Triangle& t, const glm::vec3& N)
{
    glm::vec3 AM = M - t.A;
    glm::vec3 BM = M - t.B;
    glm::vec3 CM = M - t.C;

    float n_am = glm::dot(AM, N);
    float n_bm = glm::dot(BM, N);
    float n_cm = glm::dot(CM, N);

    float d_ab = (n_am < 0.f && n_bm > 0.f) || (n_am > 0.f && n_bm < 0.f) ? 0.f :
        std::min(abs(n_am), abs(n_bm));
    float d_bc = (n_bm < 0.f && n_cm > 0.f) || (n_bm > 0.f && n_cm < 0.f) ? 0.f :
        std::min(abs(n_bm), abs(n_cm));
    float d_ca = (n_cm < 0.f && n_am > 0.f) || (n_cm > 0.f && n_am < 0.f) ? 0.f :
        std::min(abs(n_cm), abs(n_am));

    return t.A;
}

bool ARayTracingContext::rayTriangleIntersection(const glm::vec3& M, const glm::vec3& D, const Triangle& t, glm::vec3& hitPt)
{
    glm::vec3 N = cross(t.B - t.A, t.C - t.B);
    float d = -dot(t.A, N);

    if (abs(dot(N, D)) < 1.e-5f)
        return false;
    float s = -(dot(N, M) + d) / dot(N, D);

    if (s < 0.f)
        return false;
    glm::vec3 P = M + s * D;

    if (dot(N, cross(t.B - t.A, P - t.A)) < 0.f)
        return false;

    if (dot(N, cross(t.C - t.B, P - t.B)) < 0.f)
        return false;

    if (dot(N, cross(t.A - t.C, P - t.C)) < 0.f)
        return false;

    hitPt = P;
    return true;

}

glm::dvec3 ARayTracingContext::rayTracePointClouds(Controller& controller, ClickInfo& clickInfo, std::string& scanName)
{
    glm::dvec3 result = glm::dvec3(NAN);
    double pointSize = controller.getContext().getRenderPointSize();
    pointSize += 2.0;

    ClippingAssembly clipAssembly;
    controller.getGraphManager().getClippingAssembly(clipAssembly, true, false);

    bool isOrtho = (std::fabs(abs(clickInfo.fov)) <= std::numeric_limits<double>::epsilon());
    TlScanOverseer::setWorkingScansTransfo(controller.getGraphManager().getVisiblePointCloudInstances(clickInfo.panoramic, true, true));

    double cosAngleThreshold = atan(clickInfo.heightAt1m * pointSize / (1.0 * clickInfo.height)); // angle across a visible point in the viewport
    //if orthographic view, cosAngleThreshold represents the distance between the edges of a point, in a plane orthogonal to the view
    cosAngleThreshold = isOrtho ? clickInfo.heightAt1m * pointSize / (1.0 * clickInfo.height) : cos(cosAngleThreshold);

    RayTracingDisplayFilterSettings displayFilterSettings = buildRayTracingDisplayFilterSettings(clickInfo);

    TlStreamLock streamLock;
    if (TlScanOverseer::getInstance().rayTracing(clickInfo.ray, clickInfo.rayOrigin, result, cosAngleThreshold, clipAssembly, isOrtho, scanName, &displayFilterSettings) == false)
    {
        controller.updateInfo(new GuiDataTmpMessage(TEXT_RAYTRACING_FAILED));
        FUNCLOG << "picking nan detected" << LOGENDL;
        result = glm::dvec3(NAN);
    }
    return result;
}

glm::dvec3 ARayTracingContext::Triangle::normal() const
{
    glm::vec3 n = glm::cross(B - A, C - A);
    return glm::normalize(n);
}

void ARayTracingContext::rayTraceMesh(const ClickInfo& clickInfo, glm::dvec3& retPoint, glm::dvec3& retNormal)
{
    retPoint = glm::dvec3(NAN, NAN, NAN);
    retNormal = glm::dvec3(NAN, NAN, NAN);
    float minDist = std::numeric_limits<float>::max();
    glm::vec3 origin = clickInfo.rayOrigin; // conversion to float
    glm::vec3 dir = clickInfo.ray;       // conversion to float

    std::lock_guard<std::mutex> lock(m_meshMutex);
    for (const Triangle& t : m_triangles)
    {
        glm::vec3 hit;
        if (rayTriangleIntersection(origin, dir, t, hit))
        {
            float dist = length(origin - hit);
            if (dist < minDist)
            {
                minDist = dist;
                retPoint = hit;
                retNormal = t.normal();
            }
        }
    }
}
