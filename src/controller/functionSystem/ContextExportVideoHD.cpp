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
        m_frameDigits = std::max<uint8_t>(1, static_cast<uint8_t>(std::log10(std::max<long>(1, m_totalFrames)) + 1));

        controller.updateInfo(new GuiDataProcessingSplashScreenStart(m_totalFrames, TEXT_CONTEXT_EXPORT_VIDEO, TEXT_CONTEXT_EXPORT_VIDEO_STEPS.arg(0).arg(m_totalFrames)));
        m_tpStart = std::chrono::steady_clock::now();

        if (m_parameters.animMode == VideoAnimationMode::BETWEENVIEWPOINTS)
        {
            ViewPointAnimationMode mode = ViewPointAnimationMode::ConstantIntervals;
            if (!control::animation::helper::buildAnimationViewpointSequence(controller, m_parameters.viewPointAnimation, m_viewpoints, m_viewpointControlTimes, mode))
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

            if (mode == ViewPointAnimationMode::PositionAsTime)
            {
                const double offset = m_viewpointControlTimes.front();
                for (double& value : m_viewpointControlTimes)
                    value -= offset;

                const double effectiveDuration = std::max(0.0, m_viewpointControlTimes.back());
                m_totalFrames = std::max<long>(1, static_cast<long>(std::ceil(effectiveDuration * static_cast<double>(std::max(1, m_parameters.fps)))));
            }
            else if (mode == ViewPointAnimationMode::ConstantSpeed)
            {
                std::vector<double> cumulative(m_viewpoints.size(), 0.0);
                double totalDist = 0.0;
                for (size_t i = 1; i < m_viewpoints.size(); ++i)
                {
                    ReadPtr<ViewPointNode> prev = m_viewpoints[i - 1].cget();
                    ReadPtr<ViewPointNode> curr = m_viewpoints[i].cget();
                    if (!prev || !curr)
                        return abort(controller);
                    totalDist += glm::distance(prev->getCenter(), curr->getCenter());
                    cumulative[i] = totalDist;
                }
                const double targetDuration = std::max(0.001, static_cast<double>(m_parameters.length));
                if (totalDist <= 1e-9)
                {
                    for (size_t i = 0; i < cumulative.size(); ++i)
                        cumulative[i] = targetDuration * static_cast<double>(i) / static_cast<double>(std::max<size_t>(1, cumulative.size() - 1));
                }
                else
                {
                    for (double& value : cumulative)
                        value = targetDuration * value / totalDist;
                }
                m_viewpointControlTimes = cumulative;
            }
            else
            {
                const double targetDuration = std::max(0.001, static_cast<double>(m_parameters.length));
                const size_t lastIndex = m_viewpoints.size() - 1;
                m_viewpointControlTimes.resize(m_viewpoints.size(), 0.0);
                for (size_t i = 0; i <= lastIndex; ++i)
                    m_viewpointControlTimes[i] = targetDuration * static_cast<double>(i) / static_cast<double>(std::max<size_t>(1, lastIndex));
            }

            ReadPtr<ViewPointNode> rStart = m_viewpoints.front().cget();
            if (!rStart)
                return abort(controller);

            // Snap directly to the first viewpoint (same expectation as animation Start):
            // no initial transition trajectory before frame 1 capture.
            wCam->snapToViewPoint(m_viewpoints.front());
            m_lastAppliedVisibilityViewpointIndex = 0;
            controller.getControlListener()->notifyUIControl(new control::viewpoint::UpdateStatesFromViewpoint(m_viewpoints.front()));
        }
        else
        {
            m_viewpoints.clear();
            m_viewpointControlTimes.clear();
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
            const double t = std::clamp(static_cast<double>(m_animFrame - 1) / static_cast<double>(std::max(1, m_parameters.fps)), 0.0, m_viewpointControlTimes.back());
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
            const double alpha = std::clamp((t - leftTime) / safeDuration, 0.0, 1.0);

            wCam->setPosition(left->getCenter() * (1.0 - alpha) + right->getCenter() * alpha);
            wCam->setRotation(glm::normalize(glm::slerp(left->getOrientation(), right->getOrientation(), alpha)));

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
