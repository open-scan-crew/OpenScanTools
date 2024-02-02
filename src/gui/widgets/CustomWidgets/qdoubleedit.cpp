#include "gui/widgets/CustomWidgets/qdoubleedit.h"
#include <QtGui/qevent.h>
#include "magic_enum/magic_enum.hpp"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui\UnitConverter.h"

QDoubleEdit::QDoubleEdit(QWidget* parent)
	: ANumericLineEdit(parent)
	, m_validator(nullptr)
	, m_dataDispatcher(nullptr)
	, m_unit(UnitType::NO_UNIT)
	, m_type(NumericType::NONE)
	, m_valueParameters(unit_usage::by_default)
	, m_meterValue(0.0)
{
	initialiseValidator();
}

QDoubleEdit::QDoubleEdit(const QString& string, QWidget* parent)
	: ANumericLineEdit(string, parent)
	, m_validator(nullptr)
	, m_dataDispatcher(nullptr)
	, m_unit(UnitType::NO_UNIT)
	, m_type(NumericType::NONE)
	, m_valueParameters(unit_usage::by_default)
	, m_meterValue(0.0)
{
	initialiseValidator();
}

const double& QDoubleEdit::getValue()
{
	actualize(true);
	return m_meterValue;
}

void QDoubleEdit::setValue(double value)
{
	m_meterValue = value; 
	actualize(false);
}

LineEditType QDoubleEdit::getType()
{
	return LineEditType::DOUBLE;
}

QDoubleEdit::~QDoubleEdit()
{
	if (m_dataDispatcher != nullptr)
		m_dataDispatcher->unregisterObserver(this);
	delete m_validator;
}

void QDoubleEdit::registerDataDispatcher(IDataDispatcher* dataDispatcher)
{
	m_dataDispatcher = dataDispatcher;
	m_dataDispatcher->registerObserverOnKey(this, guiDType::renderValueDisplay);
}

void QDoubleEdit::informData(IGuiData* keyValue)
{
	if (keyValue->getType() == guiDType::renderValueDisplay)
	{
		m_valueParameters = static_cast<GuiDataRenderUnitUsage*>(keyValue)->m_valueParameters;
		actualize(false);
	}
}

void QDoubleEdit::setUnit(UnitType unit)
{
	m_unit = unit;
	actualize(false);
}

void QDoubleEdit::setType(NumericType type)
{
	m_type = type;
	actualize(false);
}

void QDoubleEdit::setText(const QString& text)
{
	ACustomLineEdit::setText(text);
	actualize(true);
}

void QDoubleEdit::setPower(int power)
{
	m_power = power;
}

void QDoubleEdit::resetUnit()
{
	m_unit = UnitType::NO_UNIT;
	m_type = NumericType::NONE;
	actualize(false);
}

void QDoubleEdit::initialiseValidator()
{
	m_validator = new QDoubleValidator();
	QLocale lo(QLocale::C);
	lo.setNumberOptions(QLocale::RejectGroupSeparator);
	m_validator->setNotation(QDoubleValidator::StandardNotation);
	m_validator->setLocale(lo);
	this->setValidator(m_validator);
}

bool QDoubleEdit::checkValue(const double& value)
{
	switch (m_rule)
	{
	case LineEditRules::PositiveStrict:
		if (value <= 0.0)
			return false;
	case LineEditRules::NotZero:
		if (value == 0.0)
			return false;
	case LineEditRules::NotNegative:
		if (value < 0.0)
			return false;
	}
	return true;
}

void QDoubleEdit::actualize(bool useCurrentText)
{
	switch (m_type)
	{
		case NumericType::DISTANCE:
		{
			m_unit = m_valueParameters.distanceUnit;
		}
		break;
		case NumericType::DIAMETER:
		{
			m_unit = m_valueParameters.diameterUnit;
		}
		break;
		case NumericType::VOLUME:
		{
			m_unit = m_valueParameters.volumeUnit;
		}
		break;
	}

	if (useCurrentText)
	{
		bool ok(false);
		QString t = text();
		double val(t.toDouble(&ok));
		if (ok && checkValue(val))
		{
			val = unit_converter::XToMeter(val, m_unit);
			m_meterValue = val;
		}
	}

	QString text = QString::number(unit_converter::meterToX(m_meterValue, m_unit), 'f', m_valueParameters.displayedDigits);
	if (isReadOnly() || !hasFocus())
		text += unit_converter::getUnitText(m_unit) + ((m_power > 1) ? QString::number(m_power) : QString()); 

	blockInputReject(true);
	ACustomLineEdit::setText(text);
	blockInputReject(false);
}


void QDoubleEdit::focusInEvent(QFocusEvent* fevent)
{
	ANumericLineEdit::focusInEvent(fevent);
	actualize(false);
}

void QDoubleEdit::focusOutEvent(QFocusEvent* fevent)
{
	ANumericLineEdit::focusOutEvent(fevent);
	actualize(true);
}
