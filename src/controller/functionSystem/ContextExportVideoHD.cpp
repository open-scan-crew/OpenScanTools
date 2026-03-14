#include "controller/functionSystem/ContextExportVideoHD.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"

#include "models/graph/GraphManager.h"
#include "models/graph/CameraNode.h"
#include "models/graph/ViewPointNode.h"

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
#include <limits>
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
    DecimationOptions noDecimation = m_precedentOptions;
    noDecimation.mode = DecimationMode::None;
    controller.updateInfo(new GuiDataRenderDecimationOptions(noDecimation));
    m_usePreparedCameraAnimation = false;
    m_preparedAnimation = PreparedAnimation();
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
            if (out->m_info == GeneralInfo::ANIMATIONEND && m_exportState == 1)
            {
                m_state = ContextState::ready_for_using;
            }
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
    if (m_parameters.animMode == VideoAnimationMode::BETWEENVIEWPOINTS && !m_parameters.animationId.isValid() && m_parameters.start == m_parameters.finish)
        return abort(controller);

    //Start - Move to start position
    if (m_exportState == 0)
    {
        m_usePreparedCameraAnimation = false;
        std::optional<PreparedAnimation> preparedAnimation;
        glm::dvec3 firstCenter(0.0);
        double firstTheta = 0.0;
        double firstPhi = 0.0;

        if (m_parameters.animMode == VideoAnimationMode::BETWEENVIEWPOINTS)
        {
            preparedAnimation = prepareViewpointAnimation(controller);
            if (!preparedAnimation.has_value())
            {
                controller.updateInfo(new GuiDataWarning(TEXT_CONTEXT_ANIMATION_NEED_TWO_VIEWPOINTS));
                return abort(controller);
            }

            ReadPtr<ViewPointNode> rFirstViewpoint = preparedAnimation->viewpoints.front().cget();
            if (!rFirstViewpoint)
                return abort(controller);

            const glm::dvec3 firstLookDir = glm::dvec4(0.0, 0.0, 1.0, 1.0) * rFirstViewpoint->getInverseTransformation();
            if (!(firstLookDir.x == 0.0 && firstLookDir.y == 0.0))
                firstTheta = atan2(-firstLookDir.x, firstLookDir.y);
            const double firstNormXY = sqrt(firstLookDir.x * firstLookDir.x + firstLookDir.y * firstLookDir.y);
            firstPhi = atan2(-firstNormXY, firstLookDir.z);
            firstCenter = rFirstViewpoint->getCenter();
        }

        SafePtr<CameraNode> cam = controller.getGraphManager().getCameraNode();
        WritePtr<CameraNode> wCam = cam.get();
        if (!wCam)
            return abort(controller);

        wCam->setProjectionMode(ProjectionMode::Perspective);
        m_exportState = 1;

        if (preparedAnimation.has_value())
        {
            m_preparedAnimation = preparedAnimation.value();
            m_usePreparedCameraAnimation = true;

            wCam->cleanAnimation();
            wCam->setLoop(false);
            wCam->setSpeed(1);
            wCam->setAnimationTiming(m_preparedAnimation.mode, m_preparedAnimation.durationSeconds, m_preparedAnimation.controlTimes);
            for (const SafePtr<ViewPointNode>& viewpoint : m_preparedAnimation.viewpoints)
                wCam->AddViewPoint(viewpoint);

            // Force a synchronous jump to the first viewpoint before frame generation.
            // Relying on moveToData() + asynchronous ANIMATIONEND can stall the export state machine.
            const glm::dvec3 deltaToFirst = firstCenter - wCam->getCenter();
            wCam->moveGlobal(deltaToFirst.x, deltaToFirst.y, deltaToFirst.z);
            wCam->setThetaAndPhi(firstTheta, firstPhi);
        }

    }

    //Calcul camera move delta
    if (m_exportState == 1)
    {
        SafePtr<CameraNode> cam = controller.getGraphManager().getCameraNode();
        WritePtr<CameraNode> wCam = cam.get();
        if (!wCam)
            return abort(controller);
        if (m_parameters.animMode == VideoAnimationMode::BETWEENVIEWPOINTS && m_usePreparedCameraAnimation)
            m_totalFrames = std::max<long>(1, static_cast<long>(std::llround(m_preparedAnimation.durationSeconds * static_cast<double>(m_parameters.fps))));
        else
            m_totalFrames = (long)m_parameters.length * (long)m_parameters.fps;
        m_animFrame = 1;
        m_frameDigits = std::max<uint8_t>(1, (uint8_t)(std::log10(std::max<long>(1, m_totalFrames)) + 1));

        controller.updateInfo(new GuiDataProcessingSplashScreenStart(m_totalFrames, TEXT_CONTEXT_EXPORT_VIDEO, TEXT_CONTEXT_EXPORT_VIDEO_STEPS.arg(0).arg(m_totalFrames)));
        m_tpStart = std::chrono::steady_clock::now();

        if (m_parameters.animMode == VideoAnimationMode::BETWEENVIEWPOINTS)
        {
            if (!m_usePreparedCameraAnimation)
            {
                ReadPtr<ViewPointNode> rStart;
                ReadPtr<ViewPointNode> rFinish;
                multi_cget(m_parameters.start, m_parameters.finish, rStart, rFinish);
                if (!rStart || !rFinish)
                    return abort(controller);

                glm::dvec3 finishLookDir = glm::dvec4(0.0, 0.0, 1.0, 1.0) * rFinish->getInverseTransformation();

                double endTheta;
                if (finishLookDir.x == 0.0 && finishLookDir.y == 0.0)
                    endTheta = 0.;
                else
                    endTheta = atan2(-finishLookDir.x, finishLookDir.y);

                double normXY = sqrt(finishLookDir.x * finishLookDir.x + finishLookDir.y * finishLookDir.y);
                double endPhi = atan2(-normXY, finishLookDir.z);

                CameraNode::modulo2Pi(wCam->getTheta(), endTheta);

                m_addPosition = (rFinish->getCenter() - rStart->getCenter()) / (double)m_totalFrames;
                m_addTheta = (endTheta - wCam->getTheta()) / m_totalFrames;
                m_addPhi = (endPhi - wCam->getPhi()) / m_totalFrames;

                if (rStart->m_blendMode == rFinish->m_blendMode &&
                    rStart->m_postRenderingNormals.show == rFinish->m_postRenderingNormals.show &&
                    rStart->m_postRenderingNormals.blendColor == rFinish->m_postRenderingNormals.blendColor &&
                    rStart->m_postRenderingNormals.inverseTone == rFinish->m_postRenderingNormals.inverseTone &&
                    rStart->m_mode == rFinish->m_mode &&
                    rStart->m_negativeEffect == rFinish->m_negativeEffect &&
                    rStart->m_reduceFlash == rFinish->m_reduceFlash &&
                    rStart->m_flashAdvanced == rFinish->m_flashAdvanced &&
                    rStart->m_flashControl == rFinish->m_flashControl
                    )
                {
                    m_addTransp = (rFinish->m_transparency - rStart->m_transparency) / m_totalFrames;

                    m_addNStren = (rFinish->m_postRenderingNormals.normalStrength - rStart->m_postRenderingNormals.normalStrength) / m_totalFrames;
                    m_addNGloss = (rFinish->m_postRenderingNormals.gloss - rStart->m_postRenderingNormals.gloss) / m_totalFrames;

                    m_addHue = (rFinish->m_hue - rStart->m_hue) / m_totalFrames;

                    m_addBright = (rFinish->m_brightness - rStart->m_brightness) / m_totalFrames;
                    m_addSatur = (rFinish->m_saturation - rStart->m_saturation) / m_totalFrames;
                    m_addLumi = (rFinish->m_luminance - rStart->m_luminance) / m_totalFrames;
                    m_addContr = (rFinish->m_contrast - rStart->m_contrast) / m_totalFrames;
                    m_addAlpha = (rFinish->m_alphaObject - rStart->m_alphaObject) / m_totalFrames;

                    m_addFovy = (rFinish->getFovy() - rStart->getFovy()) / m_totalFrames;
                }
            }
            else
            {
                if (!wCam->startAnimation(true, static_cast<uint64_t>(std::max(1, m_parameters.fps))))
                    return abort(controller);

                if (m_preparedAnimation.viewpoints.size() == 2 && m_parameters.interpolateRenderingBetweenViewpoints)
                {
                    ReadPtr<ViewPointNode> rStart;
                    ReadPtr<ViewPointNode> rFinish;
                    multi_cget(m_preparedAnimation.viewpoints.front(), m_preparedAnimation.viewpoints.back(), rStart, rFinish);
                    if (rStart && rFinish &&
                        rStart->m_blendMode == rFinish->m_blendMode &&
                        rStart->m_postRenderingNormals.show == rFinish->m_postRenderingNormals.show &&
                        rStart->m_postRenderingNormals.blendColor == rFinish->m_postRenderingNormals.blendColor &&
                        rStart->m_postRenderingNormals.inverseTone == rFinish->m_postRenderingNormals.inverseTone &&
                        rStart->m_mode == rFinish->m_mode &&
                        rStart->m_negativeEffect == rFinish->m_negativeEffect &&
                        rStart->m_reduceFlash == rFinish->m_reduceFlash &&
                        rStart->m_flashAdvanced == rFinish->m_flashAdvanced &&
                        rStart->m_flashControl == rFinish->m_flashControl)
                    {
                        m_addTransp = (rFinish->m_transparency - rStart->m_transparency) / m_totalFrames;
                        m_addNStren = (rFinish->m_postRenderingNormals.normalStrength - rStart->m_postRenderingNormals.normalStrength) / m_totalFrames;
                        m_addNGloss = (rFinish->m_postRenderingNormals.gloss - rStart->m_postRenderingNormals.gloss) / m_totalFrames;
                        m_addHue = (rFinish->m_hue - rStart->m_hue) / m_totalFrames;
                        m_addBright = (rFinish->m_brightness - rStart->m_brightness) / m_totalFrames;
                        m_addSatur = (rFinish->m_saturation - rStart->m_saturation) / m_totalFrames;
                        m_addLumi = (rFinish->m_luminance - rStart->m_luminance) / m_totalFrames;
                        m_addContr = (rFinish->m_contrast - rStart->m_contrast) / m_totalFrames;
                        m_addAlpha = (rFinish->m_alphaObject - rStart->m_alphaObject) / m_totalFrames;
                        m_addFovy = (rFinish->getFovy() - rStart->getFovy()) / m_totalFrames;
                    }
                }
            }
        }
        else
            m_addTheta = glm::radians(static_cast<double>(std::clamp(m_parameters.orbitalDegrees, 1, 360))) / m_totalFrames;

        controller.updateInfo(new GuiDataCallImage(m_parameters.hdImage, getNextFramePath()));
        m_exportState = 2;

        return m_state = ContextState::waiting_for_input;
    }

    //Mouvement and Image
    if (m_exportState == 2)
    {

        SafePtr<CameraNode> cam = controller.getGraphManager().getCameraNode();
        WritePtr<CameraNode> wCam = cam.get();
        if (!wCam)
            return abort(controller);

        switch (m_parameters.animMode)
        {
            case VideoAnimationMode::BETWEENVIEWPOINTS:
            {
                if (m_usePreparedCameraAnimation)
                    wCam->updateAnimation();
                else
                {
                    wCam->addGlobalTranslation(m_addPosition);
                    wCam->yaw(m_addTheta);
                    wCam->pitch(m_addPhi);
                }

                if (m_parameters.interpolateRenderingBetweenViewpoints && (!m_usePreparedCameraAnimation || m_preparedAnimation.viewpoints.size() == 2))
                {
                    wCam->m_transparency += m_addTransp;                    
                    wCam->m_postRenderingNormals.normalStrength += m_addNStren;
                    wCam->m_postRenderingNormals.gloss += m_addNGloss;
                    wCam->m_contrast += m_addContr;
                    wCam->m_brightness += m_addBright;
                    wCam->m_luminance += m_addLumi;
                    wCam->m_saturation += m_addSatur;
                    wCam->m_hue += m_addHue;
                    wCam->m_alphaObject += m_addAlpha;
                    wCam->setFovy(wCam->getFovy() + m_addFovy);
                }
            }
            break;
            case VideoAnimationMode::ORBITAL:
            {
                if (wCam->isExamineActive())
                    wCam->moveAroundExamine(0.0, m_addTheta, 0.0);
                else
                    wCam->yaw(m_addTheta);
            }
            break;
        }

        controller.updateInfo(new GuiDataCallImage(m_parameters.hdImage, getNextFramePath()));
        return m_state = ContextState::waiting_for_input;
    }

    if (m_exportState == 3)
    {
        if (m_usePreparedCameraAnimation)
        {
            SafePtr<CameraNode> cam = controller.getGraphManager().getCameraNode();
            WritePtr<CameraNode> wCam = cam.get();
            if (wCam && wCam->isAnimated())
                wCam->endAnimation();
        }

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
    if (m_usePreparedCameraAnimation)
    {
        SafePtr<CameraNode> cam = controller.getGraphManager().getCameraNode();
        WritePtr<CameraNode> wCam = cam.get();
        if (wCam && wCam->isAnimated())
            wCam->endAnimation();
    }

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
    if (m_usePreparedCameraAnimation)
    {
        SafePtr<CameraNode> cam = controller.getGraphManager().getCameraNode();
        WritePtr<CameraNode> wCam = cam.get();
        if (wCam && wCam->isAnimated())
            wCam->endAnimation();
    }

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

std::optional<ContextExportVideoHD::PreparedAnimation> ContextExportVideoHD::prepareViewpointAnimation(Controller& controller) const
{
    if (!m_parameters.animationId.isValid())
        return std::nullopt;

    const std::unordered_map<viewPointAnimationId, ViewPointAnimationConfig>& configs = controller.getContext().cgetViewPointAnimations();
    auto itConfig = configs.find(m_parameters.animationId);
    if (itConfig == configs.end())
        return std::nullopt;

    std::unordered_map<xg::Guid, SafePtr<ViewPointNode>> perspectiveById;
    const std::unordered_set<SafePtr<AGraphNode>> allViewpoints = controller.getGraphManager().getNodesByTypes({ ElementType::ViewPoint }, ObjectStatusFilter::ALL);
    for (const SafePtr<AGraphNode>& node : allViewpoints)
    {
        SafePtr<ViewPointNode> viewpoint = static_pointer_cast<ViewPointNode>(node);
        ReadPtr<ViewPointNode> rViewpoint = viewpoint.cget();
        if (!rViewpoint || rViewpoint->getProjectionMode() == ProjectionMode::Orthographic)
            continue;
        perspectiveById.insert_or_assign(rViewpoint->getId(), viewpoint);
    }

    PreparedAnimation prepared;
    prepared.mode = itConfig->second.getMode();

    double previousTime = -std::numeric_limits<double>::infinity();
    for (const ViewPointAnimationLine& line : itConfig->second.getLines())
    {
        auto itVp = perspectiveById.find(line.viewpointId);
        if (itVp == perspectiveById.end())
            continue;

        prepared.viewpoints.push_back(itVp->second);
        prepared.controlTimes.push_back(line.position);
        if (prepared.mode == ViewPointAnimationMode::PositionAsTime)
        {
            if (line.position <= previousTime)
                return std::nullopt;
            previousTime = line.position;
        }
    }

    if (prepared.viewpoints.size() < 2)
        return std::nullopt;

    if (prepared.mode == ViewPointAnimationMode::PositionAsTime)
        prepared.durationSeconds = std::max(0.0, prepared.controlTimes.back() - prepared.controlTimes.front());
    else
        prepared.durationSeconds = std::max(0.0, static_cast<double>(m_parameters.length));

    if (prepared.durationSeconds <= 0.0)
        prepared.durationSeconds = std::max(1.0 / std::max(1, m_parameters.fps), 0.001);

    return prepared;
}
