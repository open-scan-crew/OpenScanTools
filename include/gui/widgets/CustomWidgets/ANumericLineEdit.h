#ifndef NUMERIC_LINE_EDIT_H_
#define NUMERIC_LINE_EDIT_H_

#include "gui/widgets/CustomWidgets/ACustomLineEdit.h"

class ANumericLineEdit : public ACustomLineEdit
{
	
public:
	enum class LineEditRules { Nothing, NotZero, NotNegative, PositiveStrict };

public:
	ANumericLineEdit(QWidget* parent = nullptr);
	ANumericLineEdit(const QString&, QWidget* parent = nullptr);
	~ANumericLineEdit();

	void setRules(const LineEditRules& rule);

protected:
	LineEditRules m_rule;
};
#endif // !NUMERIC_LINE_EDIT_H_
