#include <QtWidgets/qlineedit.h>

#include "gui/widgets/QPFields/AQPField.h"

#ifndef QPNUMBER_FIELD_H_
#define QPNUMBER_FIELD_H_

class QPNumberField : public AQPField
{
public:
	QPNumberField(const sma::tField& field, QWidget* parent);
	~QPNumberField();

	void setValue(QString value) override;

	QLineEdit *getInfield() const;

private slots:
	void onInfieldEdit();

private:
	void buildQPField(const sma::tField& field) override;

private:
	QLineEdit *numberInfield = nullptr;
	QLabel *nameLabel = nullptr;
	QHBoxLayout *layout = nullptr;
};

#endif // !QPSTRING_FIELD_H_
