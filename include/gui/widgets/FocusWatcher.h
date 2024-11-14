#include <QtWidgets/qwidget.h>

#ifndef FOCUSWATCHER_H_
#define FOCUSWATCHER_H_

class FocusWatcher : public QObject
{
	Q_OBJECT
public:
	FocusWatcher(QObject* parent = nullptr);
	virtual ~FocusWatcher();
	bool eventFilter(QObject *obj, QEvent *event) override;

signals:
	void focusIn();
	void focusOut();
};

class FocusInOutWatcher : public QObject
{
	Q_OBJECT
public:
	FocusInOutWatcher(QObject* parent = nullptr);
	virtual ~FocusInOutWatcher();
	bool eventFilter(QObject* obj, QEvent* event) override;

public slots:
	void sendFocusOutObject(const bool& active);

signals:
	void focusIn();
	void focusOut();
	void focusOutObject(QObject* object);

protected:
	bool checkFocusIn(QObject* obj, QEvent* event);
	bool checkFocusOut(QObject* obj, QEvent* event);

private:
	bool		m_sendFocusOut;
	bool		m_doNotFocusOut;
	QObject*	m_currentFocus;
};


#endif