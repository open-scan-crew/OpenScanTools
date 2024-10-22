#include "controller/controls/ControlCreationAttributesEdition.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "utils/Logger.h"

// control::attributesEdition::

namespace control
{
	namespace attributesEdition
	{
		/*
		** SetColor
		*/

		SetDefaultColor::SetDefaultColor(const Color32& newColor)
			: m_newColor(newColor)
		{}

		SetDefaultColor::~SetDefaultColor()
		{}

		void SetDefaultColor::doFunction(Controller& controller)
		{
			controller.getContext().setActiveColor(m_newColor);
			CONTROLLOG << "control::attributesEdition::SetDefaultColor do newColor " << m_newColor << Logger::endl;
		}

		bool SetDefaultColor::canUndo() const
		{
			return (false);
		}

		void SetDefaultColor::undoFunction(Controller& controller)
		{}

		ControlType SetDefaultColor::getType() const
		{
			return (ControlType::setColorAttribute);
		}

		SetDefaultColor::SetDefaultColor() 
		{}

		/*
		** setDiscipline
		*/

		SetDefaultDiscipline::SetDefaultDiscipline(const std::wstring& discipline)
			: m_newValue(discipline)
		{}

		SetDefaultDiscipline::~SetDefaultDiscipline()
		{}

		void SetDefaultDiscipline::doFunction(Controller& controller)
		{
			controller.getContext().setActiveDiscipline(m_newValue);
			CONTROLLOG << "control::attributesEdition::SetDefaultDiscipline do newDisc " << m_newValue << Logger::endl;
		}

		bool SetDefaultDiscipline::canUndo() const
		{
			return (false);
		}

		void SetDefaultDiscipline::undoFunction(Controller& controller)
		{}

		ControlType SetDefaultDiscipline::getType() const
		{
			return (ControlType::setDisciplineAttribute);
		}

		/*
		** SetPhase
		*/

		SetDefaultPhase::SetDefaultPhase(const std::wstring& prefix)
			: m_newValue(prefix)
		{}

		SetDefaultPhase::~SetDefaultPhase()
		{}

		void SetDefaultPhase::doFunction(Controller& controller)
		{
			controller.getContext().setActivePhase(m_newValue);
			CONTROLLOG << "control::attributesEdition::SetDefaultPhase do newPref " << m_newValue << Logger::endl;
		}

		bool SetDefaultPhase::canUndo() const
		{
			return (false);
		}

		void SetDefaultPhase::undoFunction(Controller& controller)
		{}

		ControlType SetDefaultPhase::getType() const
		{
			return (ControlType::setPhaseAttribute);
		}

		/*
		** SetIdentifier
		*/

		SetDefaultIdentifier::SetDefaultIdentifier(const std::wstring& identifer)
			: m_newValue(identifer)
		{}

		SetDefaultIdentifier::~SetDefaultIdentifier()
		{}

		void SetDefaultIdentifier::doFunction(Controller& controller)
		{
			controller.getContext().setActiveIdentifer(m_newValue);
			CONTROLLOG << "control::attributesEdition::SetDefaultIdentifier do newIdent " << m_newValue << Logger::endl;
		}

		bool SetDefaultIdentifier::canUndo() const
		{
			return (false);
		}

		void SetDefaultIdentifier::undoFunction(Controller& controller)
		{}

		ControlType SetDefaultIdentifier::getType() const
		{
			return (ControlType::setIdentifierAttribute);
		}

		/*
		** SetName
		*/

		SetDefaultName::SetDefaultName(const std::wstring& name)
			: m_newName(name)
		{}

		SetDefaultName::~SetDefaultName()
		{}

		void SetDefaultName::doFunction(Controller& controller)
		{
			controller.getContext().setActiveName(m_newName);
			CONTROLLOG << "control::attributesEdition::SetDefaultName do newName " << m_newName << Logger::endl;
		}

		bool SetDefaultName::canUndo() const
		{
			return (false);
		}

		void SetDefaultName::undoFunction(Controller& controller)
		{}

		ControlType SetDefaultName::getType() const
		{
			return (ControlType::setNameAttribute);
		}
	}
}