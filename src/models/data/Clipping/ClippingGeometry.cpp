#include "models/data/clipping/ClippingGeometry.h"

// TODO - Replace the "ClippingMode" in the "ClippingGeometry" by a table of Decision by inclusion.
constexpr ClipAccepted g_decisionTable[(size_t)ClippingMode::Max_Enum][(size_t)ClipResult::Max_Enum] = {
    { ClipAccepted::No, ClipAccepted::Partially, ClipAccepted::Yes }, // ClippingMode::showInterior
    { ClipAccepted::Yes, ClipAccepted::Partially, ClipAccepted::No }, // ClippingMode::showExterior
    { ClipAccepted::No, ClipAccepted::Partially, ClipAccepted::Yes }, // ClippingMode::ramp
};

ClipResult clipCube_box(const glm::dvec3& minCorner, double sideSize, const ClippingGeometry& geom)
{
    // This test is only valid if the clipping is a box
    double xMin(DBL_MAX), xMax(-DBL_MAX), yMin(DBL_MAX), yMax(-DBL_MAX), zMin(DBL_MAX), zMax(-DBL_MAX);
    for (int x = 0; x < 2; x++)
    {
        for (int y = 0; y < 2; y++)
        {
            for (int z = 0; z < 2; z++)
            {
                glm::dvec4 corner4(minCorner + glm::dvec3(x, y, z) * sideSize, 1.0);
                corner4 = geom.matRT_inv * corner4;

                if (xMin > corner4[0])
                    xMin = corner4[0];
                if (xMax < corner4[0])
                    xMax = corner4[0];
                if (yMin > corner4[1])
                    yMin = corner4[1];
                if (yMax < corner4[1])
                    yMax = corner4[1];
                if (zMin > corner4[2])
                    zMin = corner4[2];
                if (zMax < corner4[2])
                    zMax = corner4[2];
            }
        }
    }

    bool inside = (xMin >= -geom.params.x) && (xMax <= geom.params.x) &&
        (yMin >= -geom.params.y) && (yMax <= geom.params.y) &&
        (zMin >= -geom.params.z) && (zMax <= geom.params.z);

    bool outside = (xMin > geom.params.x) || (xMax < -geom.params.x) ||
        (yMin > geom.params.y) || (yMax < -geom.params.y) ||
        (zMin > geom.params.z) || (zMax < -geom.params.z);

    return (inside ? ClipResult::Inside : (outside ? ClipResult::Outside : ClipResult::Partial));
}

ClipResult clipPoint_box(const glm::dvec4& point, const ClippingGeometry& geom)
{
    bool inside = false;
    glm::dvec4 localPt = geom.matRT_inv * point;

    inside = (abs(localPt.x) <= geom.params.x) &&
        (abs(localPt.y) <= geom.params.y) &&
        (abs(localPt.z) <= geom.params.z);

    return (inside ? ClipResult::Inside : ClipResult::Outside);
}

/*
* ClippingGeometry
*/

ClippingGeometry::ClippingGeometry(ClippingShape _shape, ClippingMode _mode, const glm::dmat4& _matRT_inv, const glm::vec4& _params, int _steps)
    : shape(_shape)
    , mode(_mode)
    , cst_decisionTable{ g_decisionTable[(size_t)_mode][(size_t)ClipResult::Outside], 
                         g_decisionTable[(size_t)_mode][(size_t)ClipResult::Partial],
                         g_decisionTable[(size_t)_mode][(size_t)ClipResult::Inside] }
    , matRT_inv(_matRT_inv)
    , params(_params)
    , color(glm::vec3(0.f))
    , rampSteps(_steps)
    , gpuDrawId(0)
    , isSelected(false)
{
    switch (shape)
    {
    case ClippingShape::box:
        f_clipCube = clipCube_box;
        f_clipPoint = clipPoint_box;

        break;
    case ClippingShape::cylinder:
        //f_clipCube = clipCube_asSphere_cylinder;
        //f_clipSphere = clipSphere_cylinder;
        //f_clipPoint = clipPoint_cylinder;
        break;
    case ClippingShape::sphere:

        break;

    case ClippingShape::torus:
        break;
    }
}

