#ifndef CONTROL_CREATION_ATTRIBUTES_EDITION_H_
#define CONTROL_CREATION_ATTRIBUTES_EDITION_H_

#include "controller/controls/IControl.h"
#include "models/data/Data.h"
#include "utils/Color32.hpp"

namespace control
{
	namespace attributesEdition
	{
		class SetDefaultColor : public AControl
		{
		public:
			SetDefaultColor(const Color32& newColor);
			~SetDefaultColor();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SetDefaultColor();
		private:
			const Color32 m_newColor;
		};

		class SetDefaultDiscipline : public AControl
		{
		public:
			SetDefaultDiscipline(const std::wstring& discipline);
			~SetDefaultDiscipline();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const std::wstring m_newValue;
		};

		class SetDefaultPhase : public AControl
		{
		public:
			SetDefaultPhase(const std::wstring& prefix);
			~SetDefaultPhase();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const std::wstring m_newValue;
		};

		class SetDefaultIdentifier : public AControl
		{
		public:
			SetDefaultIdentifier(const std::wstring& identifer);
			~SetDefaultIdentifier();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const std::wstring m_newValue;
		};

		class SetDefaultName : public AControl
		{
		public:
			SetDefaultName(const std::wstring& name);
			~SetDefaultName();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const std::wstring m_newName;
		};

	}
}

#endif // !CONTROL_CREATION_ATTRIBUTES_EDITION_H_