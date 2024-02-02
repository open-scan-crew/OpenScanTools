#ifndef CONTROL_SIGNALS_H_
#define CONTROL_SIGNALS_H_

#include "controller/controls/IControl.h"

namespace control
{
	namespace signalHandling
	{
		class ISigControl : public AControl
		{
			public:
				ISigControl();
				~ISigControl();
				virtual void doFunction(Controller& controller);
				virtual bool canUndo() const;
				virtual void undoFunction(Controller& controller);
				virtual ControlType getType() const =0;
		};

		class SigInt : public ISigControl
		{
			public:
				SigInt();
				~SigInt();
				void doFunction(Controller& controller);
				ControlType getType() const override;
		};

		class SigTerm : public ISigControl
		{
			public:
				SigTerm();
				~SigTerm();
				void doFunction(Controller& controller);
				ControlType getType() const override;
		};

		class SigIll : public ISigControl
		{
			public:
				SigIll();
				~SigIll();
				void doFunction(Controller& controller);
				ControlType getType() const override;
		};

		class SigAbrt : public ISigControl
		{
			public:
				SigAbrt();
				~SigAbrt();
				void doFunction(Controller& controller);
				ControlType getType() const override;
		};

		class SigSegv : public ISigControl
		{
			public:
				SigSegv();
				~SigSegv();
				void doFunction(Controller& controller);
				ControlType getType() const override;
		};

		class SigFpe : public ISigControl
		{
			public:
				SigFpe();
				~SigFpe();
				void doFunction(Controller& controller);
				ControlType getType() const override;
		};
	}
}
#endif