/*
* IClippingGeometry
*/

IClippingGeometry::IClippingGeometry(ClippingMode _mode, const glm::dmat4& _matRT_inv, const glm::vec4& _params, int steps, ClippingGpuId _gpuDrawId)
    : mode(_mode)
    , matRT_inv(_matRT_inv)
    , matRT_inv_store(_matRT_inv)
    , params(_params)
    , rampSteps(steps)
    , gpuDrawId(_gpuDrawId)
{}

/*
* BoxClippingGeometry
*/

BoxClippingGeometry::BoxClippingGeometry(ClippingMode mode, const glm::dmat4& matRT_inv, const glm::vec4& params, int steps)
    : IClippingGeometry(mode, matRT_inv, params, steps, 0)
{
    rampSteps = steps;
}

void BoxClippingGeometry::testSphere(const glm::dvec4& center, double radius, bool& retAccept, bool& retReject) const
{
    glm::dvec4 localCenter = matRT_inv * center;
    bool inside = false;
    bool outside = false;

    // Simplification avec des valeurs absolues
    inside = abs(localCenter.x) + radius <= params.x &&
        abs(localCenter.y) + radius <= params.y &&
        abs(localCenter.z) + radius <= params.z;

    // Simplification avec des valeurs absolues
    outside = abs(localCenter.x) - radius > params.x ||
        abs(localCenter.y) - radius > params.y ||
        abs(localCenter.z) - radius > params.z;

    // Return
    retAccept = (mode == ClippingMode::showInterior) ? inside : outside;
    retReject = (mode == ClippingMode::showInterior) ? outside : inside;
}

void BoxClippingGeometry::testCube(const glm::dvec3& minCorner, double sideSize, bool& retAccept, bool& retReject) const
{
    // This test is only valid if the clipping is a box
    double xMin(DBL_MAX), xMax(-DBL_MAX), yMin(DBL_MAX), yMax(-DBL_MAX), zMin(DBL_MAX), zMax(-DBL_MAX);
    for (int x = 0; x < 2; x++)
    {
        for (int y = 0; y < 2; y++)
        {
            for (int z = 0; z < 2; z++)
            {
                glm::dvec4 corner4(minCorner + glm::dvec3(x, y, z) * sideSize, 1.0);
                corner4 = matRT_inv * corner4;

                if (xMin > corner4[0])
                    xMin = corner4[0];
                if (xMax < corner4[0])
                    xMax = corner4[0];
                if (yMin > corner4[1])
                    yMin = corner4[1];
                if (yMax < corner4[1])
                    yMax = corner4[1];
                if (zMin > corner4[2])
                    zMin = corner4[2];
                if (zMax < corner4[2])
                    zMax = corner4[2];
            }
        }
    }

    bool inside = (xMin >= -params.x) && (xMax <= params.x) &&
        (yMin >= -params.y) && (yMax <= params.y) &&
        (zMin >= -params.z) && (zMax <= params.z);
    bool outside = (xMin > params.x) || (xMax < -params.x) ||
        (yMin > params.y) || (yMax < -params.y) ||
        (zMin > params.z) || (zMax < -params.z);

    retAccept = (mode == ClippingMode::showInterior) ? inside : outside;
    retReject = (mode == ClippingMode::showInterior) ? outside : inside;
}

bool BoxClippingGeometry::testPoint(const glm::dvec4& point) const
{
    bool inside = false;
    glm::dvec4 localPt = matRT_inv * point;

    inside = (abs(localPt.x) <= params.x) &&
        (abs(localPt.y) <= params.y) &&
        (abs(localPt.z) <= params.z);

    return ((mode == ClippingMode::showInterior) ? inside : !inside);
}

