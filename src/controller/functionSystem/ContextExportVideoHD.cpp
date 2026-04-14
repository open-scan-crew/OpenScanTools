#include "controller/functionSystem/ContextExportVideoHD.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/IControlListener.h"
#include "controller/controls/AnimationHelper.h"
#include "controller/controls/ControlViewPoint.h"

#include "models/graph/GraphManager.h"
#include "models/graph/AGraphNode.h"
#include "models/graph/AClippingNode.h"
#include "models/graph/CameraNode.h"
#include "models/graph/ViewPointNode.h"
#include "models/application/ViewPointAnimation.h"

#include "controller/messages/FilesMessage.h"
#include "controller/messages/GeneralMessage.h"
#include "controller/messages/VideoExportParametersMessage.h"

#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataIO.h"
#include "gui/GuiData/GuiDataMeasure.h"
#include "gui/GuiData/GuiDataHD.h"
#include "gui/GuiData/GuiDataRendering.h"

#include "gui/texts/ContextTexts.hpp"
#include "gui/UITransparencyConverter.h"

#include "utils/Logger.h"
#include "utils/Utils.h"
#include "utils/ColorConversion.h"
#include "utils/math/basic_define.h"
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include <glm/gtx/quaternion.hpp>
#include <filesystem>
#include <QtCore/QProcess>
#include <QtCore/QStringList>
#include "utils/Config.h"

