#ifndef CONTROL_LISTENER_H
#define CONTROL_LISTENER_H

#include "controller/IControlListener.h"

#include <list>
#include <mutex>

class ControlListener : public IControlListener
{
public:
	void virtual notifyUIControl(AControl *event) override;

	/*!
	 * \brief Used to pop the normally stored controlls inside the control queue
	 */
	std::list<AControl*> popBlockControls();
public:
	ControlListener();
	~ControlListener();

	//bool isControlFiltred(const ControlType& type) const override;
	

private:
	std::list<AControl*> m_eventQueue;
	std::mutex			 m_listenerMutex;
};

#endif // !CONTROL_LISTENER_H