ClippingShape BoxClippingGeometry::getShape() const
{
    return ClippingShape::box;
}

/*
* ABasicClippingGeometry
*/

ABasicClippingGeometry::ABasicClippingGeometry(ClippingMode mode, const glm::dmat4& matRT_inv, const glm::vec4& params, int steps, ClippingGpuId gpuDrawId)
    : IClippingGeometry(mode, matRT_inv, params, steps, gpuDrawId)
{}

void ABasicClippingGeometry::testCube(const glm::dvec3& minCorner, double sideSize, bool& retAccept, bool& retReject) const
{
    return testSphere(glm::dvec4(minCorner + glm::dvec3(sideSize / 2.0), 1.0), sideSize * sqrt(3.0) / 2.0, retAccept, retReject);
}

/*
* SphereClippingGeometry
*/

SphereClippingGeometry::SphereClippingGeometry(ClippingMode mode, const glm::dmat4& matRT_inv, const glm::vec4& params, int steps)
    : ABasicClippingGeometry(mode, matRT_inv, params, steps, 0)
{}

void SphereClippingGeometry::testSphere(const glm::dvec4& center, double radius, bool& retAccept, bool& retReject) const
{
    glm::dvec4 localCenter = matRT_inv * center;
    bool inside = false;
    bool outside = false;

    double dxyz = std::sqrt(localCenter.x * localCenter.x + localCenter.y * localCenter.y + localCenter.z * localCenter.z);
    inside = (dxyz - radius) >= params.x && (dxyz + radius) <= params.y;
    outside = (dxyz + radius) < params.x || (dxyz - radius) > params.y;

    // Return
    retAccept = (mode == ClippingMode::showInterior) ? inside : outside;
    retReject = (mode == ClippingMode::showInterior) ? outside : inside;
}

bool SphereClippingGeometry::testPoint(const glm::dvec4& point) const
{
    glm::dvec4 localPt = matRT_inv * point;
    double dxyz = std::sqrt(localPt.x * localPt.x + localPt.y * localPt.y + localPt.z * localPt.z);
    bool inside = (dxyz >= params.x) && (dxyz <= params.y);
    return ((mode == ClippingMode::showInterior) ? inside : !inside);
}

ClippingShape SphereClippingGeometry::getShape() const
{
    return ClippingShape::sphere;
}

/*
* CylinderClippingGeometry
*/

CylinderClippingGeometry::CylinderClippingGeometry(ClippingMode mode, const glm::dmat4& matRT_inv, const glm::vec4& params, int steps)
   : ABasicClippingGeometry(mode, matRT_inv, params, steps, 0)
{}

void CylinderClippingGeometry::testSphere(const glm::dvec4& center, double radius, bool& retAccept, bool& retReject) const
{
    glm::dvec4 localCenter = matRT_inv * center;
    bool inside = false;
    bool outside = false;

    double dxy = std::sqrt(localCenter.x * localCenter.x + localCenter.y * localCenter.y);

    inside = (dxy - radius) >= params.x && (dxy + radius) <= params.y;
    inside &= abs(localCenter.z) + radius <= params.z;

    outside = (dxy + radius) < params.x || (dxy - radius) > params.y;
    outside |= abs(localCenter.z) - radius > params.z;

    // Return
    retAccept = (mode == ClippingMode::showInterior) ? inside : outside;
    retReject = (mode == ClippingMode::showInterior) ? outside : inside;
}

bool CylinderClippingGeometry::testPoint(const glm::dvec4& point) const
{
    bool inside = false;
    glm::dvec4 localPt = matRT_inv * point;

    double dxy = std::sqrt(localPt.x * localPt.x + localPt.y * localPt.y);
    inside = (dxy >= params.x) && (dxy <= params.y) && (abs(localPt.z) <= params.z);

    return ((mode == ClippingMode::showInterior) ? inside : !inside);
}

