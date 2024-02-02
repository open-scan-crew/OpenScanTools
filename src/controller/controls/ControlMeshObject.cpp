#include "controller/controls/ControlMeshObject.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/controls/ControlFunction.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "models/3d/Graph/MeshObjectNode.h"
#include "models/3d/Graph/ClusterNode.h"
#include "models/3d/DuplicationTypes.h"

#include "gui/GuiData/GuiDataMessages.h"
#include "gui/Texts.hpp"
#include "controller/messages/FilesMessage.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataTree.h"
#include "gui/GuiData/GuiData3dObjects.h"

#include "vulkan/MeshManager.h"

#include "utils/Logger.h"

namespace control
{
	namespace meshObject
	{
		/*
		* CreateMeshObjectFromFile
		*/

		CreateMeshObjectFromFile::CreateMeshObjectFromFile(const FileInputData& data)
			: m_data(data)
		{}

		CreateMeshObjectFromFile::~CreateMeshObjectFromFile()
		{}
		
		void CreateMeshObjectFromFile::doFunction(Controller& controller)
		{
			controller.getFunctionManager().launchFunction(controller, ContextType::meshObjectCreation);
			controller.getFunctionManager().feedMessageToSpecificContext(controller, new ImportMeshObjectMessage(m_data), ContextType::meshObjectCreation);
		}

		bool CreateMeshObjectFromFile::canUndo() const
		{
			return (false);
		}

		void CreateMeshObjectFromFile::undoFunction(Controller& controller)
		{
			controller.getFunctionManager().abort(controller);
		}

		ControlType  CreateMeshObjectFromFile::getType() const
		{
			return (ControlType::createWavefrontFromFile);
		}

		/*
		* StepSimplification
		*/

		StepSimplification::StepSimplification(const FileInputData& data, const StepClassification& classification, const double& keepPercent, const std::filesystem::path& outputPath, bool importAfter)
			: m_data(data)
			, m_classification(classification)
			, m_keepPercent(keepPercent)
			, m_importAfter(importAfter)
			, m_outputPath(outputPath)
		{}

		StepSimplification::~StepSimplification()
		{}

		void StepSimplification::doFunction(Controller & controller)
		{
			controller.getFunctionManager().launchFunction(controller, ContextType::stepSimplification);
			controller.getFunctionManager().feedMessageToSpecificContext(controller, new StepSimplificationMessage(m_data, m_classification, m_keepPercent, m_outputPath, m_importAfter), ContextType::stepSimplification);
		}

		bool StepSimplification::canUndo() const
		{
			return false;
		}

		void StepSimplification::undoFunction(Controller & controller)
		{
		}

		ControlType StepSimplification::getType() const
		{
			return ControlType::stepSimplification;
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
            controller.getFunctionManager().launchFunction(controller, ContextType::meshObjectDuplication);
			CONTROLLOG << "control::meshObject::ActivateDuplicate" << LOGENDL;
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
			return (ControlType::activateDuplicateFunctionWavefront);
		}


	

	}
}