#include "controller/controls/ControlPCObject.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"
#include "io/SaveLoadSystem.h"
#include "models/3d/DuplicationTypes.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/Texts.hpp"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataTree.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "controller/messages/PointCloudObjectCreationParametersMessage.h"
#include "utils/Utils.h"

#include "models/graph/ScanObjectNode.h"

namespace control
{
	namespace pcObject
	{
		
		/*
		* CreatePCObjectFromBoxActivate
		*/

		CreatePCObjectFromBoxActivate::CreatePCObjectFromBoxActivate()
		{}

		CreatePCObjectFromBoxActivate::~CreatePCObjectFromBoxActivate()
		{}

		void CreatePCObjectFromBoxActivate::doFunction(Controller& controller)
		{
			controller.getFunctionManager().launchFunction(controller, ContextType::pointCloudObjectCreation);
		}

		ControlType CreatePCObjectFromBoxActivate::getType() const
		{
			return (ControlType::createPCObjectFromBox);
		}

		/*
		* CreatePCObjectFromFile
		*/

		CreatePCObjectFromFile::CreatePCObjectFromFile(const std::list<std::filesystem::path>& pcFiles)
			:m_files(pcFiles)
		{}

		CreatePCObjectFromFile::~CreatePCObjectFromFile()
		{}
		
		void CreatePCObjectFromFile::doFunction(Controller& controller)
		{
			if (m_files.empty())
				return;

			for (const std::filesystem::path& path : m_files)
			{
				SaveLoadSystem::ErrorCode error;
				SaveLoadSystem::ImportTlsFileAsObject(path, controller, error);
				if (error == SaveLoadSystem::ErrorCode::Success)
					CONTROLLOG << "control::pcObject::CreatePCObjectFromFile do " << LOGENDL;
				else
					controller.updateInfo(new GuiDataWarning(TEXT_SCAN_IMPORT_AS_OBJECT_FAILED));
				
			}
		}

		ControlType  CreatePCObjectFromFile::getType() const
		{
			return (ControlType::createPCObjectFromFile);
		}

		/*
		** ActivateDuplicate
		*/

		ActivateDuplicate::ActivateDuplicate()
		{
		}

		ActivateDuplicate::~ActivateDuplicate()
		{
		}

		void ActivateDuplicate::doFunction(Controller& controller)
		{
            controller.getFunctionManager().launchFunction(controller, ContextType::pointCloudObjectDuplication);
			CONTROLLOG << "control::pcObject::ActivateDuplicate" << LOGENDL;
		}

		bool ActivateDuplicate::canUndo() const
		{
			return (false);
		}

		void ActivateDuplicate::undoFunction(Controller& controller)
		{
		}

		ControlType ActivateDuplicate::getType() const
		{
			return (ControlType::activateDuplicateFunctionPCO);
		}

	}
}