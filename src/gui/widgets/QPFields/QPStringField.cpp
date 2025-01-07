#include "gui/widgets/QPFields/QPStringField.h"

QPStringField::QPStringField(const sma::tField& field, QWidget* parent)
	: AQPField(field, parent)
{
	buildQPField(field);
}

QPStringField::~QPStringField()
{}

void QPStringField::buildQPField(const sma::tField& field)
{
	m_field = field;
	m_layout = new QHBoxLayout();
	m_stringInfield = new QLineEdit();
	m_stringInfield->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_nameLabel = new QLabel(QString::fromStdWString(field.m_name));
	m_nameLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	m_layout->addWidget(m_nameLabel);
	m_layout->addWidget(m_stringInfield);
	setLayout(m_layout);

	QObject::connect(m_stringInfield, &QLineEdit::editingFinished, this, &QPStringField::onInfieldEdit);
}

QLineEdit * QPStringField::getInfield() const
{
	return (m_stringInfield);
}

void QPStringField::setValue(QString value)
{
	m_stringInfield->blockSignals(true);
	m_stringInfield->setText(value);
	m_stringInfield->blockSignals(false);
}

void QPStringField::onInfieldEdit()
{
	emit edited(m_field.m_id, m_stringInfield->text().toStdWString());
}