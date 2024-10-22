#include "controller/controls/ControlSignals.h"
#include "controller/Controller.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "utils/Logger.h"

#include "gui/texts/SystemTexts.hpp"

namespace control
{
	namespace signalHandling
	{

		ISigControl::ISigControl() {}

		ISigControl::~ISigControl() {}

		void ISigControl::doFunction(Controller& controller)
		{
			controller.updateInfo(new GuiDataWarning(TEXT_APPLICATION_CRASHED));
			//to check
			/*if (controller.getCurrentProject() != nullptr)
			{
				controller.getCurrentProject()->getProjectInternalInfo().setProjectFolderPath("crash_project", controller.getCurrentProject()->cgetProjectInfo()._projectName);
				controller.setIsCurrentProjectSaved(true);
				controller.updateInfo({ uiDataKey(guiDType::saveState), new GuiDataSaveAble(false) });
				CONTROLLOG << "control::signalHandling::ISigControl [ crash_project ]" << LOGENDL;
			}*/
			controller.updateInfo(new GuiDataQuit());
		}

		bool ISigControl::canUndo() const
		{
			return (false);
		}

		void ISigControl::undoFunction(Controller& controller) {}



		SigInt::SigInt() {}

		SigInt::~SigInt() {}

		void SigInt::doFunction(Controller& controller)
		{
			CONTROLLOG << "SIG raise: SIGINT" << LOGENDL;
			ISigControl::doFunction(controller);
		}
		
		ControlType SigInt::getType() const
		{
			return (ControlType::SIGINTSignalHandling);
		}



		SigTerm::SigTerm() {}

		SigTerm::~SigTerm() {}

		void SigTerm::doFunction(Controller& controller)
		{
			CONTROLLOG << "SIG raise: SIGTERM" << LOGENDL;
			ISigControl::doFunction(controller);
		}
		
		ControlType SigTerm::getType() const
		{
			return (ControlType::SIGTERMSignalHandling);
		}



		SigIll::SigIll() {}

		SigIll::~SigIll() {}

		void SigIll::doFunction(Controller& controller) 
		{
			CONTROLLOG << "SIG raise: SIGILL" << LOGENDL;
			ISigControl::doFunction(controller);
		}
			
		ControlType SigIll::getType() const
		{
			return (ControlType::SIGILLSignalHandling);
		}



		SigAbrt::SigAbrt() {}

		SigAbrt::~SigAbrt() {}

		void SigAbrt::doFunction(Controller& controller) 
		{
			CONTROLLOG << "SIG raise: SIGABRT" << LOGENDL;
			ISigControl::doFunction(controller);
		}

		ControlType SigAbrt::getType() const
		{
			return (ControlType::SIGABRTSignalHandling);
		}



		SigSegv::SigSegv() {}

		SigSegv::~SigSegv() {}

		void SigSegv::doFunction(Controller& controller)
		{
			CONTROLLOG << "SIG raise: SIGSEGV" << LOGENDL;
			ISigControl::doFunction(controller);
		}

		ControlType SigSegv::getType() const 
		{
			return (ControlType::SIGSEGVSignalHandling);
		}



		SigFpe::SigFpe() {}

		SigFpe::~SigFpe() {}

		void SigFpe::doFunction(Controller& controller) 
		{
			CONTROLLOG << "SIG raise: SIGFPE" << LOGENDL;
			ISigControl::doFunction(controller);
		}

		ControlType SigFpe::getType() const 
		{
			return (ControlType::SIGFPESignalHandling);
		}
	}
}
