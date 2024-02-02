#include "gui/Dialog/CalendarDialog.h"

CalendarDialog::CalendarDialog(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::CalendarWidget)
{
	ui->setupUi(this);
	setModal(true);
	QObject::connect(ui->DateCalendar, &QCalendarWidget::selectionChanged, this, &CalendarDialog::dateUpdate);
	QObject::connect(ui->CancelBtn, SIGNAL(clicked()), this, SLOT(cancelClick()));
	QObject::connect(ui->ValidateBtn, SIGNAL(clicked()), this, SLOT(validateClick()));

	_savedDate = ui->DateCalendar->selectedDate();
}

CalendarDialog::~CalendarDialog()
{
}

void CalendarDialog::setCurrentDate(QDate date)
{
	ui->DateCalendar->setCurrentPage(date.year(), date.month());
	ui->DateCalendar->setSelectedDate(date);
}

void CalendarDialog::dateUpdate()
{
	_savedDate = ui->DateCalendar->selectedDate();
	PANELLOG << "User click on date " << _savedDate.toString().toStdString() << LOGENDL;
}

void CalendarDialog::validateClick()
{
	emit finished(_savedDate);
	PANELLOG << "Validate date at " << _savedDate.toString().toStdString() << LOGENDL;
}

void CalendarDialog::cancelClick()
{
	emit cancel();
	PANELLOG << "Cancel date" << LOGENDL;
}
