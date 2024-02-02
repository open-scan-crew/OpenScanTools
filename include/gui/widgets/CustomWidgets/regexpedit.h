#ifndef QREGEXP_EDIT_H_
#define QREGEXP_EDIT_H_

#include "ACustomLineEdit.h"
#include <QRegularExpressionValidator>

class QRegExpEdit : public ACustomLineEdit
{
public: 
	static const QRegularExpression Name;
	static const QRegularExpression AlphaNumeric;
	static const QRegularExpression AlphaNumericWithSeparator;
	static const QRegularExpression Path;
	static const QRegularExpression FilePath;
	static const QRegularExpression Email;
	static const QRegularExpression Phone;

public:
	QRegExpEdit(QWidget* parent = nullptr);
	QRegExpEdit(const QRegularExpression& reg, QWidget* parent = nullptr);
	QRegExpEdit(const QRegularExpression& reg, const QString& placeholder, QWidget* parent = nullptr);
	~QRegExpEdit();

	void setRegExp(const QRegularExpression& reg);
	QString getValue();
	bool checkValue();

	LineEditType getType() override;

private:
	void initialiseValidator();

private:
	QRegularExpressionValidator* m_validator;
};

#endif // !QREGEXP_EDIT_H_