namespace
{
QString resolveFfmpegExecutable()
{
    std::filesystem::path ffmpegDir = Config::getFFmpegPath();
    if (!ffmpegDir.empty())
    {
        std::filesystem::path candidate = ffmpegDir / "ffmpeg.exe";
        if (!std::filesystem::exists(candidate))
            candidate = ffmpegDir / "ffmpeg";
        return QString::fromStdWString(candidate.wstring());
    }
    return QStringLiteral("ffmpeg");
}

glm::vec3 color32ToNormalizedRgb(const Color32& color)
{
    return glm::vec3(
        static_cast<float>(color.r) / 255.0f,
        static_cast<float>(color.g) / 255.0f,
        static_cast<float>(color.b) / 255.0f);
}

Color32 normalizedRgbToColor32(const glm::vec3& rgb, uint8_t alpha)
{
    const glm::vec3 clamped = glm::clamp(rgb, glm::vec3(0.0f), glm::vec3(1.0f));
    return Color32(
        static_cast<uint8_t>(std::round(clamped.x * 255.0f)),
        static_cast<uint8_t>(std::round(clamped.y * 255.0f)),
        static_cast<uint8_t>(std::round(clamped.z * 255.0f)),
        alpha);
}

double smoothstep01(double x)
{
    x = std::clamp(x, 0.0, 1.0);
    return x * x * (3.0 - 2.0 * x);
}

double smootherstep01(double x)
{
    x = std::clamp(x, 0.0, 1.0);
    return x * x * x * (x * (x * 6.0 - 15.0) + 10.0);
}

glm::dvec3 evaluateCentripetalCatmullRom(
    const glm::dvec3& p0,
    const glm::dvec3& p1,
    const glm::dvec3& p2,
    const glm::dvec3& p3,
    double t)
{
    const double alpha = 0.5;
    const auto computeKnot = [alpha](double prev, const glm::dvec3& a, const glm::dvec3& b)
    {
        return prev + std::pow(std::max(glm::distance(a, b), 1e-9), alpha);
    };

    const double t0 = 0.0;
    const double t1 = computeKnot(t0, p0, p1);
    const double t2 = computeKnot(t1, p1, p2);
    const double t3 = computeKnot(t2, p2, p3);
    const double u = t1 + (t2 - t1) * std::clamp(t, 0.0, 1.0);

    const glm::dvec3 a1 = ((t1 - u) / (t1 - t0)) * p0 + ((u - t0) / (t1 - t0)) * p1;
    const glm::dvec3 a2 = ((t2 - u) / (t2 - t1)) * p1 + ((u - t1) / (t2 - t1)) * p2;
    const glm::dvec3 a3 = ((t3 - u) / (t3 - t2)) * p2 + ((u - t2) / (t3 - t2)) * p3;

    const glm::dvec3 b1 = ((t2 - u) / (t2 - t0)) * a1 + ((u - t0) / (t2 - t0)) * a2;
    const glm::dvec3 b2 = ((t3 - u) / (t3 - t1)) * a2 + ((u - t1) / (t3 - t1)) * a3;

    return ((t2 - u) / (t2 - t1)) * b1 + ((u - t1) / (t2 - t1)) * b2;
}

glm::dquat quaternionLog(const glm::dquat& q)
{
    const glm::dquat normalized = glm::normalize(q);
    const glm::dvec3 v(normalized.x, normalized.y, normalized.z);
    const double vNorm = glm::length(v);
    if (vNorm < 1e-12)
        return glm::dquat(0.0, 0.0, 0.0, 0.0);

    const double angle = std::atan2(vNorm, normalized.w);
    const glm::dvec3 axis = v / vNorm;
    const glm::dvec3 logV = axis * angle;
    return glm::dquat(0.0, logV.x, logV.y, logV.z);
}

glm::dquat quaternionExp(const glm::dquat& q)
{
    const glm::dvec3 v(q.x, q.y, q.z);
    const double theta = glm::length(v);
    if (theta < 1e-12)
        return glm::normalize(glm::dquat(std::cos(theta), v.x, v.y, v.z));

    const glm::dvec3 axis = v / theta;
    const double sinTheta = std::sin(theta);
    return glm::normalize(glm::dquat(std::cos(theta), axis.x * sinTheta, axis.y * sinTheta, axis.z * sinTheta));
}

glm::dquat slerpShortestPath(const glm::dquat& q0, const glm::dquat& q1, double t)
{
    glm::dquat q1Shortest = q1;
    if (glm::dot(q0, q1Shortest) < 0.0)
        q1Shortest = -q1Shortest;
    return glm::slerp(q0, q1Shortest, std::clamp(t, 0.0, 1.0));
}

glm::dquat computeSquadIntermediate(const glm::dquat& qPrev, const glm::dquat& qCurr, const glm::dquat& qNext)
{
    const glm::dquat qPrevAligned = (glm::dot(qCurr, qPrev) < 0.0) ? -qPrev : qPrev;
    const glm::dquat qNextAligned = (glm::dot(qCurr, qNext) < 0.0) ? -qNext : qNext;

    const glm::dquat invCurr = glm::inverse(qCurr);
    const glm::dquat logPrev = quaternionLog(invCurr * qPrevAligned);
    const glm::dquat logNext = quaternionLog(invCurr * qNextAligned);

    const glm::dquat averageLog(
        0.0,
        -0.25 * (logPrev.x + logNext.x),
        -0.25 * (logPrev.y + logNext.y),
        -0.25 * (logPrev.z + logNext.z));

    return glm::normalize(qCurr * quaternionExp(averageLog));
}

glm::dquat squadShortestPath(const glm::dquat& q0, const glm::dquat& q1, const glm::dquat& s0, const glm::dquat& s1, double t)
{
    const double clampedT = std::clamp(t, 0.0, 1.0);

    glm::dquat q1Aligned = q1;
    glm::dquat s1Aligned = s1;
    if (glm::dot(q0, q1Aligned) < 0.0)
    {
        q1Aligned = -q1Aligned;
        s1Aligned = -s1Aligned;
    }

    glm::dquat s0Aligned = s0;
    if (glm::dot(q0, s0Aligned) < 0.0)
        s0Aligned = -s0Aligned;
    if (glm::dot(q1Aligned, s1Aligned) < 0.0)
        s1Aligned = -s1Aligned;

    const glm::dquat slerpDirect = slerpShortestPath(q0, q1Aligned, clampedT);
    const glm::dquat slerpControl = slerpShortestPath(s0Aligned, s1Aligned, clampedT);
    const double blend = 2.0 * clampedT * (1.0 - clampedT);
    return glm::normalize(slerpShortestPath(slerpDirect, slerpControl, blend));
}

void enforceQuaternionSignContinuity(std::vector<glm::dquat>& quaternions)
{
    for (size_t i = 1; i < quaternions.size(); ++i)
    {
        if (glm::dot(quaternions[i - 1], quaternions[i]) < 0.0)
            quaternions[i] = -quaternions[i];
    }
}

std::vector<double> computeMonotonicCubicSlopes(const std::vector<double>& x, const std::vector<double>& y)
{
    const size_t count = x.size();
    std::vector<double> slopes(count, 0.0);
    if (count < 2 || y.size() != count)
        return slopes;

    std::vector<double> h(count - 1, 0.0);
    std::vector<double> delta(count - 1, 0.0);
    for (size_t i = 0; i + 1 < count; ++i)
    {
        h[i] = std::max(x[i + 1] - x[i], 1e-9);
        delta[i] = (y[i + 1] - y[i]) / h[i];
    }

    slopes.front() = delta.front();
    slopes.back() = delta.back();
    if (count == 2)
        return slopes;

    for (size_t i = 1; i + 1 < count; ++i)
    {
        if (delta[i - 1] * delta[i] <= 0.0)
        {
            slopes[i] = 0.0;
            continue;
        }

        const double w1 = 2.0 * h[i] + h[i - 1];
        const double w2 = h[i] + 2.0 * h[i - 1];
        slopes[i] = (w1 + w2) / (w1 / delta[i - 1] + w2 / delta[i]);
    }
    return slopes;
}

double evaluateMonotonicCubicHermite(const std::vector<double>& x, const std::vector<double>& y, const std::vector<double>& slopes, double xQuery)
{
    if (x.size() < 2 || y.size() != x.size() || slopes.size() != x.size())
        return 0.0;
    if (xQuery <= x.front())
        return y.front();
    if (xQuery >= x.back())
        return y.back();

    auto upper = std::upper_bound(x.begin(), x.end(), xQuery);
    size_t i = static_cast<size_t>(std::distance(x.begin(), upper) - 1);
    i = std::min(i, x.size() - 2);

    const double x0 = x[i];
    const double x1 = x[i + 1];
    const double h = std::max(x1 - x0, 1e-9);
    const double t = std::clamp((xQuery - x0) / h, 0.0, 1.0);

    const double h00 = (2.0 * t * t * t - 3.0 * t * t + 1.0);
    const double h10 = (t * t * t - 2.0 * t * t + t);
    const double h01 = (-2.0 * t * t * t + 3.0 * t * t);
    const double h11 = (t * t * t - t * t);

    return h00 * y[i] + h10 * h * slopes[i] + h01 * y[i + 1] + h11 * h * slopes[i + 1];
}

void buildSampledPlaybackPath(
    const std::vector<SafePtr<ViewPointNode>>& viewpoints,
    std::vector<glm::dvec3>& outPositions,
    std::vector<glm::dquat>& outOrientations,
    std::vector<size_t>& outControlIndices)
{
    outPositions.clear();
    outOrientations.clear();
    outControlIndices.clear();
    if (viewpoints.size() < 2)
        return;

    const size_t controlCount = viewpoints.size();
    outControlIndices.resize(controlCount, 0);
    outPositions.reserve(controlCount * 12);
    outOrientations.reserve(controlCount * 12);

    std::vector<glm::dvec3> controlPositions;
    std::vector<glm::dquat> controlOrientations;
    controlPositions.reserve(controlCount);
    controlOrientations.reserve(controlCount);
    for (const SafePtr<ViewPointNode>& viewpoint : viewpoints)
    {
        ReadPtr<ViewPointNode> rViewpoint = viewpoint.cget();
        if (!rViewpoint)
        {
            outPositions.clear();
            outOrientations.clear();
            outControlIndices.clear();
            return;
        }
        controlPositions.push_back(rViewpoint->getCenter());
        controlOrientations.push_back(glm::normalize(rViewpoint->getOrientation()));
    }

    enforceQuaternionSignContinuity(controlOrientations);

    if (controlCount < 3)
    {
        outPositions = controlPositions;
        outOrientations = controlOrientations;
        for (size_t i = 0; i < controlCount; ++i)
            outControlIndices[i] = i;
        return;
    }

    outPositions.push_back(controlPositions[0]);
    outOrientations.push_back(controlOrientations[0]);
    outControlIndices[0] = 0;

    std::vector<glm::dquat> squadIntermediates(controlCount);
    squadIntermediates.front() = controlOrientations.front();
    squadIntermediates.back() = controlOrientations.back();
    for (size_t i = 1; i + 1 < controlCount; ++i)
        squadIntermediates[i] = computeSquadIntermediate(controlOrientations[i - 1], controlOrientations[i], controlOrientations[i + 1]);

    for (size_t i = 0; i + 1 < controlCount; ++i)
    {
        const glm::dvec3& p0 = (i == 0) ? controlPositions[i] : controlPositions[i - 1];
        const glm::dvec3& p1 = controlPositions[i];
        const glm::dvec3& p2 = controlPositions[i + 1];
        const glm::dvec3& p3 = (i + 2 < controlCount) ? controlPositions[i + 2] : controlPositions[i + 1];

        const double segmentDistance = glm::distance(p1, p2);
        const int positionSampleCount = std::clamp(static_cast<int>(std::ceil(segmentDistance / 0.25)), 4, 48);
        const double orientationAlignment = std::clamp(std::abs(glm::dot(controlOrientations[i], controlOrientations[i + 1])), 0.0, 1.0);
        const double orientationAngle = 2.0 * std::acos(orientationAlignment);
        const int orientationSampleCount = std::clamp(static_cast<int>(std::ceil(orientationAngle / glm::radians(2.0))), 1, 48);

        int sampleCount = std::max(positionSampleCount, orientationSampleCount);
        if (i + 1 == controlCount - 1)
            sampleCount = std::min(sampleCount * 2, 64);
        sampleCount = std::clamp(sampleCount, 4, 64);

        for (int step = 1; step <= sampleCount; ++step)
        {
            const double localT = static_cast<double>(step) / static_cast<double>(sampleCount);
            outPositions.push_back(evaluateCentripetalCatmullRom(p0, p1, p2, p3, localT));
            outOrientations.push_back(squadShortestPath(controlOrientations[i], controlOrientations[i + 1], squadIntermediates[i], squadIntermediates[i + 1], localT));
        }
        outControlIndices[i + 1] = outPositions.size() - 1;
    }
}

std::vector<double> computeSampledTimes(
    ViewPointAnimationMode mode,
    double targetDuration,
    const std::vector<double>& controlTimesInput,
    bool smoothTransitions,
    const std::vector<glm::dvec3>& sampledPositions,
    const std::vector<size_t>& controlIndices)
{
    std::vector<double> sampledTimes(sampledPositions.size(), 0.0);
    if (sampledPositions.size() < 2)
        return sampledTimes;

    std::vector<double> cumulativeDistances(sampledPositions.size(), 0.0);
    for (size_t i = 1; i < sampledPositions.size(); ++i)
        cumulativeDistances[i] = cumulativeDistances[i - 1] + glm::distance(sampledPositions[i - 1], sampledPositions[i]);
    const double totalDistance = cumulativeDistances.back();

    if (mode == ViewPointAnimationMode::ConstantSpeed || controlTimesInput.size() < 2)
    {
        for (size_t i = 1; i < sampledTimes.size(); ++i)
        {
            const double alpha = (totalDistance > 1e-9) ? cumulativeDistances[i] / totalDistance : static_cast<double>(i) / static_cast<double>(sampledTimes.size() - 1);
            sampledTimes[i] = targetDuration * alpha;
        }
        return sampledTimes;
    }

    std::vector<double> controlTimes = controlTimesInput;
    const double firstTime = controlTimes.front();
    for (double& t : controlTimes)
        t -= firstTime;

    if (mode == ViewPointAnimationMode::ConstantIntervals)
    {
        const size_t last = controlTimes.size() - 1;
        for (size_t i = 0; i <= last; ++i)
            controlTimes[i] = targetDuration * static_cast<double>(i) / static_cast<double>(last);
    }

    sampledTimes.front() = controlTimes.front();
    const size_t segmentCount = controlTimes.size() - 1;
    if (!smoothTransitions)
    {
        for (size_t seg = 0; seg < segmentCount; ++seg)
        {
            const size_t startIdx = controlIndices[seg];
            const size_t endIdx = std::max(controlIndices[seg + 1], startIdx + 1);
            const double startDistance = cumulativeDistances[startIdx];
            const double endDistance = cumulativeDistances[endIdx];
            const double segmentDuration = std::max(0.001, controlTimes[seg + 1] - controlTimes[seg]);
            for (size_t i = startIdx + 1; i <= endIdx; ++i)
            {
                const double segmentAlpha = (endDistance > startDistance) ? (cumulativeDistances[i] - startDistance) / (endDistance - startDistance) : static_cast<double>(i - startIdx) / static_cast<double>(endIdx - startIdx);
                sampledTimes[i] = controlTimes[seg] + segmentDuration * smoothstep01(segmentAlpha);
            }
        }
        return sampledTimes;
    }

    std::vector<double> controlRatios(controlTimes.size(), 0.0);
    controlRatios.front() = 0.0;
    for (size_t seg = 0; seg < segmentCount; ++seg)
    {
        const size_t endIdx = controlIndices[seg + 1];
        controlRatios[seg + 1] = (totalDistance > 1e-9)
            ? std::clamp(cumulativeDistances[endIdx] / totalDistance, 0.0, 1.0)
            : static_cast<double>(seg + 1) / static_cast<double>(segmentCount);
    }
    for (size_t i = 1; i < controlRatios.size(); ++i)
    {
        if (controlRatios[i] <= controlRatios[i - 1])
            controlRatios[i] = std::min(1.0, controlRatios[i - 1] + 1e-6);
    }
    controlRatios.back() = 1.0;

    const std::vector<double> slopes = computeMonotonicCubicSlopes(controlRatios, controlTimes);
    for (size_t i = 1; i < sampledPositions.size(); ++i)
    {
        const double ratio = (totalDistance > 1e-9)
            ? std::clamp(cumulativeDistances[i] / totalDistance, 0.0, 1.0)
            : static_cast<double>(i) / static_cast<double>(sampledPositions.size() - 1);
        const double easedRatio = smootherstep01(ratio);
        const double blendRatio = std::clamp(0.75 * ratio + 0.25 * easedRatio, 0.0, 1.0);
        sampledTimes[i] = evaluateMonotonicCubicHermite(controlRatios, controlTimes, slopes, blendRatio);
    }
    for (size_t i = 1; i < sampledTimes.size(); ++i)
        sampledTimes[i] = std::max(sampledTimes[i], sampledTimes[i - 1] + 1e-6);
    sampledTimes.back() = std::max(controlTimes.back(), sampledTimes[sampledTimes.size() - 2] + 1e-6);
    return sampledTimes;
}



bool supportsInterpolatedObjectTransform(ElementType type)
{
    return type == ElementType::Box ||
        type == ElementType::Cylinder ||
        type == ElementType::Sphere ||
        type == ElementType::MeshObject ||
        type == ElementType::Tag ||
        type == ElementType::Point ||
        type == ElementType::PCO;
}

bool isPositionOnlyInterpolatedObject(ElementType type)
{
    return type == ElementType::Tag || type == ElementType::Point;
}

bool supportsInterpolatedClippingDistances(ElementType type)
{
    return type == ElementType::Box ||
        type == ElementType::Tag ||
        type == ElementType::Point ||
        type == ElementType::Cylinder ||
        type == ElementType::Sphere ||
        type == ElementType::SimpleMeasure ||
        type == ElementType::PolylineMeasure;
}

bool supportsInterpolatedLengthThreshold(ElementType type)
{
    return type == ElementType::Cylinder ||
        type == ElementType::SimpleMeasure ||
        type == ElementType::PolylineMeasure;
}

void interpolateAndApplyObjects(const ViewPointNode& left, const ViewPointNode& right, double alpha)
{
    const auto& leftTransforms = left.getObjectsTransform();
    const auto& rightTransforms = right.getObjectsTransform();
    for (const auto& [object, leftTransform] : leftTransforms)
    {
        auto itRightTransform = rightTransforms.find(object);
        if (itRightTransform == rightTransforms.end())
            continue;

        WritePtr<AGraphNode> wObject = object.get();
        if (!wObject || !supportsInterpolatedObjectTransform(wObject->getType()))
            continue;

        const glm::dvec3 interpolatedCenter = leftTransform.getCenter() + (itRightTransform->second.getCenter() - leftTransform.getCenter()) * alpha;
        if (isPositionOnlyInterpolatedObject(wObject->getType()))
        {
            wObject->setPosition(interpolatedCenter);
            continue;
        }

        const glm::dquat interpolatedOrientation = glm::normalize(glm::slerp(leftTransform.getOrientation(), itRightTransform->second.getOrientation(), alpha));
        const glm::dvec3 interpolatedScale = leftTransform.getScale() + (itRightTransform->second.getScale() - leftTransform.getScale()) * alpha;
        wObject->setTransformationModule(TransformationModule(interpolatedCenter, interpolatedOrientation, interpolatedScale));
    }

    const auto& leftClips = left.getObjectsClippingDistances();
    const auto& rightClips = right.getObjectsClippingDistances();
    for (const auto& [object, leftClip] : leftClips)
    {
        auto itRightClip = rightClips.find(object);
        if (itRightClip == rightClips.end())
            continue;

        WritePtr<AGraphNode> wObject = object.get();
        if (!wObject || !supportsInterpolatedClippingDistances(wObject->getType()))
            continue;

        AClippingNode* clippingObject = static_cast<AClippingNode*>(wObject.operator->());
        clippingObject->setMinClipDist(leftClip.minClip + (itRightClip->second.minClip - leftClip.minClip) * static_cast<float>(alpha));
        clippingObject->setMaxClipDist(leftClip.maxClip + (itRightClip->second.maxClip - leftClip.maxClip) * static_cast<float>(alpha));
        if (supportsInterpolatedLengthThreshold(wObject->getType()))
            clippingObject->setLengthThresholdClip(leftClip.lengthThreshold + (itRightClip->second.lengthThreshold - leftClip.lengthThreshold) * static_cast<float>(alpha));
    }

    const auto& leftRamps = left.getObjectsRampDistances();
    const auto& rightRamps = right.getObjectsRampDistances();
    for (const auto& [object, leftRamp] : leftRamps)
    {
        auto itRightRamp = rightRamps.find(object);
        if (itRightRamp == rightRamps.end())
            continue;

        WritePtr<AGraphNode> wObject = object.get();
        if (!wObject || !supportsInterpolatedClippingDistances(wObject->getType()))
            continue;

        AClippingNode* clippingObject = static_cast<AClippingNode*>(wObject.operator->());
        clippingObject->setRampMin(leftRamp.minRamp + (itRightRamp->second.minRamp - leftRamp.minRamp) * static_cast<float>(alpha));
        clippingObject->setRampMax(leftRamp.maxRamp + (itRightRamp->second.maxRamp - leftRamp.maxRamp) * static_cast<float>(alpha));
        const float steps = static_cast<float>(leftRamp.stepsRamp) + static_cast<float>(itRightRamp->second.stepsRamp - leftRamp.stepsRamp) * static_cast<float>(alpha);
        clippingObject->setRampSteps(std::max(1, static_cast<int>(std::round(steps))));
        if (wObject->getType() == ElementType::Box)
            clippingObject->setRampClamped(leftRamp.rampClamped);
    }
}

Color32 interpolateColorHsvShortestPath(const Color32& start, const Color32& end, float alpha)
{
    const glm::vec3 startHsv = utils::color::rgb2hsv(color32ToNormalizedRgb(start));
    const glm::vec3 endHsv = utils::color::rgb2hsv(color32ToNormalizedRgb(end));

    float hueDelta = endHsv.x - startHsv.x;
    if (hueDelta > 0.5f)
        hueDelta -= 1.0f;
    else if (hueDelta < -0.5f)
        hueDelta += 1.0f;

    glm::vec3 hsv;
    hsv.x = startHsv.x + hueDelta * alpha;
    if (hsv.x < 0.0f)
        hsv.x += 1.0f;
    else if (hsv.x >= 1.0f)
        hsv.x -= 1.0f;
    hsv.y = startHsv.y + (endHsv.y - startHsv.y) * alpha;
    hsv.z = startHsv.z + (endHsv.z - startHsv.z) * alpha;

    const uint8_t interpolatedAlpha = static_cast<uint8_t>(std::round(
        static_cast<float>(start.a) + (static_cast<float>(end.a) - static_cast<float>(start.a)) * alpha));
    return normalizedRgbToColor32(utils::color::hsv2rgb(hsv), interpolatedAlpha);
}

}

