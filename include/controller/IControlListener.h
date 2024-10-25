#ifndef _ICONTROL_LISTENER_H_
#define _ICONTROL_LISTENER_H_

#include "controls/IControl.h"

class IControlListener
{
public:

	/*!
	 * \brief Used to notify that a new event has been trigered.
	 * secondary mean that the event is less important than the others
	 * secondary controls will be process after all primary controls.
	 */

	virtual void notifyUIControl(AControl *event) = 0;
};

#endif // !_IEVENTLISTENER_H_
