#include "controller/controls/ControlSMeasureEdition.h"
#include "controller/Controller.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataTree.h"

#include "models/data/BeamBendingMeasure.h"
#include "models/data/SimpleMeasure.h"
#include "models/data/PointToPlaneMeasure.h"

#include <glm/glm.hpp>

// control::SMeasureEdition::

namespace control
{
	namespace SMeasureEdition
	{
		/*
		** SetDescription
		*/

		SetDescription::SetDescription(dataId id, std::string newDesc)
		{
			m_id = id;
			m_newDesc = newDesc;
		}

		SetDescription::~SetDescription()
		{
		}

		void SetDescription::doFunction(Controller& controller)
		{
			CONTROLLOG << "control::SMeasureEdition::SetDescription do elemid " << m_id << LOGENDL;

			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			ModelSimpleMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<ModelSimpleMeasure>(m_id);
			if (_element == nullptr)
			{
				CONTROLLOG << "_element" << LOGENDL;
				return;
			}
			m_oldDesc = _element->getDescription();
			_element->setDescription(m_newDesc);
			CONTROLLOG << "m_oldDesc " << m_oldDesc << " m_newDesc " << m_newDesc << LOGENDL;
			controller.updateInfo(new GuiDataSimpleMeasureProperties(_element->ProduceUISimpleMeasure()));
		}

		bool SetDescription::canUndo()
		{
			return (true);
		}

		void SetDescription::undoFunction(Controller& controller)
		{
			CONTROLLOG << "control::SMeasureEdition::SetDescription undo elemid " << m_id << " m_oldDesc " << m_oldDesc << " m_newDesc " << m_newDesc << LOGENDL;
			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			ModelSimpleMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<ModelSimpleMeasure>(m_id);
			if (_element == nullptr)
				return;
			_element->setDescription(m_oldDesc);
			controller.updateInfo(new GuiDataSimpleMeasureProperties(_element->ProduceUISimpleMeasure()));
		}

		UIControl SetDescription::getType()
		{
			return (UIControl::setDescriptionSMeasureEdit);
		}

		/*
		** SetName
		*/

		SetName::SetName(dataId id, const std::string newName)
		{
			m_id = id;
			m_newName = newName;
		}

		SetName::~SetName()
		{}

		void SetName::doFunction(Controller& controller)
		{
			CONTROLLOG << "control::SMeasureEdition::SetName do elemid " << m_id << LOGENDL;
			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			ModelSimpleMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<ModelSimpleMeasure>(m_id);
			if (_element == nullptr)
				return;
			m_oldName = _element->getName();
			_element->setName(m_newName);
			controller.actualizeOnId(m_id, true);
		}

		bool SetName::canUndo()
		{
			return true;
		}

		void SetName::undoFunction(Controller& controller)
		{
			CONTROLLOG << "control::SMeasureEdition::SetName undo elemid " << m_id << LOGENDL;
			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			ModelSimpleMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<ModelSimpleMeasure>(m_id);
			if (_element == nullptr)
				return;
			_element->setName(m_oldName);

			controller.actualizeOnId(m_id, true);
		}
		
		UIControl  SetName::getType()
		{
			return (UIControl::setNameSMeasureEdit);
		}
	}
	namespace BBMeasureEdition
	{
		/*
		** SetDescription
		*/

		SetDescription::SetDescription(dataId id, std::string newDesc)
		{
			m_id = id;
			m_newDesc = newDesc;
		}

		SetDescription::~SetDescription()
		{
		}

		void SetDescription::doFunction(Controller& controller)
		{
			CONTROLLOG << "control::BBMeasureEdition::SetDescription do elemid " << m_id << LOGENDL;
			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			BeamBendingMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<BeamBendingMeasure>(m_id);
			if (_element == nullptr)
			{
				CONTROLLOG << "_element" << LOGENDL;
				return;
			}
			m_oldDesc = _element->getDescription();
			_element->setDescription(m_newDesc);
			CONTROLLOG << "m_oldDesc " << m_oldDesc << " m_newDesc " << m_newDesc << LOGENDL;
			controller.updateInfo(new GuiDataBeamBendingProperties(_element->ProduceUIBeamBendingMeasure()));
		}

		bool SetDescription::canUndo()
		{
			return (true);
		}

		void SetDescription::undoFunction(Controller& controller)
		{
			CONTROLLOG << "control::BBMeasureEdition::SetDescription undo elemid " << m_id << " m_oldDesc " << m_oldDesc << " m_newDesc " << m_newDesc << LOGENDL;
			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			BeamBendingMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<BeamBendingMeasure>(m_id);
			if (_element == nullptr)
				return;
			_element->setDescription(m_oldDesc);
			controller.updateInfo(new GuiDataBeamBendingProperties(_element->ProduceUIBeamBendingMeasure()));
		}

		UIControl SetDescription::getType()
		{
			return (UIControl::setDescriptionBBMeasureEdit);
		}

		/*
		** SetName
		*/

