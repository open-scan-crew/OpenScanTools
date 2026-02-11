#include "gui/widgets/QPFields/QPDateField.h"
#include "utils/Logger.h"

QPDateField::QPDateField(const sma::tField& field, QWidget* parent)
	: AQPField(field, parent)
	, m_activeCalendar(nullptr)
{
	buildQPField(field);
}

QPDateField::~QPDateField()
{}

void QPDateField::buildQPField(const sma::tField& field)
{
	m_field = field;
	m_layout = new QHBoxLayout();
	m_dateBtn = new QPushButton();
	m_dateBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_nameLabel = new QLabel(QString::fromStdWString(field.m_name));
	m_nameLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	m_layout->addWidget(m_nameLabel);
	m_layout->addWidget(m_dateBtn);
	setLayout(m_layout);
	QObject::connect(m_dateBtn, &QPushButton::clicked, this, &QPDateField::launchCalendar);
}

QPushButton * QPDateField::getButton() const
{
	return (m_dateBtn);
}

void QPDateField::setValue(QString value)
{
	m_dateBtn->setText(value);
}

void QPDateField::launchCalendar()
{
	if (!m_activeCalendar)
	{
		m_activeCalendar = new CalendarDialog(static_cast<QWidget*>(this->parent()));
		connect(m_activeCalendar, &CalendarDialog::finished, this, &QPDateField::getCalendarResult);
		connect(m_activeCalendar, &CalendarDialog::cancel, this, &QPDateField::cancelCalendar);
	}
	m_activeCalendar->exec();
}

void QPDateField::getCalendarResult(QDate date)
{
	GUI_LOG << "receive TO calendar " << date.toString().toStdString() << LOGENDL;
	QDateTime dt;
	QTime time;
	time.setHMS(12, 0, 0);
	dt.setTime(time);
	dt.setDate(date);
	time_t t = dt.toTime_t();
	m_dateBtn->setText(date.toString());
	emit edited(m_field.m_id, date.toString().toStdWString());
	cancelCalendar();
}

void QPDateField::cancelCalendar()
{
	if (m_activeCalendar)
	{
		m_activeCalendar->close();
		delete(m_activeCalendar);
		m_activeCalendar = nullptr;
	}
}