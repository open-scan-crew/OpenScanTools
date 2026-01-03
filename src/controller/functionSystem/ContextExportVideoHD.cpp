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
    if (m_parameters.animMode == VideoAnimationMode::BETWEENVIEWPOINTS && m_parameters.start == m_parameters.finish)
        return abort(controller);

    //Start - Move to start position
    if (m_exportState == 0)
    {
        SafePtr<CameraNode> cam = controller.getGraphManager().getCameraNode();
        WritePtr<CameraNode> wCam = cam.get();
        if (!wCam)
            return abort(controller);

        wCam->setProjectionMode(ProjectionMode::Perspective);
        m_exportState = 1;

        if (m_parameters.animMode == VideoAnimationMode::BETWEENVIEWPOINTS)
        {
            wCam->moveToData(m_parameters.start);
            return m_state = ContextState::waiting_for_input;
        }

    }

    //Calcul camera move delta
    if (m_exportState == 1)
    {
        SafePtr<CameraNode> cam = controller.getGraphManager().getCameraNode();
        ReadPtr<CameraNode> rCam = cam.cget();
        if (!rCam)
            return abort(controller);
        m_totalFrames = (long)m_parameters.length * (long)m_parameters.fps;
        m_animFrame = 1;

        controller.updateInfo(new GuiDataProcessingSplashScreenStart(m_totalFrames, TEXT_CONTEXT_EXPORT_VIDEO, TEXT_CONTEXT_EXPORT_VIDEO_STEPS.arg(0).arg(m_totalFrames)));
        m_tpStart = std::chrono::steady_clock::now();

        if (m_parameters.animMode == VideoAnimationMode::BETWEENVIEWPOINTS)
        {
            ReadPtr<ViewPointNode> rStart;
            ReadPtr<ViewPointNode> rFinish;
            multi_cget(m_parameters.start, m_parameters.finish, rStart, rFinish);
            if (!rStart || !rFinish)
                return abort(controller);

            glm::dvec3 finishLookDir = glm::dvec4(0.0, 0.0, 1.0, 1.0) * rFinish->getInverseTransformation();

            double endTheta;
            if (finishLookDir.x == 0.0 && finishLookDir.y == 0.0)   // Must we change the test to x < epsilon ?
                endTheta = 0.;
            else
                endTheta = atan2(-finishLookDir.x, finishLookDir.y);

            double normXY = sqrt(finishLookDir.x * finishLookDir.x + finishLookDir.y * finishLookDir.y);
            double endPhi = atan2(-normXY, finishLookDir.z);

            CameraNode::modulo2Pi(rCam->getTheta(), endTheta);

            m_addPosition = (rFinish->getCenter() - rStart->getCenter()) / (double)m_totalFrames;
            m_addTheta = (endTheta - rCam->getTheta()) / m_totalFrames;
            m_addPhi = (endPhi - rCam->getPhi()) / m_totalFrames;

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
                const float denom = std::max(1l, m_totalFrames - 1);
                m_addTranspUIScale = (ui::transparency::trueValue_to_uiValue(rFinish->m_transparency) - ui::transparency::trueValue_to_uiValue(rStart->m_transparency)) / denom;

                m_addNStren = (rFinish->m_postRenderingNormals.normalStrength - rStart->m_postRenderingNormals.normalStrength) / denom;
                m_addNGloss = (rFinish->m_postRenderingNormals.gloss - rStart->m_postRenderingNormals.gloss) / denom;

                m_addHue = (rFinish->m_hue - rStart->m_hue) / denom;

                m_addBright = (rFinish->m_brightness - rStart->m_brightness) / denom;
                m_addSatur = (rFinish->m_saturation - rStart->m_saturation) / denom;
                m_addLumi = (rFinish->m_luminance - rStart->m_luminance) / denom;
                m_addContr = (rFinish->m_contrast - rStart->m_contrast) / denom;
                m_addAlpha = (rFinish->m_alphaObject - rStart->m_alphaObject) / denom;

                m_flashControlStart = rStart->m_flashControl;
                m_addFlashControl = (rFinish->m_flashControl - rStart->m_flashControl) / denom;
                m_flashAdvancedStart = rStart->m_flashAdvanced ? 1.f : 0.f;
                float targetFlashAdvanced = rFinish->m_flashAdvanced ? 1.f : 0.f;
                m_addFlashAdvanced = (targetFlashAdvanced - m_flashAdvancedStart) / denom;

                m_addFovy = (rFinish->getFovy() - rStart->getFovy()) / denom;
            }
        }
        else
            m_addTheta = 2 * M_PI / m_totalFrames;

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
                wCam->addGlobalTranslation(m_addPosition);
                wCam->yaw(m_addTheta);
                wCam->pitch(m_addPhi);

                if (m_parameters.interpolateRenderingBetweenViewpoints)
                {
                    
                    wCam->m_transparency = ui::transparency::uiValue_to_trueValue(ui::transparency::trueValue_to_uiValue(wCam->m_transparency) + m_addTranspUIScale);
                    wCam->m_postRenderingNormals.normalStrength += m_addNStren;
                    wCam->m_postRenderingNormals.gloss += m_addNGloss;
                    wCam->m_contrast += m_addContr;
                    wCam->m_brightness += m_addBright;
                    wCam->m_luminance += m_addLumi;
                    wCam->m_saturation += m_addSatur;
                    wCam->m_hue += m_addHue;
                    wCam->m_alphaObject += m_addAlpha;
                    long step = std::max(0l, m_animFrame - 1);
                    wCam->m_flashControl = m_flashControlStart + m_addFlashControl * step;
                    m_flashAdvancedValue = m_flashAdvancedStart + m_addFlashAdvanced * step;
                    wCam->m_flashAdvanced = m_flashAdvancedValue >= 0.5f;
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
        if(m_parameters.openFolderAfterExport)
            controller.updateInfo(new GuiDataOpenInExplorer(m_exportPath));
        return validate(controller);
    }

    return abort(controller);
}

ContextState ContextExportVideoHD::abort(Controller& controller)
{
    controller.updateInfo(new GuiDataRenderDecimationOptions(m_precedentOptions));

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

std::filesystem::path ContextExportVideoHD::getNextFramePath()
{
    std::filesystem::path nextPath = m_exportPath;
    std::wstring filename = m_exportPath.stem().wstring();
    uint8_t size = std::log10(m_totalFrames) + 1;
    nextPath = nextPath / (filename + L"_" + Utils::wCompleteWithZeros(m_animFrame, size));
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
