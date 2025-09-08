#include "controller/functionSystem/ContextImportScan.h"
#include "controller/messages/ImportMessage.h"
#include "controller/controls/ControlProject.h"
#include "controller/Controller.h"
#include "controller/IControlListener.h"
#include "io/SaveLoadSystem.h"

#include "models/graph/PointCloudNode.h"
#include "models/graph/GraphManager.h"

#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "gui/texts/SplashScreenTexts.hpp"
#include "gui/texts/PointCloudTexts.hpp"

#include "utils/Logger.h"


ContextImportScan::ContextImportScan(const ContextId& id)
	: ARayTracingContext(id)
	, m_currentStep(0)
{}

ContextImportScan::~ContextImportScan()
{}

ContextState ContextImportScan::start(Controller& controller)
{
	return (m_state = ContextState::waiting_for_input);
}

ContextState ContextImportScan::feedMessage(IMessage* message, Controller& controller)
{
	if (message->getType() == IMessage::MessageType::IMPORT_SCAN)
	{
		ImportScanMessage* out = dynamic_cast<ImportScanMessage*>(message);
		if (out == nullptr)
		{
			FUNCLOG << "failed to covert in importMessage" << LOGENDL;
			return (m_state);
		}
		m_scanInfo = out->m_data;
		if (m_scanInfo.positionOption == PositionOptions::ClickPosition)
		{
			m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_POINT_CLOUD_OBJECT_START });
			return ARayTracingContext::start(controller);
		}
		else
			return (m_state = ContextState::ready_for_using);
	}
	else
		return ARayTracingContext::feedMessage(message, controller);
}

ContextState ContextImportScan::launch(Controller& controller)
{
	// --- Ray Tracing ---
	if (m_scanInfo.positionOption == PositionOptions::ClickPosition) {
		ARayTracingContext::getNextPosition(controller);
		if (pointMissing())
			return waitForNextPoint(controller);
	}
	// -!- Ray Tracing -!-

	controller.updateInfo(new GuiDataProcessingSplashScreenStart(m_scanInfo.paths.size(), TEXT_IMPORTING_SCAN, QString()));
	GraphManager& graphManager = controller.getGraphManager();


	for (const std::filesystem::path& inputFile : m_scanInfo.paths)
	{
		if (m_state != ContextState::running)
			return ContextState::abort;
		controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString::fromStdWString(inputFile.wstring())));
		SaveLoadSystem::ErrorCode error;

		SafePtr<PointCloudNode> pcObj = SaveLoadSystem::ImportNewTlsFile(inputFile, m_scanInfo.asObject, controller, error);

		if (error != SaveLoadSystem::ErrorCode::Success)
		{
			CONTROLLOG << "Error during ContextImportScan" << LOGENDL;
			controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(TEXT_SCAN_IMPORT_FAILED).arg(QString::fromStdWString(inputFile.wstring()))));
			controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));
			controller.getControlListener()->notifyUIControl(new control::project::StartSave());
			continue;
		}

		if (m_scanInfo.asObject)
		{
			WritePtr<PointCloudNode> wPcObj = pcObj.get();
			if (!wPcObj)
				continue;
			switch (m_scanInfo.positionOption)
			{
			case PositionOptions::ClickPosition:
				wPcObj->setPosition(m_clickResults[0].position);
				break;
			case PositionOptions::GivenCoordinates:
				wPcObj->setPosition(m_scanInfo.positionAsObject);
				break;
			}
		}

		updateStep(controller, QString(TEXT_SCAN_IMPORT_DONE_TEXT).arg(QString::fromStdWString(inputFile.stem().wstring())), 1);
	}

	controller.getControlListener()->notifyUIControl(new control::project::StartSave());
		
	controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));

	return (m_state = ContextState::done);
}

bool ContextImportScan::canAutoRelaunch() const
{
	return (false);
}

ContextType ContextImportScan::getType() const
{
	return ContextType::scanImport;
}

void ContextImportScan::updateStep(Controller& controller, const QString& state, const uint64_t& step)
{
	m_currentStep += step;
	controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(state, m_currentStep));
}