ContextExportVideoHD::ContextExportVideoHD(const ContextId& id)
	: AContext(id)
    , m_precedentOptions()
{
}

ContextExportVideoHD::~ContextExportVideoHD()
{}

ContextState ContextExportVideoHD::start(Controller& controller)
{
    m_precedentOptions = controller.getContext().getDecimationOptions();
    m_exportState = 0;
    m_animFrame = 0;
    m_totalFrames = 0;
    m_viewpoints.clear();
    m_viewpointControlTimes.clear();
    m_sampledPositions.clear();
    m_sampledOrientations.clear();
    m_sampledTimes.clear();
    m_controlPointSampleIndices.clear();
    m_viewpointAnimationMode = ViewPointAnimationMode::ConstantIntervals;
    m_smoothViewpointTransitions = false;
    m_lastAppliedVisibilityViewpointIndex = 0;
    m_orbitalTotalAngleRad = 0.0;
    m_orbitalLastAppliedRad = 0.0;
    m_orbitalRealAppliedRad = 0.0;
    m_orbitalDirectionSign = 1.0;
    m_orbitalUsesExamine = false;
    m_orbitalVertical = false;
    DecimationOptions noDecimation = m_precedentOptions;
    noDecimation.mode = DecimationMode::None;
    controller.updateInfo(new GuiDataRenderDecimationOptions(noDecimation));
    return m_state = ContextState::waiting_for_input;
}

