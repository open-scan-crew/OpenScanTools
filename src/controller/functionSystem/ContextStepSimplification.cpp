#include "controller/functionSystem/ContextStepSimplification.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h" // forward declaration
#include "controller/messages/ImportMessage.h"
#include "utils/Logger.h"
#include "utils/Utils.h"

#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/MeshObjectTexts.hpp"
#include "gui/texts/SplashScreenTexts.hpp"
#include "gui/texts/ContextTexts.hpp"
#include "io/imports/step-simplification/step-simplification.h"
#include "controller/controls/ControlMeshObject.h"

#include "magic_enum/magic_enum.hpp"
#include "vulkan/Graph/MemoryReturnCode.h"


ContextStepSimplification::ContextStepSimplification(const ContextId& id)
	: ARayTracingContext(id)
{}

ContextStepSimplification::~ContextStepSimplification()
{}

ContextState ContextStepSimplification::start(Controller& controller)
{
	return (m_state = ContextState::waiting_for_input);
}

ContextState ContextStepSimplification::feedMessage(IMessage* message, Controller& controller)
{
	switch (message->getType())
	{
		case IMessage::MessageType::STEP_SIMPLIFICATION:
		{
			StepSimplificationMessage* msg = static_cast<StepSimplificationMessage*>(message);
			m_keepPercent = msg->m_keepPercent;
			m_classification = msg->m_classification;
			m_data = msg->m_data;
			m_outputPath = msg->m_outputPath;
			m_importAfter = msg->m_importAfter;
			if (m_importAfter && m_data.posOption == PositionOptions::ClickPosition)
			{
				m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_MESHOBJECT_START });
				return ARayTracingContext::start(controller);
			}
			else
				return (m_state = ContextState::ready_for_using);
		}
		case IMessage::MessageType::FULL_CLICK:
		{
			return ARayTracingContext::feedMessage(message, controller);
		}
		break;
		default:
			FUNCLOG << "wrong message type (" << magic_enum::enum_name<IMessage::MessageType>(message->getType()) << ")" << LOGENDL;
			break;
	}
    return m_state;
}

ContextState ContextStepSimplification::launch(Controller& controller)
{
    m_state = ContextState::running;

	// --- Ray Tracing ---
	if (m_importAfter && m_data.posOption == PositionOptions::ClickPosition) {
		ARayTracingContext::getNextPosition(controller);
		if (pointMissing())
			return waitForNextPoint(controller);
	}
	// -!- Ray Tracing -!-

    FUNCLOG << "AConstextStepSimplification launch" << LOGENDL;
	
	std::filesystem::path newFile;
	
	if (!m_outputPath.empty())
		newFile = m_outputPath;
	else
	{
		const std::filesystem::path& folder = controller.getContext().cgetProjectInternalInfo().getObjectsFilesFolderPath();
	
		std::string filename = m_data.file.filename().stem().string() + '_' + std::string(magic_enum::enum_name<StepClassification>(m_classification)) + '_' + std::to_string(m_keepPercent) + Utils::to_utf8(m_data.file.filename().extension().wstring());
		newFile = folder / filename;
	}
	std::filesystem::path newFileParentPath = newFile.parent_path();
	if (!std::filesystem::exists(newFileParentPath) && !std::filesystem::create_directories(newFileParentPath))
	{
		controller.updateInfo(new GuiDataWarning(TEXT_DIRECTORY_CREATION_FAILED.arg(QString::fromStdWString(newFile.wstring()))));
		return ARayTracingContext::abort(controller);
	}
	
	if (getFileType(m_data.file.extension()) == FileType::MAX_ENUM)
		m_data.file += ".step";

	controller.updateInfo(new GuiDataProcessingSplashScreenStart(5, TEXT_SPLASH_SCREEN_SIMPLIFY_STEP_DATA_TITLE, TEXT_SPLASH_SCREEN_SIMPLIFY_STEP_DATA_PROCESSING));
	ObjectAllocation::ReturnCode ret(StepSimplification::modelSimplification(m_data.file, newFile, m_classification, m_keepPercent, controller));


	if (ret != ObjectAllocation::ReturnCode::Success)
	{
		controller.updateInfo(new GuiDataWarning(ObjectAllocation::getText(ret)));
		controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_ABORT));
		return ARayTracingContext::abort(controller);
	}


	controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));


	if (m_importAfter)
	{
		m_data.file = newFile;
		if (m_data.posOption == PositionOptions::ClickPosition)
		{
			m_data.posOption = PositionOptions::GivenCoordinates;
			m_data.position = m_clickResults[0].position;
		}

		controller.getControlListener()->notifyUIControl(new control::meshObject::CreateMeshObjectFromFile(m_data));
	}
    
    return ARayTracingContext::validate(controller);
}

bool ContextStepSimplification::canAutoRelaunch() const
{
    return false;
}

ContextType ContextStepSimplification::getType() const
{
	return ContextType::stepSimplification;
}
