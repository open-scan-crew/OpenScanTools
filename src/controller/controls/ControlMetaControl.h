#ifndef CONTROL_META_CONTROL_H_
#define CONTROL_META_CONTROL_H_

#include <list>
#include "controller/controls/IControl.h"

namespace control
{
	namespace meta
	{
		class ControlMetaControl : public AControl
		{
		public:
			ControlMetaControl(const std::list<AControl*>& controls);
			~ControlMetaControl();

			void doFunction(Controller& controller);
			bool canUndo() const;
			void undoFunction(Controller& controller);
			void redoFunction(Controller& controller);
			ControlType getType() const;

		private:
			std::list<AControl*> m_controls;
		};

		class ControlStartMetaControl : public AControl
		{
		public:
			ControlStartMetaControl();
			~ControlStartMetaControl();

			void doFunction(Controller& controller);
			bool canUndo() const;
			void undoFunction(Controller& controller) {}
			ControlType getType() const;
		};

		class ControlStopMetaControl : public AControl
		{
		public:
			ControlStopMetaControl();
			~ControlStopMetaControl();

			void doFunction(Controller& controller);
			bool canUndo() const;
			void undoFunction(Controller& controller) {}
			ControlType getType() const;
		};
	}
}

#endif