ContextState ContextExportVideoHD::feedMessage(IMessage* message, Controller& controller)
{
    switch (message->getType())
    {
        case IMessage::MessageType::VIDEO_EXPORT_PARAMETERS:
        {
            VideoExportParametersMessage* out = static_cast<VideoExportParametersMessage*>(message);
            m_parameters = out->m_parameters;
            m_videoFilePath = m_parameters.outputFilePath;
            m_state = m_exportPath.empty() ? ContextState::waiting_for_input : ContextState::ready_for_using;
        }
        break;
        case IMessage::MessageType::FILES:
        {
            FilesMessage* out = static_cast<FilesMessage*>(message);
            if (out->m_inputFiles.size() != 1)
                break;
            m_exportPath = out->m_inputFiles[0];
            m_state = m_parameters.animMode == VideoAnimationMode::NONE ? ContextState::waiting_for_input : ContextState::ready_for_using;
        }
        break;
        case IMessage::MessageType::GENERALMESSAGE:
        {
            GeneralMessage* out = static_cast<GeneralMessage*>(message);
            if (out->m_info == GeneralInfo::IMAGEEND && m_exportState == 2)
            {
                controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(TEXT_CONTEXT_EXPORT_VIDEO_STEPS.arg(m_animFrame).arg(m_totalFrames), m_animFrame));
                m_animFrame++;
                if (m_animFrame > m_totalFrames)
                    m_exportState = 3;
                else
                    m_exportState = 1;
                m_state = ContextState::ready_for_using;
            }
        }
        break;
    }
    return m_state;
}

