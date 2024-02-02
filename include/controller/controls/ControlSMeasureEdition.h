#ifndef CONTROLSMEASQUREEDITION_H_
#define CONTROLSMEASQUREEDITION_H_

#include "controller/controls/IControl.h"
#include "models/data/Data.h"

namespace control
{
	namespace SMeasureEdition
	{
		class SetDescription : public AControl
		{
		public:
			SetDescription(xg::Guid id, std::string newDesc);
			~SetDescription();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			xg::Guid m_id;
			std::string m_oldDesc;
			std::string m_newDesc;
		};

		class SetName : public AControl
		{
		public:
			SetName(xg::Guid id, const std::string newName);
			~SetName();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			xg::Guid m_id;
			std::string m_newName;
			std::string m_oldName;
		};
	}
	namespace BBMeasureEdition
	{
		class SetDescription : public AControl
		{
		public:
			SetDescription(xg::Guid id, std::string newDesc);
			~SetDescription();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			xg::Guid m_id;
			std::string m_oldDesc;
			std::string m_newDesc;
		};

		class SetName : public AControl
		{
		public:
			SetName(xg::Guid id, const std::string newName);
			~SetName();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			xg::Guid m_id;
			std::string m_newName;
			std::string m_oldName;
		};
	}
	namespace PPMeasureEdition
	{
		class SetDescription : public AControl
		{
		public:
			SetDescription(xg::Guid id, std::string newDesc);
			~SetDescription();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			xg::Guid m_id;
			std::string m_oldDesc;
			std::string m_newDesc;
		};

		class SetName : public AControl
		{
		public:
			SetName(xg::Guid id, const std::string newName);
			~SetName();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			xg::Guid m_id;
			std::string m_newName;
			std::string m_oldName;
		};
	}
	namespace PiPMeasureEdition
	{
		class SetDescription : public AControl
		{
		public:
			SetDescription(xg::Guid id, std::string newDesc);
			~SetDescription();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			xg::Guid m_id;
			std::string m_oldDesc;
			std::string m_newDesc;
		};

		class SetName : public AControl
		{
		public:
			SetName(xg::Guid id, const std::string newName);
			~SetName();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			xg::Guid m_id;
			std::string m_newName;
			std::string m_oldName;
		};
	}
	namespace PiPlMeasureEdition
	{
		class SetDescription : public AControl
		{
		public:
			SetDescription(xg::Guid id, std::string newDesc);
			~SetDescription();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			xg::Guid m_id;
			std::string m_oldDesc;
			std::string m_newDesc;
		};

		class SetName : public AControl
		{
		public:
			SetName(xg::Guid id, const std::string newName);
			~SetName();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			xg::Guid m_id;
			std::string m_newName;
			std::string m_oldName;
		};
	}
}

#endif // !CONTROLSMEASQUREEDITION_H_