ClippingShape CylinderClippingGeometry::getShape() const
{
    return ClippingShape::cylinder;
}

/*
* TorusClippingGeometry
*/

TorusClippingGeometry::TorusClippingGeometry(ClippingMode mode, const glm::dmat4& matRT_inv, const glm::vec4& params, int steps)
    : ABasicClippingGeometry(mode, matRT_inv, params, steps, 0)
{}

void TorusClippingGeometry::testSphere(const glm::dvec4& center, double radius, bool& retAccept, bool& retReject) const
{}

bool TorusClippingGeometry::testPoint(const glm::dvec4& point) const
{
    return false;
}

ClippingShape TorusClippingGeometry::getShape() const
{
    return ClippingShape::torus;
}

/*
* ClippingAssembly
*/

ClippingAssembly::ClippingAssembly()
{}

ClippingAssembly::~ClippingAssembly()
{
    clippingUnion.clear();
    clippingIntersection.clear();
    rampActives.clear();
}

void ClippingAssembly::testSphere(const glm::dvec4& center, double radius, bool& retAccept, bool& retReject, ClippingAssembly& retAssembly) const
{
    // Union
    bool acceptUnion = clippingUnion.empty() ? true : false;
    bool rejectUnion = clippingUnion.empty() ? false : true;
    for (const std::shared_ptr<IClippingGeometry>& cg : clippingUnion)
    {
        bool accept = false;
        bool reject = false;
        // Le test dépend de la géométrie
        cg->testSphere(center, radius, accept, reject);

        // NOTE - We are doing an union :
        //   -> we need at least ONE geometry to accept
        //   -> we need ALL geometries to reject
        acceptUnion |= accept;
        rejectUnion &= reject;

        if (!reject)
            retAssembly.clippingUnion.push_back(cg);
    }
    if (acceptUnion)
        retAssembly.clippingUnion.clear();

    // Intersection
    bool acceptInter = true;
    bool rejectInter = false;
    for (const std::shared_ptr<IClippingGeometry>& cg : clippingIntersection)
    {
        bool accept = false;
        bool reject = false;
        cg->testSphere(center, radius, accept, reject);

        // NOTE - We are doing an intersection :
        //   -> we need ALL geometries to accept
        //   -> we need only ONE geometry to reject
        acceptInter &= accept;
        rejectInter |= reject;

        if (!accept)
            retAssembly.clippingIntersection.push_back(cg);
    }
    if (acceptInter)
        retAssembly.clippingIntersection.clear();

    // Intersection of the {Union} and the {Intersection}
    retAccept = acceptUnion && acceptInter;
    retReject = rejectUnion || rejectInter;
}

void ClippingAssembly::testCube(const glm::dvec3& minCorner, double sideSize, bool& retAccept, bool& retReject, ClippingAssembly& retAssembly) const
{
    // Union
    bool acceptUnion = clippingUnion.empty() ? true : false;
    bool rejectUnion = clippingUnion.empty() ? false : true;
    for (const std::shared_ptr<IClippingGeometry>& cg : clippingUnion)
    {
        bool accept = false;
        bool reject = false;
        // Le test dépend de la géométrie
        cg->testCube(minCorner, sideSize, accept, reject);

        // NOTE - We are doing an union :
        //   -> we need at least ONE geometry to accept
        //   -> we need ALL geometries to reject
        acceptUnion |= accept;
        rejectUnion &= reject;

        // the full condition is (!reject && !accept)
        // But this condition is overriden by the "if (acceptUnion) clear()"
        if (!reject)
            retAssembly.clippingUnion.push_back(cg);
    }
    if (acceptUnion)
        retAssembly.clippingUnion.clear();

    // Intersection
    bool acceptInter = true;
    bool rejectInter = false;
    for (const std::shared_ptr<IClippingGeometry>& cg : clippingIntersection)
    {
        bool accept = false;
        bool reject = false;
        cg->testCube(minCorner, sideSize, accept, reject);

        // NOTE - We are doing an intersection :
        //   -> we need ALL geometries to accept
        //   -> we need only ONE geometry to reject
        acceptInter &= accept;
        rejectInter |= reject;

        // the full condition is (!reject && !accept)
        // But this condition is overriden by the "if (acceptInter) clear()"
        if (!accept)
            retAssembly.clippingIntersection.push_back(cg);
    }
    if (acceptInter)
        retAssembly.clippingIntersection.clear();

    for (const std::shared_ptr<IClippingGeometry>& cg : rampActives)
    {
        bool accept = false;
        bool reject = false;
        cg->testCube(minCorner, sideSize, accept, reject);

        // On garde toutes les rampes qui ne sont pas rejetés.
        // Même si une rampe est acceptés, on doit la garder pour la colorer dans le shader.
        if (!reject)
            retAssembly.rampActives.push_back(cg);
    }

    // Intersection of the {Union} and the {Intersection}
    retAccept = acceptUnion && acceptInter && retAssembly.rampActives.empty();
    retReject = (rejectUnion || rejectInter);
}

