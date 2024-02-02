#include "gui/widgets/CustomWidgets/regexpedit.h"

const QRegularExpression QRegExpEdit::Name = QRegularExpression("[\\s\\w\\xc0-\\xff]+");
const QRegularExpression QRegExpEdit::AlphaNumeric = QRegularExpression("[\\s\\w\\xc0-\\xff]+");
const QRegularExpression QRegExpEdit::AlphaNumericWithSeparator = QRegularExpression("[\\s\\w\\xc0-\\xff._-]+");
const QRegularExpression QRegExpEdit::Path = QRegularExpression("[A-Z]{1}:[/\\\\]{1}([\\w\\xc0-\\xff_-]+/)+");
const QRegularExpression QRegExpEdit::FilePath = QRegularExpression("[A-Z]{1}:[/\\\\]{1}([\\w\\xc0-\\xff_-]+/)+[\\s\\w\\xc0-\\xff]+.[\\s\\w\\xc0-\\xff]+");
const QRegularExpression QRegExpEdit::Email = QRegularExpression("(\\w+)((\\.|_|-)(\\w+))*@(\\w+)((\\.|_|-)(\\w+))*((\\.)(\\w+))");
const QRegularExpression QRegExpEdit::Phone = QRegularExpression("^[\+]?[(]?[0-9]{3}[)]?[-\s\.]?[0-9]{3}[-\s\.]?[0-9]{4,6}$");

QRegExpEdit::QRegExpEdit(QWidget* parent) 
	: ACustomLineEdit(parent)
	, m_validator(nullptr)
{
	initialiseValidator();
}

QRegExpEdit::QRegExpEdit(const QRegularExpression& reg, QWidget* parent)
	: ACustomLineEdit(parent)
	, m_validator(nullptr)
{
	initialiseValidator();
	setRegExp(reg);
}

QRegExpEdit::QRegExpEdit(const QRegularExpression& reg, const QString& placeholder, QWidget* parent)
	: ACustomLineEdit(placeholder, parent)
	, m_validator(nullptr)
{
	initialiseValidator();
	setRegExp(reg);
}

QRegExpEdit::~QRegExpEdit()
{
	delete m_validator;
}

void QRegExpEdit::setRegExp(const QRegularExpression& reg)
{
	m_validator->setRegularExpression(reg);
}

QString QRegExpEdit::getValue()
{
	if (checkValue())
	{
		//setPlaceholderText(text());
		return text();
	}
	inputRejected();
	return placeholderText();
}

void QRegExpEdit::initialiseValidator()
{
	m_validator = new QRegularExpressionValidator();
	QLocale lo(QLocale::system());
	m_validator->setLocale(lo);
	this->setValidator(m_validator);
}

bool QRegExpEdit::checkValue()
{
	int pos;
	QString str = text();
	return m_validator->validate(str, pos) == QValidator::State::Acceptable;
}

LineEditType QRegExpEdit::getType()
{
	return LineEditType::REGEXP;
}
