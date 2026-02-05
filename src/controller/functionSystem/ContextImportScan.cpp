#include "controller/functionSystem/ContextImportScan.h"
#include "controller/messages/ImportMessage.h"
#include "controller/controls/ControlProject.h"
#include "controller/Controller.h"
#include "controller/IControlListener.h"
#include "controller/functionSystem/ProgressTracker.h"
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

	const size_t total_units = m_scanInfo.paths.size();
	const size_t total_steps = total_units > 0 ? total_units * 100 : 1;
	controller.updateInfo(new GuiDataProcessingSplashScreenStart(total_steps, TEXT_IMPORTING_SCAN, TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(0).arg(total_units)));
	GraphManager& graphManager = controller.getGraphManager();

	ProgressTracker progressTracker(controller, total_units,
		[](size_t unitsDone, size_t totalUnits, int percent)
		{
			return QString("%1 - %2%")
				.arg(TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(unitsDone).arg(totalUnits))
				.arg(percent);
		});

	size_t units_done = 0;

	for (const std::filesystem::path& inputFile : m_scanInfo.paths)
	{
		if (m_state != ContextState::running)
			return ContextState::abort;
		controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString::fromStdWString(inputFile.wstring())));
		SaveLoadSystem::ErrorCode error;

		auto copyProgress = progressTracker.makeCallback(units_done, 0, 100);
		SafePtr<PointCloudNode> pcObj = SaveLoadSystem::ImportNewTlsFile(inputFile, m_scanInfo.asObject, controller, error, copyProgress);

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

		units_done++;
		progressTracker.update(units_done, 100);
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
