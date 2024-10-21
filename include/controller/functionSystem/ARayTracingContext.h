#ifndef A_RAY_TRACING_CONTEXT_H
#define A_RAY_TRACING_CONTEXT_H

#include "controller/functionSystem/AContext.h"
#include "models/pointCloud/TLS.h"
#include "models/3d/ClickInfo.h"
#include "models/Types.hpp"
#include "utils/safe_ptr.h"

#include <deque>
#include <mutex>
#include <unordered_set>
#include <QString>

class MeshBuffer;

class AGraphNode;

class ARayTracingContext : public AContext
{
public:
    ARayTracingContext(const ContextId& id);
    ~ARayTracingContext();
    ContextState start(Controller& controller) override;
    virtual ContextState feedMessage(IMessage* message, Controller& controller);
    virtual ContextState launch(Controller& controller) = 0;
    virtual ContextState abort(Controller& controller);
    virtual ContextState validate(Controller& controller);
    bool canAutoRelaunch() const = 0;

    ContextState waitForNextPoint(Controller& controller, QString errorOnPrecedentPointMessage = "");
    bool pointMissing();

    virtual ContextType getType() const override = 0;

protected:
    //if the next point fail, return false
    bool getNextPosition(Controller& controller);
    void nextText(Controller& controller);

    // TODO - pass in private
    struct Triangle
    {
        glm::vec3 A;
        glm::vec3 B;
        glm::vec3 C;
        glm::dvec3 normal() const;
    };
    bool downloadTriangleList(const std::shared_ptr<MeshBuffer> meshBuffer, glm::dmat4 meshTransfo);
    bool getTriangleList(void* bufferData, const std::shared_ptr<MeshBuffer> meshBuffer, glm::dmat4 meshTransfo, std::vector<Triangle>& triangles);
    glm::vec3 minimalDistanceOnMesh(glm::vec3 M);
    glm::vec3 projectionOnTriangle(const glm::vec3& M, const Triangle t);
    glm::vec3 projectionAlongAxis(const glm::vec3& M, const Triangle& t, const glm::vec3& N);
    bool rayTriangleIntersection(const glm::vec3& M, const glm::vec3& D, const Triangle& t, glm::vec3& hitPt);

private:
    glm::dvec3 rayTracePointClouds(Controller& controller, ClickInfo& clickInfo, std::string& scanName);
    void rayTraceMesh(const ClickInfo& clickInfo, glm::dvec3& retPoint, glm::dvec3& retNormal);

protected:
    tls::ScanGuid m_panoramic;
    glm::dvec3 m_lastCameraPos;
    bool m_repeatInput;
	bool m_isDisplayingClickTarget;

    struct ClickUsage {
        bool getObjectCenter;
        std::unordered_set<ElementType> objectTypesAccepted;
        QString uiMessage;
    };
    std::vector<ClickUsage> m_usages;

    struct ClickResult {
        glm::dvec3 position;
        glm::dvec3 normal;  // optional, used with mesh
        SafePtr<AGraphNode> object;
        std::string scanName;
    };
    std::deque<ClickResult> m_clickResults;

private:
    std::mutex m_clickMutex;
    std::deque<ClickInfo> m_waitingClicks;

    SafePtr<AGraphNode> m_meshSelected;
    std::mutex m_meshMutex;
    std::vector<Triangle> m_triangles;

};

#endif
