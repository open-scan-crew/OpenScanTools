#ifndef QPDATE_FIELD_H_
#define QPDATE_FIELD_H_

#include "gui/widgets/QPFields/AQPField.h"
#include "gui/Dialog/CalendarDialog.h"

#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qlabel.h>

class QPDateField : public AQPField
{
public:
	QPDateField(const sma::tField& field, QWidget* parent);
	~QPDateField();

	void setValue(QString value) override;

	QPushButton *getButton() const;

public slots:
	void launchCalendar();
	void getCalendarResult(QDate date);
	void cancelCalendar();

private:
	void buildQPField(const sma::tField& field) override;

	//QLineEdit *stringInfield;
	CalendarDialog *m_activeCalendar;
	QPushButton *m_dateBtn;
	QLabel *m_nameLabel;
	QHBoxLayout *m_layout;
};

#endif // !QPSTRING_FIELD_H_