ContextState ContextExportVideoHD::launch(Controller& controller)
{
    if (m_exportState == 0)
    {
        SafePtr<CameraNode> cam = controller.getGraphManager().getCameraNode();
        WritePtr<CameraNode> wCam = cam.get();
        if (!wCam)
            return abort(controller);

        wCam->setProjectionMode(ProjectionMode::Perspective);

        m_totalFrames = std::max<long>(1, static_cast<long>(m_parameters.length) * static_cast<long>(m_parameters.fps));
        m_animFrame = 1;

        if (m_parameters.animMode == VideoAnimationMode::BETWEENVIEWPOINTS)
        {
            if (!control::animation::helper::buildAnimationViewpointSequence(
                controller,
                m_parameters.viewPointAnimation,
                m_viewpoints,
                m_viewpointControlTimes,
                m_viewpointAnimationMode,
                m_smoothViewpointTransitions))
                return abort(controller);

            bool canInterpolate = true;
            ReadPtr<ViewPointNode> rReference = m_viewpoints.front().cget();
            if (!rReference)
                return abort(controller);
            for (size_t i = 1; i < m_viewpoints.size() && canInterpolate; ++i)
            {
                ReadPtr<ViewPointNode> rCandidate = m_viewpoints[i].cget();
                canInterpolate = rCandidate && control::animation::helper::areViewpointsInterpolationCompatible(*&rReference, *&rCandidate);
            }
            wCam->setViewpointRenderInterpolationEnabled(m_parameters.interpolateRenderingBetweenViewpoints && canInterpolate);

            buildSampledPlaybackPath(m_viewpoints, m_sampledPositions, m_sampledOrientations, m_controlPointSampleIndices);
            if (m_sampledPositions.size() < 2 || m_sampledOrientations.size() != m_sampledPositions.size() || m_controlPointSampleIndices.size() != m_viewpoints.size())
                return abort(controller);

            if (m_viewpointAnimationMode == ViewPointAnimationMode::PositionAsTime)
            {
                const double offset = m_viewpointControlTimes.front();
                for (double& value : m_viewpointControlTimes)
                    value -= offset;

                const double effectiveDuration = std::max(0.0, m_viewpointControlTimes.back());
                m_totalFrames = std::max<long>(1, static_cast<long>(std::ceil(effectiveDuration * static_cast<double>(std::max(1, m_parameters.fps)))));
            }
            else if (m_viewpointAnimationMode == ViewPointAnimationMode::ConstantSpeed)
            {
                const double targetDuration = std::max(0.001, static_cast<double>(m_parameters.length));
                std::vector<double> cumulative(m_sampledPositions.size(), 0.0);
                for (size_t i = 1; i < m_sampledPositions.size(); ++i)
                    cumulative[i] = cumulative[i - 1] + glm::distance(m_sampledPositions[i - 1], m_sampledPositions[i]);

                const double totalDist = cumulative.back();
                m_viewpointControlTimes.resize(m_viewpoints.size(), 0.0);
                if (totalDist <= 1e-9)
                {
                    const size_t lastIndex = m_viewpoints.size() - 1;
                    for (size_t i = 0; i <= lastIndex; ++i)
                        m_viewpointControlTimes[i] = targetDuration * static_cast<double>(i) / static_cast<double>(std::max<size_t>(1, lastIndex));
                }
                else
                {
                    for (size_t i = 0; i < m_viewpoints.size(); ++i)
                    {
                        const size_t sampleIdx = std::min(m_controlPointSampleIndices[i], cumulative.size() - 1);
                        m_viewpointControlTimes[i] = targetDuration * cumulative[sampleIdx] / totalDist;
                    }
                }
            }
            else
            {
                const double targetDuration = std::max(0.001, static_cast<double>(m_parameters.length));
                const size_t lastIndex = m_viewpoints.size() - 1;
                m_viewpointControlTimes.resize(m_viewpoints.size(), 0.0);
                for (size_t i = 0; i <= lastIndex; ++i)
                    m_viewpointControlTimes[i] = targetDuration * static_cast<double>(i) / static_cast<double>(std::max<size_t>(1, lastIndex));
            }

            const double playbackDuration = (m_viewpointAnimationMode == ViewPointAnimationMode::PositionAsTime)
                ? std::max(0.001, m_viewpointControlTimes.back())
                : std::max(0.001, static_cast<double>(m_parameters.length));
            m_sampledTimes = computeSampledTimes(
                m_viewpointAnimationMode,
                playbackDuration,
                m_viewpointControlTimes,
                m_smoothViewpointTransitions,
                m_sampledPositions,
                m_controlPointSampleIndices);
            if (m_sampledTimes.size() != m_sampledPositions.size())
                return abort(controller);

            ReadPtr<ViewPointNode> rStart = m_viewpoints.front().cget();
            if (!rStart)
                return abort(controller);

            // Snap directly to the first viewpoint (same expectation as animation Start):
            // no initial transition trajectory before frame 1 capture.
            // Video export in viewpoints mode must keep current toolbar image settings.
            wCam->snapToViewPoint(m_viewpoints.front(), true);
            m_lastAppliedVisibilityViewpointIndex = 0;
            controller.getControlListener()->notifyUIControl(new control::viewpoint::UpdateStatesFromViewpoint(m_viewpoints.front()));
        }
        else
        {
            m_viewpoints.clear();
            m_viewpointControlTimes.clear();
            m_sampledPositions.clear();
            m_sampledOrientations.clear();
            m_sampledTimes.clear();
            m_controlPointSampleIndices.clear();
            m_orbitalVertical = m_parameters.verticalOrbital;
            m_orbitalDirectionSign = -1.0; // current vertical behavior: bottom -> top
            const int maxDegrees = m_orbitalVertical ? 180 : 360;
            const double requestedAngleRad = glm::radians(static_cast<double>(std::clamp(m_parameters.orbitalDegrees, 1, maxDegrees)));
            if (m_orbitalVertical)
            {
                const double phi = wCam->getPhi();
                const double remainingUntilClamp = (m_orbitalDirectionSign >= 0.0)
                    ? std::max(0.0, 0.0 - phi)
                    : std::max(0.0, phi + M_PI);
                m_orbitalTotalAngleRad = std::min(requestedAngleRad, remainingUntilClamp);
            }
            else
            {
                m_orbitalTotalAngleRad = requestedAngleRad;
            }
            m_orbitalLastAppliedRad = 0.0;
            m_orbitalRealAppliedRad = 0.0;
            m_orbitalUsesExamine = wCam->isExamineActive();
        }

        m_frameDigits = std::max<uint8_t>(1, static_cast<uint8_t>(std::log10(std::max<long>(1, m_totalFrames)) + 1));
        controller.updateInfo(new GuiDataProcessingSplashScreenStart(m_totalFrames, TEXT_CONTEXT_EXPORT_VIDEO, TEXT_CONTEXT_EXPORT_VIDEO_STEPS.arg(0).arg(m_totalFrames)));
        m_tpStart = std::chrono::steady_clock::now();

        m_exportState = 1;
        return m_state = ContextState::ready_for_using;
    }

    if (m_exportState == 1)
    {
        SafePtr<CameraNode> cam = controller.getGraphManager().getCameraNode();
        WritePtr<CameraNode> wCam = cam.get();
        if (!wCam)
            return abort(controller);

        if (m_animFrame > m_totalFrames)
        {
            m_exportState = 3;
            return m_state = ContextState::ready_for_using;
        }

        if (m_animFrame > 1 && m_parameters.animMode == VideoAnimationMode::BETWEENVIEWPOINTS)
        {
            const double t = std::clamp(static_cast<double>(m_animFrame - 1) / static_cast<double>(std::max(1, m_parameters.fps)), 0.0, m_sampledTimes.back());

            auto sampledUpper = std::upper_bound(m_sampledTimes.begin(), m_sampledTimes.end(), t);
            size_t sampledRightIndex = static_cast<size_t>(std::distance(m_sampledTimes.begin(), sampledUpper));
            if (sampledRightIndex == 0)
                sampledRightIndex = 1;
            if (sampledRightIndex >= m_sampledTimes.size())
                sampledRightIndex = m_sampledTimes.size() - 1;
            const size_t sampledLeftIndex = sampledRightIndex - 1;
            const double leftSampleTime = m_sampledTimes[sampledLeftIndex];
            const double rightSampleTime = m_sampledTimes[sampledRightIndex];
            const double safeSampleDuration = std::max(1e-9, rightSampleTime - leftSampleTime);
            const double sampleAlpha = std::clamp((t - leftSampleTime) / safeSampleDuration, 0.0, 1.0);
            wCam->setPosition(m_sampledPositions[sampledLeftIndex] * (1.0 - sampleAlpha) + m_sampledPositions[sampledRightIndex] * sampleAlpha);
            wCam->setRotation(glm::normalize(slerpShortestPath(m_sampledOrientations[sampledLeftIndex], m_sampledOrientations[sampledRightIndex], sampleAlpha)));

            auto upper = std::upper_bound(m_viewpointControlTimes.begin(), m_viewpointControlTimes.end(), t);
            size_t rightIndex = static_cast<size_t>(std::distance(m_viewpointControlTimes.begin(), upper));
            if (rightIndex == 0)
                rightIndex = 1;
            if (rightIndex >= m_viewpointControlTimes.size())
                rightIndex = m_viewpointControlTimes.size() - 1;
            const size_t leftIndex = rightIndex - 1;
            size_t activeViewpointIndex = leftIndex;
            if (rightIndex < m_viewpointControlTimes.size() && t >= m_viewpointControlTimes[rightIndex])
                activeViewpointIndex = rightIndex;
            activeViewpointIndex = std::min(activeViewpointIndex, m_viewpoints.size() - 1);

            if (activeViewpointIndex != m_lastAppliedVisibilityViewpointIndex)
            {
                m_lastAppliedVisibilityViewpointIndex = activeViewpointIndex;
                controller.getControlListener()->notifyUIControl(new control::viewpoint::UpdateStatesFromViewpoint(m_viewpoints[activeViewpointIndex]));
            }

            ReadPtr<ViewPointNode> left = m_viewpoints[leftIndex].cget();
            ReadPtr<ViewPointNode> right = m_viewpoints[rightIndex].cget();
            if (!left || !right)
                return abort(controller);

            const double leftTime = m_viewpointControlTimes[leftIndex];
            const double rightTime = m_viewpointControlTimes[rightIndex];
            const double safeDuration = std::max(1e-9, rightTime - leftTime);
            const double linearAlpha = std::clamp((t - leftTime) / safeDuration, 0.0, 1.0);
            const bool useSmoothTransition =
                m_smoothViewpointTransitions &&
                (m_viewpointAnimationMode == ViewPointAnimationMode::PositionAsTime ||
                 m_viewpointAnimationMode == ViewPointAnimationMode::ConstantIntervals);
            const double alpha = useSmoothTransition ? smoothstep01(linearAlpha) : linearAlpha;

            if (m_parameters.interpolateRenderingBetweenViewpoints && control::animation::helper::areViewpointsInterpolationCompatible(*&left, *&right))
            {
                wCam->m_transparency = left->m_transparency * static_cast<float>(1.0 - alpha) + right->m_transparency * static_cast<float>(alpha);
                wCam->m_postRenderingNormals.normalStrength = left->m_postRenderingNormals.normalStrength * static_cast<float>(1.0 - alpha) + right->m_postRenderingNormals.normalStrength * static_cast<float>(alpha);
                wCam->m_postRenderingNormals.gloss = left->m_postRenderingNormals.gloss * static_cast<float>(1.0 - alpha) + right->m_postRenderingNormals.gloss * static_cast<float>(alpha);
                wCam->m_contrast = left->m_contrast * static_cast<float>(1.0 - alpha) + right->m_contrast * static_cast<float>(alpha);
                wCam->m_brightness = left->m_brightness * static_cast<float>(1.0 - alpha) + right->m_brightness * static_cast<float>(alpha);
                wCam->m_luminance = left->m_luminance * static_cast<float>(1.0 - alpha) + right->m_luminance * static_cast<float>(alpha);
                wCam->m_saturation = left->m_saturation * static_cast<float>(1.0 - alpha) + right->m_saturation * static_cast<float>(alpha);
                wCam->m_hue = left->m_hue * static_cast<float>(1.0 - alpha) + right->m_hue * static_cast<float>(alpha);
                wCam->m_alphaObject = left->m_alphaObject * static_cast<float>(1.0 - alpha) + right->m_alphaObject * static_cast<float>(alpha);
                wCam->setFovy(left->getFovy() * (1.0 - alpha) + right->getFovy() * alpha);

                if (left->m_mode == UiRenderMode::Scans_Color || left->m_mode == UiRenderMode::Clusters_Color)
                {
                    const std::unordered_set<SafePtr<AGraphNode>>& leftVisible = left->getVisibleObjects();
                    const std::unordered_set<SafePtr<AGraphNode>>& rightVisible = right->getVisibleObjects();
                    const std::unordered_map<SafePtr<AGraphNode>, Color32>& leftColors = left->getScanClusterColors();
                    const std::unordered_map<SafePtr<AGraphNode>, Color32>& rightColors = right->getScanClusterColors();
                    const float alphaF = static_cast<float>(alpha);

                    for (const auto& [object, leftColor] : leftColors)
                    {
                        if (leftVisible.find(object) == leftVisible.end() || rightVisible.find(object) == rightVisible.end())
                            continue;

                        auto itRightColor = rightColors.find(object);
                        if (itRightColor == rightColors.end())
                            continue;

                        WritePtr<AGraphNode> wObject = object.get();
                        if (!wObject)
                            continue;

                        wObject->setColor(interpolateColorHsvShortestPath(leftColor, itRightColor->second, alphaF));
                    }
                }

                interpolateAndApplyObjects(*&left, *&right, alpha);
            }
        }
        else if (m_animFrame > 1)
        {
            const double target = m_orbitalTotalAngleRad * static_cast<double>(m_animFrame - 1) / static_cast<double>(std::max<long>(1, m_totalFrames));
            const double delta = target - m_orbitalLastAppliedRad;
            if (delta > 0.0)
            {
                if (m_orbitalVertical)
                {
                    const double signedDelta = m_orbitalDirectionSign * delta;
                    const double phiBefore = wCam->getPhi();
                    if (m_orbitalUsesExamine)
                        wCam->moveAroundExamine(0.0, 0.0, signedDelta);
                    else
                        wCam->pitch(signedDelta);
                    const double phiAfter = wCam->getPhi();
                    m_orbitalRealAppliedRad += std::abs(phiAfter - phiBefore);
                }
                else
                {
                    if (m_orbitalUsesExamine)
                        wCam->moveAroundExamine(0.0, delta, 0.0);
                    else
                        wCam->yaw(delta);
                    m_orbitalRealAppliedRad = target;
                }
                m_orbitalLastAppliedRad = target;
            }

            if (m_orbitalRealAppliedRad + 1e-9 >= m_orbitalTotalAngleRad)
            {
                m_animFrame = m_totalFrames + 1;
            }
        }

        controller.updateInfo(new GuiDataCallImage(m_parameters.hdImage, getNextFramePath()));
        m_exportState = 2;
        return m_state = ContextState::waiting_for_input;
    }

    if (m_exportState == 2)
    {
        return m_state = ContextState::waiting_for_input;
    }

    if (m_exportState == 3)
    {
        if (!encodeVideo())
            return abort(controller);

        if(m_parameters.openFolderAfterExport)
        {
            std::filesystem::path folderToOpen = m_exportPath;
            if (m_parameters.outputType == VideoExportOutputType::MP4 && !m_videoFilePath.empty())
                folderToOpen = m_videoFilePath;
            controller.updateInfo(new GuiDataOpenInExplorer(folderToOpen));
        }
        return validate(controller);
    }

    return abort(controller);
}