		SetName::SetName(dataId id, const std::string newName)
		{
			m_id = id;
			m_newName = newName;
		}

		SetName::~SetName()
		{}

		void SetName::doFunction(Controller& controller)
		{
			CONTROLLOG << "control::BBMeasureEdition::SetName do elemid " << m_id << LOGENDL;
			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			BeamBendingMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<BeamBendingMeasure>(m_id);
			if (_element == nullptr)
				return;
			m_oldName = _element->getName();
			_element->setName(m_newName);
			controller.actualizeOnId(m_id, true);
		}

		bool SetName::canUndo()
		{
			return true;
		}

		void SetName::undoFunction(Controller& controller)
		{
			CONTROLLOG << "control::BBMeasureEdition::SetName undo elemid " << m_id << LOGENDL;
			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			ModelSimpleMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<ModelSimpleMeasure>(m_id);
			if (_element == nullptr)
				return;
			_element->setName(m_oldName);
			controller.actualizeOnId(m_id, true);
		}

		UIControl  SetName::getType()
		{
			return (UIControl::setNameBBMeasureEdit);
		}
	}
	namespace PPMeasureEdition
	{
		/*
		** SetDescription
		*/

		SetDescription::SetDescription(dataId id, std::string newDesc)
		{
			m_id = id;
			m_newDesc = newDesc;
		}

		SetDescription::~SetDescription()
		{
		}

		void SetDescription::doFunction(Controller& controller)
		{
			CONTROLLOG << "control::PPMeasureEdition::SetDescription do elemid " << m_id << LOGENDL;
			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			PointToPlaneMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<PointToPlaneMeasure>(m_id);
			if (_element == nullptr)
			{
				CONTROLLOG << "_element" << LOGENDL;
				return;
			}
			m_oldDesc = _element->getDescription();
			_element->setDescription(m_newDesc);
			CONTROLLOG << "m_oldDesc " << m_oldDesc << " m_newDesc " << m_newDesc << LOGENDL;
			controller.updateInfo(new GuiDataPointToPlaneProperties(_element->ProduceUIPointToPlanMeasure()));
		}

		bool SetDescription::canUndo()
		{
			return (true);
		}

		void SetDescription::undoFunction(Controller& controller)
		{
			CONTROLLOG << "control::PPMeasureEdition::SetDescription undo elemid " << m_id << " m_oldDesc " << m_oldDesc << " m_newDesc " << m_newDesc << LOGENDL;
			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			PointToPlaneMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<PointToPlaneMeasure>(m_id);
			if (_element == nullptr)
				return;
			_element->setDescription(m_oldDesc);
			controller.updateInfo(new GuiDataPointToPlaneProperties(_element->ProduceUIPointToPlanMeasure()));
		}

		UIControl SetDescription::getType()
		{
			return (UIControl::setDescriptionPPMeasureEdit);
		}

		/*
		** SetName
		*/

		SetName::SetName(dataId id, const std::string newName)
		{
			m_id = id;
			m_newName = newName;
		}

		SetName::~SetName()
		{}

		void SetName::doFunction(Controller& controller)
		{
			CONTROLLOG << "control::PPMeasureEdition::SetName do elemid " << m_id << LOGENDL;
			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			PointToPlaneMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<PointToPlaneMeasure>(m_id);
			if (_element == nullptr)
				return;
			m_oldName = _element->getName();
			_element->setName(m_newName);
			controller.actualizeOnId(m_id, true);
		}

		bool SetName::canUndo()
		{
			return true;
		}

		void SetName::undoFunction(Controller& controller)
		{
			CONTROLLOG << "control::PPMeasureEdition::SetName undo elemid " << m_id << LOGENDL;
			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			PointToPlaneMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<PointToPlaneMeasure>(m_id);
			if (_element == nullptr)
				return;
			_element->setName(m_oldName);
			controller.actualizeOnId(m_id, true);
		}

		UIControl  SetName::getType()
		{
			return (UIControl::setNamePPMeasureEdit);
		}
	}
	namespace PiPMeasureEdition
	{
		/*
		** SetDescription
		*/

		SetDescription::SetDescription(dataId id, std::string newDesc)
		{
			m_id = id;
			m_newDesc = newDesc;
		}

		SetDescription::~SetDescription()
		{
		}

		void SetDescription::doFunction(Controller& controller)
		{
			CONTROLLOG << "control::PiPMeasureEdition::SetDescription do elemid " << m_id << LOGENDL;
			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			PipeToPipeMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<PipeToPipeMeasure>(m_id);
			if (_element == nullptr)
			{
				CONTROLLOG << "_element" << LOGENDL;
				return;
			}
			m_oldDesc = _element->getDescription();
			_element->setDescription(m_newDesc);
			CONTROLLOG << "m_oldDesc " << m_oldDesc << " m_newDesc " << m_newDesc << LOGENDL;
			controller.updateInfo(new GuiDataPipeToPipeProperties(_element->ProduceUIPipeToPipeMeasure()));
		}

