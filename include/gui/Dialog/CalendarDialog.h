#ifndef CALENDAR_DIALOG_H_
#define CALENDAR_DIALOG_H_

#include <QtWidgets/QDialog>
#include <QtWidgets/QCalendarWidget>
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "ui_Calendar_Dialog.h"

class CalendarDialog : public QDialog
{
	Q_OBJECT

public:
	explicit CalendarDialog(QWidget *parent = 0);
	~CalendarDialog();

	void setCurrentDate(QDate date);

signals:
	void cancel();
	void finished(QDate date);

public slots:

	void dateUpdate();
	void validateClick();
	void cancelClick();

private:
	QDate _savedDate;

	Ui::CalendarWidget *ui;
};

#endif // !CALENDAR_DIALOG_H_