ContextState ContextExportVideoHD::abort(Controller& controller)
{
    controller.updateInfo(new GuiDataRenderDecimationOptions(m_precedentOptions));

    encodeVideo();

    std::chrono::steady_clock::time_point tpEnd = std::chrono::steady_clock::now();
    double totalDurationSeconds = std::chrono::duration<double>(tpEnd - m_tpStart).count();

    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(TEXT_CONTEXT_EXPORT_VIDEO_TIME.arg(QString::fromStdString(Utils::roundFloat(totalDurationSeconds)))));
    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_CONTEXT_EXPORT_VIDEO_FAIL));

    return AContext::validate(controller);
}

ContextState ContextExportVideoHD::validate(Controller& controller)
{
    controller.updateInfo(new GuiDataRenderDecimationOptions(m_precedentOptions));

    std::chrono::steady_clock::time_point tpEnd = std::chrono::steady_clock::now();
    double totalDurationSeconds = std::chrono::duration<double>(tpEnd - m_tpStart).count();

    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(TEXT_CONTEXT_EXPORT_VIDEO_TIME.arg(QString::fromStdString(Utils::roundFloat(totalDurationSeconds)))));
    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_CONTEXT_EXPORT_VIDEO_DONE));

    return AContext::validate(controller);
}

bool ContextExportVideoHD::canAutoRelaunch() const
{
	return false;
}

