#include "gui/widgets/CustomWidgets/ANumericLineEdit.h"
#include <QtConcurrent/qtconcurrentrun.h>

ANumericLineEdit::ANumericLineEdit(QWidget* parent)
	: ACustomLineEdit(parent)
	, m_rule(LineEditRules::Nothing)
{}

ANumericLineEdit::ANumericLineEdit(const QString& string, QWidget* parent)
	: ACustomLineEdit(string, parent)
	, m_rule(LineEditRules::Nothing)
{}

ANumericLineEdit::~ANumericLineEdit()
{}

void ANumericLineEdit::setRules(const LineEditRules& rule)
{
	m_rule = rule;
}