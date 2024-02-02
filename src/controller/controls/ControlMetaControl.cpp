#include "ControlMetaControl.h"
#include "utils/Logger.h"

#include "controller/Controller.h"

namespace control
{
	namespace meta
	{

		/*
		* ControlMetaControl
		*/

		ControlMetaControl::ControlMetaControl(const std::list<AControl*>& controls)
			: m_controls(controls)
		{}

		ControlMetaControl::~ControlMetaControl()
		{
			for (AControl* ctrl : m_controls)
				delete ctrl;
		}

		void ControlMetaControl::doFunction(Controller& controller)
		{
			for (AControl* ctrl : m_controls)
				ctrl->doFunction(controller);
			m_controls.reverse();

			CONTROLLOG << "control::meta::ControlMetaControl do on " << m_controls.size() << LOGENDL;
		}

		bool ControlMetaControl::canUndo() const
		{
			for (AControl* ctrl : m_controls)
				if (ctrl->canUndo())
					return true;
			return false;
		}

		void ControlMetaControl::undoFunction(Controller& controller)
		{
			for (AControl* ctrl : m_controls)
				if(ctrl->canUndo())
					ctrl->undoFunction(controller);
			m_controls.reverse();

			CONTROLLOG << "control::meta::ControlMetaControl undo on " << m_controls.size() << LOGENDL;
		}

		void ControlMetaControl::redoFunction(Controller& controller)
		{
			for (AControl* ctrl : m_controls)
				if (ctrl->canUndo())
					ctrl->redoFunction(controller);
			m_controls.reverse();

			CONTROLLOG << "control::meta::ControlMetaControl redo on " << m_controls.size() << LOGENDL;
		}

		ControlType ControlMetaControl::getType() const
		{
			return ControlType::metaControl;
		}

		/*
		* ControlStartMetaControl
		*/

		ControlStartMetaControl::ControlStartMetaControl()
		{}

		ControlStartMetaControl::~ControlStartMetaControl()
		{}

		void ControlStartMetaControl::doFunction(Controller& controller)
		{
			controller.startMetaControl();
		}

		bool ControlStartMetaControl::canUndo() const
		{
			return false;
		}

		ControlType ControlStartMetaControl::getType() const
		{
			return ControlType::startMetaControl;
		}

		/*
		* ControlStopMetaControl
		*/

		ControlStopMetaControl::ControlStopMetaControl()
		{}

		ControlStopMetaControl::~ControlStopMetaControl()
		{
		}

		void ControlStopMetaControl::doFunction(Controller& controller)
		{
			controller.stopMetaControl();
		}

		bool ControlStopMetaControl::canUndo() const
		{
			return false;
		}

		ControlType ControlStopMetaControl::getType() const
		{
			return ControlType::stopMetaControl;
		}
	}
}