bool ClippingAssembly::testPoint(const glm::dvec4& point) const
{
    bool intersectionOk = true;
    bool unionOk = clippingUnion.empty() ? true : false;
    for (const std::shared_ptr<IClippingGeometry>& cg : clippingIntersection)
    {
        intersectionOk &= cg->testPoint(point);
        // early test
        if (intersectionOk == false)
            return false;
    }

    for (const std::shared_ptr<IClippingGeometry>& cg : clippingUnion)
    {
        unionOk |= cg->testPoint(point);
        // early test
        if (unionOk == true)
            break;
    }

    return (intersectionOk && unionOk);
}

void ClippingAssembly::addTransformation(const glm::dmat4& matrix)
{
    for (std::shared_ptr<IClippingGeometry>& cg : clippingIntersection)
    {
        cg->matRT_inv *= matrix;
    }
    for (std::shared_ptr<IClippingGeometry>& cg : clippingUnion)
    {
        cg->matRT_inv *= matrix;
    }
    for (std::shared_ptr<IClippingGeometry>& cg : rampActives)
    {
        cg->matRT_inv *= matrix;
    }
}

void ClippingAssembly::clearMatrix()
{
    for (std::shared_ptr<IClippingGeometry>& cg : clippingIntersection)
    {
        cg->matRT_inv = cg->matRT_inv_store;
    }
    for (std::shared_ptr<IClippingGeometry>& cg : clippingUnion)
    {
        cg->matRT_inv = cg->matRT_inv_store;
    }
    for (std::shared_ptr<IClippingGeometry>& cg : rampActives)
    {
        cg->matRT_inv = cg->matRT_inv_store;
    }
}

bool ClippingAssembly::empty()
{
    return (clippingIntersection.empty() && clippingUnion.empty());
}

/*
* ClippingAssembly_bis
*/

ClippingAssembly_bis::ClippingAssembly_bis()
{}

ClippingAssembly_bis::~ClippingAssembly_bis()
{
    clippingUnion.clear();
    clippingIntersection.clear();
    rampActives.clear();
}

void ClippingAssembly_bis::addTransformation(const glm::dmat4& matrix)
{
    for (ClippingGeometry& cg : clippingIntersection)
    {
        cg.matRT_inv *= matrix;
    }
    for (ClippingGeometry& cg : clippingUnion)
    {
        cg.matRT_inv *= matrix;
    }
    for (ClippingGeometry& cg : rampActives)
    {
        cg.matRT_inv *= matrix;
    }
}

void ClippingAssembly_bis::testSphere(const glm::dvec4& center, double radius, bool& retAccept, bool& retReject, AssemblyDrawResult& retAssembly) const
{
    
}

