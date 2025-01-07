#include "gui/widgets/FocusWatcher.h"
#include <QtCore/qcoreevent.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qdialog.h>
#include <QtWidgets/qlineedit.h>


static QObject* activeObj = nullptr;

FocusWatcher::FocusWatcher(QObject* parent) : QObject(parent)
{
	if (parent)
		parent->installEventFilter(this);
}

FocusWatcher::~FocusWatcher() {}

bool FocusWatcher::eventFilter(QObject *obj, QEvent *event)
{
	QEvent::Type type = event->type();

	if (type == QEvent::FocusIn)
		emit focusIn();
	else if (type == QEvent::FocusOut)
		emit focusOut();

	return false;
}

FocusInOutWatcher::FocusInOutWatcher(QObject* parent) 
	: QObject(parent)
	, m_currentFocus(nullptr)
	, m_sendFocusOut(true)
{
	if (parent)
		parent->installEventFilter(this);
}

FocusInOutWatcher::~FocusInOutWatcher() {}

void FocusInOutWatcher::sendFocusOutObject(const bool& active)
{
	m_sendFocusOut = active;
}

bool FocusInOutWatcher::eventFilter(QObject* obj, QEvent* event)
{
	
	if (event->type() == QEvent::FocusIn)
		return checkFocusIn(obj, event);
	else if (event->type() == QEvent::FocusOut)
		return checkFocusOut(obj, event);
	return false;
}

bool FocusInOutWatcher::checkFocusIn(QObject* obj, QEvent* event)
{
	QFocusEvent* focusInEvent = static_cast<QFocusEvent*>(event);
	switch (focusInEvent->reason())
	{
		case Qt::FocusReason::MouseFocusReason:
		case Qt::FocusReason::TabFocusReason:
		case Qt::FocusReason::ActiveWindowFocusReason:
			if (obj->isWidgetType())
			{
				if (dynamic_cast<QLineEdit*>(obj) /* || dynamic_cast<QPlainTextEdit*>(obj)*/)
				{
					QObject* parent(obj->parent());
					while (parent)
					{
						if(dynamic_cast<QDialog*>(parent))
							return obj->event(event);
						parent = parent->parent();
					}
					m_currentFocus = obj;
					emit focusIn();
				}
			}
	}
	return obj->event(event);
}

bool FocusInOutWatcher::checkFocusOut(QObject* obj, QEvent* event)
{
	if (m_currentFocus != obj)
		return false;
	if (m_sendFocusOut)
		emit focusOutObject(m_currentFocus);
	m_currentFocus = nullptr;
	emit focusOut();
	return obj->event(event);
}
