#include "controller/controls/ControlDuplication.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "utils/Logger.h"

namespace control {
	namespace duplication
	{
		/*
		** SetDuplicationMode
		*/

		SetDuplicationMode::SetDuplicationMode(const DuplicationMode& type)
			: m_type(type)
		{}

		SetDuplicationMode::~SetDuplicationMode()
		{}

		void SetDuplicationMode::doFunction(Controller& controller)
		{
			DuplicationSettings settings(controller.getContext().CgetDuplicationSettings());
			settings.type = m_type;
			controller.getContext().setDuplicationSettings(settings);
			CONTROLLOG << "control::function::clipping::SetDuplicationMode" << Logger::endl;
		}

		bool SetDuplicationMode::canUndo() const
		{
			return (false);
		}
		void SetDuplicationMode::undoFunction(Controller& controller)
		{}

		ControlType SetDuplicationMode::getType() const
		{
			return (ControlType::setDefaultDuplicationModeFunction);
		}

		/*
		** SetDuplicationStepSize
		*/

		SetDuplicationStepSize::SetDuplicationStepSize(const glm::ivec3& step)
			: m_step(step)
		{}

		SetDuplicationStepSize::~SetDuplicationStepSize()
		{}

		void SetDuplicationStepSize::doFunction(Controller& controller)
		{
			DuplicationSettings settings(controller.getContext().CgetDuplicationSettings());
			settings.step = m_step;
			controller.getContext().setDuplicationSettings(settings);
			CONTROLLOG << "control::function::clipping::SetDuplicationStepSize" << Logger::endl;
		}

		bool SetDuplicationStepSize::canUndo() const
		{
			return (false);
		}

		void SetDuplicationStepSize::undoFunction(Controller& controller)
		{}

		ControlType SetDuplicationStepSize::getType() const
		{
			return (ControlType::setDefaultDuplicationStepSizeFunction);
		}

		/*
		** SetDuplicationStepSize
		*/

		SetDuplicationOffsetValue::SetDuplicationOffsetValue(const glm::vec3& offset)
			: m_offset(offset)
		{}

		SetDuplicationOffsetValue::~SetDuplicationOffsetValue()
		{}

		void SetDuplicationOffsetValue::doFunction(Controller& controller)
		{
			DuplicationSettings settings(controller.getContext().CgetDuplicationSettings());
			settings.offset = m_offset;
			controller.getContext().setDuplicationSettings(settings);
			CONTROLLOG << "control::function::clipping::SetDuplicationOffsetValue" << Logger::endl;
		}

		bool SetDuplicationOffsetValue::canUndo() const
		{
			return (false);
		}

		void SetDuplicationOffsetValue::undoFunction(Controller& controller)
		{}

		ControlType SetDuplicationOffsetValue::getType() const
		{
			return (ControlType::setDefaultDuplicationOffsetFunction);
		}

		/*
		** SetDuplicationIsLocal
		*/

		SetDuplicationIsLocal::SetDuplicationIsLocal(const bool& isLocal)
			: m_isLocal(isLocal)
		{}

		SetDuplicationIsLocal::~SetDuplicationIsLocal()
		{}

		void SetDuplicationIsLocal::doFunction(Controller& controller)
		{
			DuplicationSettings settings(controller.getContext().CgetDuplicationSettings());
			settings.isLocal = m_isLocal;
			controller.getContext().setDuplicationSettings(settings);
			CONTROLLOG << "control::function::clipping::SetDuplicationIsLocal" << Logger::endl;
		}

		bool SetDuplicationIsLocal::canUndo() const
		{
			return (false);
		}

		void SetDuplicationIsLocal::undoFunction(Controller& controller)
		{}

		ControlType SetDuplicationIsLocal::getType() const
		{
			return (ControlType::setDefaultDuplicationIsLocalFunction);
		}
	}
}