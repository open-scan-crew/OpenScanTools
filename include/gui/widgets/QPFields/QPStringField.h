#ifndef QPSTRING_FIELD_H_
#define QPSTRING_FIELD_H_

#include <QtWidgets/QLineEdit>

#include "gui/widgets/QPFields/AQPField.h"
#include "gui/IPanel.h"

class QPStringField : public AQPField
{
public:
	QPStringField(const sma::tField& field, QWidget* parent);
	~QPStringField();

	void setValue(QString value) override;

	QLineEdit *getInfield() const;

private slots:

	void onInfieldEdit();
private:
	void buildQPField(const sma::tField& field) override;

private:
	QLineEdit *m_stringInfield;
	QLabel *m_nameLabel;
	QHBoxLayout *m_layout;
};

#endif // !QPSTRING_FIELD_H_