bool ContextExportVideoHD::encodeVideo()
{
    if (m_parameters.outputType != VideoExportOutputType::MP4)
        return true;

    long producedFrames = std::max<long>(0, m_animFrame - 1);
    if (producedFrames <= 0)
        return false;

    if (m_videoFilePath.empty())
    {
        m_videoFilePath = m_exportPath;
        m_videoFilePath.replace_extension(".mp4");
    }

    std::wstring baseName = m_exportPath.stem().wstring();
    if (baseName.empty())
        baseName = L"video";

    auto firstFrame = firstFrameFilepath();
    if (!firstFrame.has_value())
        return false;

    std::wstring extension = firstFrame->extension().wstring();
    if (extension.empty())
        return false;

    uint8_t padding = std::max<uint8_t>(1, m_frameDigits);
    std::filesystem::path patternPath = m_exportPath / (baseName + L"_%0" + std::to_wstring(padding) + L"d" + extension);

    QProcess ffmpeg;
    QStringList args;
    args << "-y"
         << "-v" << "error"
         << "-stats"
         << "-framerate" << QString::number(m_parameters.fps)
         << "-start_number" << "1"
         << "-i" << QString::fromStdWString(patternPath.wstring())
         << "-frames:v" << QString::number(producedFrames)
         << "-c:v" << "libx265"
         << "-b:v" << QString::number(m_parameters.bitrateKbps) + "k"
         << "-pix_fmt" << "yuv420p"
         << QString::fromStdWString(m_videoFilePath.wstring());

    ffmpeg.start(resolveFfmpegExecutable(), args);
    if (!ffmpeg.waitForStarted(3000))
    {
        ffmpeg.kill();
        cleanupFrames();
        return false;
    }
    if (!ffmpeg.waitForFinished(-1))
    {
        ffmpeg.kill();
        cleanupFrames();
        return false;
    }
    bool success = ffmpeg.exitStatus() == QProcess::NormalExit && ffmpeg.exitCode() == 0;
    cleanupFrames();
    return success;
}

