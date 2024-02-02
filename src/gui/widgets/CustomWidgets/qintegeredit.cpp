#include "gui/widgets/CustomWidgets/qintegeredit.h"

QIntegerEdit::QIntegerEdit(QWidget* parent)
	: ANumericLineEdit(parent)
	, m_validator(nullptr)
	, m_forbidden(false)
{
	initialiseValidator();
}

QIntegerEdit::QIntegerEdit(const QString& string, QWidget* parent)
	: ANumericLineEdit(string, parent)
	, m_validator(nullptr)
{
	initialiseValidator();
}

long QIntegerEdit::getValue()
{
	bool ok(false);
	long val(text().toLong(&ok));

	if (ok && checkValue(val))
	{
		if (m_setPlaceholder)
			setPlaceholderText(text());
		return val;
	}
	inputRejected();
	return placeholderText().toLong();
}

LineEditType QIntegerEdit::getType()
{
	return LineEditType::INTEGER;
}

QIntegerEdit::~QIntegerEdit()
{
	delete m_validator;
}

void QIntegerEdit::initialiseValidator()
{
	m_validator = new QIntValidator();
	QLocale lo(QLocale::C);
	lo.setNumberOptions(QLocale::RejectGroupSeparator);
	m_validator->setLocale(lo);
	this->setValidator(m_validator);
}

bool QIntegerEdit::checkValue(const long& value)
{
	switch (m_rule)
	{
	case LineEditRules::PositiveStrict:
		if (value <= 0)
			return false;
	case LineEditRules::NotZero:
		if (!value)
			return false;
	case LineEditRules::NotNegative:
		if (value < 0)
			return false;
	}
	return true;
}