void ClippingAssembly_bis::testCube(const glm::dvec3& minCorner, double sideSize, bool& retAccept, bool& retReject, const AssemblyDrawResult& srcAssembly, AssemblyDrawResult& retAssembly) const
{
    // Union
    bool acceptUnion = clippingUnion.empty() ? true : false;
    bool rejectUnion = clippingUnion.empty() ? false : true;
    for (uint32_t idx : srcAssembly.unionIdxToTest)
    {
        const ClippingGeometry& cg = clippingUnion[idx];
        // Le test dépend de la géométrie
        ClipResult clip = cg.f_clipCube(minCorner, sideSize, cg);

        ClipAccepted deci = g_decisionTable[(size_t)cg.mode][(size_t)clip];
        // NOTE - We are doing an union :
        //   -> we need at least ONE geometry to accept
        //   -> we need ALL geometries to reject
        acceptUnion |= (deci == ClipAccepted::Yes);
        rejectUnion &= (deci == ClipAccepted::No);

        if (deci == ClipAccepted::Partially)
            retAssembly.unionIdxToTest.push_back(idx);
    }
    if (acceptUnion)
        retAssembly.unionIdxToTest.clear();

    // Intersection
    bool acceptInter = true;
    bool rejectInter = false;
    for (uint32_t idx : srcAssembly.interIdxToTest)
    {
        const ClippingGeometry& cg = clippingIntersection[idx];

        ClipResult clip = cg.f_clipCube(minCorner, sideSize, cg);

        ClipAccepted acceptance = g_decisionTable[(size_t)cg.mode][(size_t)clip];
        // NOTE - We are doing an intersection :
        //   -> we need ALL geometries to accept
        //   -> we need only ONE geometry to reject
        acceptInter &= (acceptance == ClipAccepted::Yes);
        rejectInter |= (acceptance == ClipAccepted::No);

        // Insert what is left to test
        if (acceptance == ClipAccepted::Partially)
            retAssembly.interIdxToTest.push_back(idx);
    }
    if (acceptInter)
        retAssembly.interIdxToTest.clear();

    // Ramps
    retAssembly.rampIdxToDraw = srcAssembly.rampIdxToDraw;
    for (uint32_t idx : srcAssembly.rampIdxToTest)
    {
        const ClippingGeometry& cg = clippingIntersection[idx];

        ClipResult clip = cg.f_clipCube(minCorner, sideSize, cg);

        ClipAccepted acceptance = g_decisionTable[(size_t)cg.mode][(size_t)clip];

        if (acceptance == ClipAccepted::Partially)
            retAssembly.rampIdxToTest.push_back(idx);

        // On garde toutes les rampes qui ne sont pas rejetés.
        // Même si une rampe est acceptés, on doit la garder pour la colorer dans le shader.
        if (acceptance == ClipAccepted::Yes)
            retAssembly.rampIdxToDraw.push_back(idx);
    }

    // Intersection of the {Union} and the {Intersection}
    retAccept = acceptUnion && acceptInter && retAssembly.rampIdxToTest.empty() && retAssembly.rampIdxToDraw.empty();
    retReject = (rejectUnion || rejectInter);
}

bool ClippingAssembly_bis::testPoint(const glm::dvec4& point) const
{
    bool intersectionOk = true;
    bool unionOk = clippingUnion.empty() ? true : false;
    for (const ClippingGeometry& cg : clippingIntersection)
    {
        ClipResult clip = cg.f_clipPoint(point, cg);
        ClipAccepted decision = g_decisionTable[(size_t)cg.mode][(size_t)clip];
        intersectionOk &= (decision == ClipAccepted::Yes);
        // early test
        if (intersectionOk == false)
            return false;
    }

    for (const ClippingGeometry& cg : clippingUnion)
    {
        ClipResult clip = cg.f_clipPoint(point, cg);
        ClipAccepted decision = g_decisionTable[(size_t)cg.mode][(size_t)clip];
        unionOk |= (decision == ClipAccepted::Yes);
        // early test
        if (unionOk == true)
            break;
    }

    return (intersectionOk && unionOk);
}