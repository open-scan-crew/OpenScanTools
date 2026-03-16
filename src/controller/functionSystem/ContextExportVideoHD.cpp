#include "controller/functionSystem/ContextExportVideoHD.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"

#include "models/graph/GraphManager.h"
#include "models/graph/CameraNode.h"
#include "models/graph/ViewPointNode.h"

#include "controller/messages/FilesMessage.h"
#include "controller/messages/GeneralMessage.h"
#include "controller/messages/VideoExportParametersMessage.h"
#include "controller/controls/ControlAnimation.h"

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
    m_exportState = 0;
    m_animFrame = 0;
    m_totalFrames = 0;
    m_orbitalStepRad = 0.0;

    m_precedentOptions = controller.getContext().getDecimationOptions();
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
    if (m_parameters.animMode == VideoAnimationMode::BETWEENVIEWPOINTS && !m_parameters.viewpointAnimationId.isValid())
        return abort(controller);

    // Start and prepare trajectory
    if (m_exportState == 0)
    {
        control::animation::PreparedViewpointsAnimationPlayback preparedViewpoints;
        if (m_parameters.animMode == VideoAnimationMode::BETWEENVIEWPOINTS)
        {
            const bool prepared = control::animation::buildViewpointsAnimationPlayback(
                controller,
                m_parameters.viewpointAnimationId,
                m_parameters.length,
                m_parameters.interpolateRenderingBetweenViewpoints,
                preparedViewpoints,
                true);
            if (!prepared)
                return abort(controller);
        }

        SafePtr<CameraNode> cam = controller.getGraphManager().getCameraNode();
        WritePtr<CameraNode> wCam = cam.get();
        if (!wCam)
            return abort(controller);

        wCam->setProjectionMode(ProjectionMode::Perspective);
        m_totalFrames = 1;
        if (m_parameters.animMode == VideoAnimationMode::BETWEENVIEWPOINTS)
        {
            wCam->cleanAnimation();
            wCam->setLoop(false);
            wCam->setSpeed(1);
            wCam->setViewpointRenderInterpolationEnabled(preparedViewpoints.enableInterpolation);
            wCam->setAnimationTiming(preparedViewpoints.mode, static_cast<double>(m_parameters.length), preparedViewpoints.controlTimes, preparedViewpoints.smoothTransitions);
            for (const SafePtr<ViewPointNode>& viewpoint : preparedViewpoints.viewpoints)
                wCam->AddViewPoint(viewpoint);

            if (!wCam->startAnimation(true, static_cast<uint64_t>(std::max(1, m_parameters.fps))))
                return abort(controller);

            const double durationSeconds = std::max(0.0, preparedViewpoints.metrics.effectiveDurationSeconds);
            m_totalFrames = std::max<long>(1, static_cast<long>(std::llround(durationSeconds * static_cast<double>(std::max(1, m_parameters.fps)))) + 1);
        }
        else
        {
            const double durationSeconds = std::max(0.0, static_cast<double>(m_parameters.length));
            m_totalFrames = std::max<long>(1, static_cast<long>(std::llround(durationSeconds * static_cast<double>(std::max(1, m_parameters.fps)))) + 1);
            const long moveSteps = std::max<long>(1, m_totalFrames - 1);
            m_orbitalStepRad = glm::radians(static_cast<double>(std::clamp(m_parameters.orbitalDegrees, 1, 360))) / static_cast<double>(moveSteps);
        }

        m_animFrame = 1;
        m_frameDigits = std::max<uint8_t>(1, static_cast<uint8_t>(std::log10(std::max<long>(1, m_totalFrames)) + 1));

        controller.updateInfo(new GuiDataProcessingSplashScreenStart(m_totalFrames, TEXT_CONTEXT_EXPORT_VIDEO, TEXT_CONTEXT_EXPORT_VIDEO_STEPS.arg(0).arg(m_totalFrames)));
        m_tpStart = std::chrono::steady_clock::now();

        GUI_LOG << "[VIDEO_EXPORT] start export"
            << " mode=" << static_cast<int>(m_parameters.animMode)
            << " fps=" << m_parameters.fps
            << " totalFrames=" << m_totalFrames
            << " orbitalDegrees=" << m_parameters.orbitalDegrees
            << LOGENDL;

        controller.updateInfo(new GuiDataCallImage(m_parameters.hdImage, getNextFramePath()));
        m_exportState = 2;

        return m_state = ContextState::waiting_for_input;
    }

    // Move and image
    if (m_exportState == 2)
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

        switch (m_parameters.animMode)
        {
            case VideoAnimationMode::BETWEENVIEWPOINTS:
            {
                wCam->updateAnimation();
            }
            break;
            case VideoAnimationMode::ORBITAL:
            {
                if (wCam->isExamineActive())
                    wCam->moveAroundExamine(0.0, m_orbitalStepRad, 0.0);
                else
                    wCam->yaw(m_orbitalStepRad);
            }
            break;
        }

        GUI_LOG << "[VIDEO_EXPORT] frame=" << m_animFrame << "/" << m_totalFrames << LOGENDL;

        controller.updateInfo(new GuiDataCallImage(m_parameters.hdImage, getNextFramePath()));
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