		bool SetDescription::canUndo()
		{
			return (true);
		}

		void SetDescription::undoFunction(Controller& controller)
		{
			CONTROLLOG << "control::PiPMeasureEdition::SetDescription undo elemid " << m_id << " m_oldDesc " << m_oldDesc << " m_newDesc " << m_newDesc << LOGENDL;
			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			PipeToPipeMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<PipeToPipeMeasure>(m_id);
			if (_element == nullptr)
				return;
			_element->setDescription(m_oldDesc);
			controller.updateInfo(new GuiDataPipeToPipeProperties(_element->ProduceUIPipeToPipeMeasure()));
		}

		UIControl SetDescription::getType()
		{
			return (UIControl::setDescriptionPiPMeasureEdit);
		}

		/*
		** SetName
		*/

		SetName::SetName(dataId id, const std::string newName)
		{
			m_id = id;
			m_newName = newName;
		}

		SetName::~SetName()
		{}

		void SetName::doFunction(Controller& controller)
		{
			CONTROLLOG << "control::PiPMeasureEdition::SetName do elemid " << m_id << LOGENDL;
			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			PipeToPipeMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<PipeToPipeMeasure>(m_id);
			if (_element == nullptr)
				return;
			m_oldName = _element->getName();
			_element->setName(m_newName);
			controller.actualizeOnId(m_id, true);
		}

		bool SetName::canUndo()
		{
			return true;
		}

		void SetName::undoFunction(Controller& controller)
		{
			CONTROLLOG << "control::PiPMeasureEdition::SetName undo elemid " << m_id << LOGENDL;
			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			PipeToPipeMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<PipeToPipeMeasure>(m_id);
			if (_element == nullptr)
				return;
			_element->setName(m_oldName);
			controller.actualizeOnId(m_id, true);
		}

		UIControl  SetName::getType()
		{
			return (UIControl::setNamePiPMeasureEdit);
		}
	}
	namespace PiPlMeasureEdition
	{
		/*
		** SetDescription
		*/

		SetDescription::SetDescription(dataId id, std::string newDesc)
		{
			m_id = id;
			m_newDesc = newDesc;
		}

		SetDescription::~SetDescription()
		{
		}

		void SetDescription::doFunction(Controller& controller)
		{
			CONTROLLOG << "control::PiPlMeasureEdition::SetDescription do elemid " << m_id << LOGENDL;
			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			PipeToPlaneMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<PipeToPlaneMeasure>(m_id);
			if (_element == nullptr)
			{
				CONTROLLOG << "_element" << LOGENDL;
				return;
			}
			m_oldDesc = _element->getDescription();
			_element->setDescription(m_newDesc);
			CONTROLLOG << "m_oldDesc " << m_oldDesc << " m_newDesc " << m_newDesc << LOGENDL;
			controller.updateInfo(new GuiDataPipeToPlaneProperties(_element->ProduceUIPipeToPlaneMeasure()));
		}

		bool SetDescription::canUndo()
		{
			return (true);
		}

		void SetDescription::undoFunction(Controller& controller)
		{
			CONTROLLOG << "control::PiPlMeasureEdition::SetDescription undo elemid " << m_id << " m_oldDesc " << m_oldDesc << " m_newDesc " << m_newDesc << LOGENDL;
			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			PipeToPlaneMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<PipeToPlaneMeasure>(m_id);
			if (_element == nullptr)
				return;
			_element->setDescription(m_oldDesc);
			controller.updateInfo(new GuiDataPipeToPlaneProperties(_element->ProduceUIPipeToPlaneMeasure()));
		}

		UIControl SetDescription::getType()
		{
			return (UIControl::setDescriptionPiPlMeasureEdit);
		}

		/*
		** SetName
		*/

		SetName::SetName(dataId id, const std::string newName)
		{
			m_id = id;
			m_newName = newName;
		}

		SetName::~SetName()
		{}

		void SetName::doFunction(Controller& controller)
		{
			CONTROLLOG << "control::PiPlMeasureEdition::SetName do elemid " << m_id << LOGENDL;
			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			PipeToPlaneMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<PipeToPlaneMeasure>(m_id);
			if (_element == nullptr)
				return;
			m_oldName = _element->getName();
			_element->setName(m_newName);
			controller.actualizeOnId(m_id, true);
		}

		bool SetName::canUndo()
		{
			return true;
		}

		void SetName::undoFunction(Controller& controller)
		{
			CONTROLLOG << "control::PiPlMeasureEdition::SetName undo elemid " << m_id << LOGENDL;
			if (controller.getContext().getCurrentProject() == nullptr)
				return;
			PipeToPlaneMeasure *_element = controller.getContext().getCurrentProject()->getObjectOnId<PipeToPlaneMeasure>(m_id);
			if (_element == nullptr)
				return;
			_element->setName(m_oldName);
			controller.actualizeOnId(m_id, true);
		}

		UIControl  SetName::getType()
		{
			return (UIControl::setNamePiPlMeasureEdit);
		}
	}
}