#include <exception>
#include <cctype>

#include "gui/widgets/QPFields/QPNumberField.h"

QPNumberField::QPNumberField(const sma::tField& field, QWidget* parent)
	: AQPField(field, parent)
{
	buildQPField(field);
}

QPNumberField::~QPNumberField()
{}

void QPNumberField::buildQPField(const sma::tField& field)
{
	m_field = field;
	layout = new QHBoxLayout();
	numberInfield = new QLineEdit();
	numberInfield->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	nameLabel = new QLabel(QString::fromStdWString(field.m_name));
	nameLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	layout->addWidget(nameLabel);
	layout->addWidget(numberInfield);
	setLayout(layout);

	QObject::connect(numberInfield, &QLineEdit::editingFinished, this, &QPNumberField::onInfieldEdit);
}

QLineEdit * QPNumberField::getInfield() const
{
	return (numberInfield);
}

void QPNumberField::onInfieldEdit()
{
	std::string str = numberInfield->text().toStdString();

	if (str.empty() == false)
	{
		std::string::iterator it;
		for (it = str.begin(); it != str.end(); it++)
		{
			if (std::isdigit(*it) == 0)
			{
				numberInfield->setText(QString::fromStdString("0"));
				return;
			}
		}
		emit edited(m_field.m_id, numberInfield->text().toStdWString());
	}
	else
		numberInfield->setText(QString::fromStdString("0"));
}

void QPNumberField::setValue(QString value)
{
	numberInfield->blockSignals(true);
	numberInfield->setText(value);
	numberInfield->blockSignals(false);
}