std::optional<std::filesystem::path> ContextExportVideoHD::firstFrameFilepath() const
{
    if (m_exportPath.empty())
        return std::nullopt;

    uint8_t padding = std::max<uint8_t>(1, m_frameDigits);
    std::filesystem::path baseFramePath = m_exportPath / (m_exportPath.stem().wstring() + L"_" + Utils::wCompleteWithZeros(1, padding));

    const std::wstring extensions[] = { L".PNG", L".JPG", L".JPEG", L".TIFF", L".BMP" };
    for (const auto& ext : extensions)
    {
        std::filesystem::path candidate = baseFramePath;
        candidate += ext;
        if (std::filesystem::exists(candidate))
            return candidate;
    }

    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(m_exportPath))
        {
            if (entry.is_regular_file())
                return entry.path();
        }
    }
    catch (const std::exception&)
    {
    }

    return std::nullopt;
}

void ContextExportVideoHD::cleanupFrames()
{
    if (m_parameters.outputType != VideoExportOutputType::MP4)
        return;

    try
    {
        if (!m_exportPath.empty() && std::filesystem::exists(m_exportPath))
            std::filesystem::remove_all(m_exportPath);
    }
    catch (const std::exception&)
    {
    }
}

std::filesystem::path ContextExportVideoHD::getNextFramePath()
{
    std::filesystem::path nextPath = m_exportPath;
    std::wstring filename = m_exportPath.stem().wstring();
    uint8_t padding = m_frameDigits ? m_frameDigits : (uint8_t)(std::log10(std::max<long>(1, m_totalFrames)) + 1);
    nextPath = nextPath / (filename + L"_" + Utils::wCompleteWithZeros(m_animFrame, padding));
    try
    {
        std::filesystem::create_directories(nextPath.parent_path());
    }
    catch (std::exception e)
    {
        assert(false);
    }
    return nextPath;
}

ContextType ContextExportVideoHD::getType() const
{
	return ContextType::exportVideoHD;
}
