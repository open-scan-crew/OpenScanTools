#ifndef QINTEGER_EDIT_H_
#define QINTEGER_EDIT_H_

#include "ANumericLineEdit.h"
#include <QIntValidator>

class QIntegerEdit : public ANumericLineEdit
{
public:
	QIntegerEdit(QWidget* parent = nullptr);
	QIntegerEdit(const QString&, QWidget* parent = nullptr);
	~QIntegerEdit();

	long getValue();

	LineEditType getType() override;

private:
	void initialiseValidator();
	bool checkValue(const long& value);

private:
	QIntValidator* m_validator;
	bool m_forbidden;
};

#endif // !QINTEGER_EDIT_H_