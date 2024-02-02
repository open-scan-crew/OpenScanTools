#include "controller/functionSystem/ContextImportScan.h"
#include "controller/messages/FilesMessage.h"
#include "controller/controls/ControlProject.h"
#include "controller/controls/ControlTree.h"
#include "controller/controls/ControlMetaControl.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "io/SaveLoadSystem.h"

#include "models/3d/Graph/ScanNode.h"
#include "models/3d/Graph/OpenScanToolsGraphManager.hxx"

#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataTree.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "gui/texts/ContextTexts.hpp"
#include "gui/texts/SplashScreenTexts.hpp"
#include "magic_enum/magic_enum.hpp"

ContextImportScan::ContextImportScan(const ContextId& id)
	: AContext(id)
{}

ContextImportScan::~ContextImportScan()
{}

ContextState ContextImportScan::start(Controller& controller)
{
	return (m_state = ContextState::waiting_for_input);
}

ContextState ContextImportScan::feedMessage(IMessage* message, Controller& controller)
{
	if (message->getType() != IMessage::MessageType::FILES)
	{
		FUNCLOG << "wrong message type (" << magic_enum::enum_name<IMessage::MessageType>(message->getType())<< ")" << LOGENDL;
		return (m_state);
	}
	FilesMessage* out = dynamic_cast<FilesMessage*>(message);
	if (out == nullptr)
	{
		FUNCLOG << "failed to covert in importMessage" << LOGENDL;
		return (m_state);
	}
	m_inputFiles = out->m_inputFiles;
	FUNCLOG << "function set ready to import!" << LOGENDL;
	return (m_state = ContextState::ready_for_using);
}

ContextState ContextImportScan::launch(Controller& controller)
{
	controller.updateInfo(new GuiDataProcessingSplashScreenStart(m_inputFiles.size(), TEXT_IMPORTING_SCAN, QString()));
	OpenScanToolsGraphManager& graphManager = controller.getOpenScanToolsGraphManager();

	SafePtr<ScanNode> scan;

	for (const std::filesystem::path& inputFile : m_inputFiles)
	{
		if (m_state != ContextState::running)
			return ContextState::abort;
		controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString::fromStdWString(inputFile.wstring())));
		SaveLoadSystem::ErrorCode error;
		if (!(scan = SaveLoadSystem::ImportNewTlsFile(inputFile, controller, error)))
		{
			CONTROLLOG << "Error during control::project::ImportScan" << LOGENDL;
			controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(TEXT_SCAN_IMPORT_FAILED).arg(QString::fromStdWString(inputFile.wstring()))));
			controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));
			controller.getControlListener()->notifyUIControl(new control::project::StartSave());
			//return (m_state = ContextState::abort);
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
