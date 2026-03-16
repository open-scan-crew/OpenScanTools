#include "controller/functionSystem/ContextExportVideoHD.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/controls/AnimationHelper.h"

#include "models/graph/GraphManager.h"
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
    m_orbitalTotalAngleRad = 0.0;
    m_orbitalLastAppliedRad = 0.0;
    m_orbitalUsesExamine = false;
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
            wCam->moveToData(m_viewpoints.front());
        }
        else
        {
            m_viewpoints.clear();
            m_viewpointControlTimes.clear();
            m_orbitalTotalAngleRad = glm::radians(static_cast<double>(std::clamp(m_parameters.orbitalDegrees, 1, 360)));
            m_orbitalLastAppliedRad = 0.0;
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
            }
        }
        else if (m_animFrame > 1)
        {
            const double target = m_orbitalTotalAngleRad * static_cast<double>(m_animFrame - 1) / static_cast<double>(std::max<long>(1, m_totalFrames));
            const double delta = target - m_orbitalLastAppliedRad;
            if (delta > 0.0)
            {
                if (m_orbitalUsesExamine)
                    wCam->moveAroundExamine(0.0, delta, 0.0);
                else
                    wCam->yaw(delta);
                m_orbitalLastAppliedRad = target;
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
