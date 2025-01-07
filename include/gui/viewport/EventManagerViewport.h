#ifndef EVENT_MANAGER_VIEWPORT_H
#define EVENT_MANAGER_VIEWPORT_H

#include <QtWidgets/qwidget.h>
#include "gui/IDataDispatcher.h"

class EventManagerViewport : public QObject
{
public:
	EventManagerViewport(QWidget* viewW, IDataDispatcher& dispatcher);

	bool eventFilter(QObject *object, QEvent *event);
	void dropEvent(QDropEvent* event);
private:
	IDataDispatcher&	m_dataDispatcher;
	QWidget*			m_widget;

};
#endif