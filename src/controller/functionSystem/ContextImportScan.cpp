#include "controller/functionSystem/ContextImportScan.h"
#include "controller/messages/ImportMessage.h"
#include "controller/controls/ControlProject.h"
#include "controller/controls/ControlTree.h"
#include "controller/controls/ControlMetaControl.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "io/SaveLoadSystem.h"

#include "models/graph/ScanNode.h"
#include "models/graph/ScanObjectNode.h"
#include "models/graph/GraphManager.hxx"

#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataTree.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "gui/texts/ContextTexts.hpp"
#include "gui/texts/SplashScreenTexts.hpp"
#include "gui/texts/PointCloudTexts.hpp"
#include "magic_enum/magic_enum.hpp"

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

	controller.updateInfo(new GuiDataProcessingSplashScreenStart(m_scanInfo.paths.size(), TEXT_IMPORTING_SCAN, QString()));
	GraphManager& graphManager = controller.getGraphManager();


	for (const std::filesystem::path& inputFile : m_scanInfo.paths)
	{
		if (m_state != ContextState::running)
			return ContextState::abort;
		controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString::fromStdWString(inputFile.wstring())));
		SaveLoadSystem::ErrorCode error;

		bool importSuccess = false;

		if (m_scanInfo.asObject)
		{
			SafePtr<ScanObjectNode> scanObj = SaveLoadSystem::ImportTlsFileAsObject(inputFile, controller, error);
			if (scanObj)
			{
				WritePtr<ScanObjectNode> wScanObj = scanObj.get();
				switch (m_scanInfo.positionOption)
				{
					case PositionOptions::ClickPosition:
						wScanObj->setPosition(m_clickResults[0].position);
						break;
					case PositionOptions::GivenCoordinates:
						wScanObj->setPosition(m_scanInfo.positionAsObject);
						break;
				}
				importSuccess = true;
			}
		}
		else
			importSuccess = bool(SaveLoadSystem::ImportNewTlsFile(inputFile, controller, error));

		if (!importSuccess)
		{
			CONTROLLOG << "Error during control::project::ImportScan" << LOGENDL;
			controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(TEXT_SCAN_IMPORT_FAILED).arg(QString::fromStdWString(inputFile.wstring()))));
			controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));
			controller.getControlListener()->notifyUIControl(new control::project::StartSave());

			continue;
		}
		CONTROLLOG << "update infos during control::project::ImportScan" << LOGENDL;
		updateStep(controller, QString(TEXT_SCAN_IMPORT_DONE_TEXT).arg(QString::fromStdWString(inputFile.stem().wstring())), 1);
	}


	controller.getControlListener()->notifyUIControl(new control::project::ApplyProjectTransformation());
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
